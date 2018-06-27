/**
 * Copyright (c) 2011-2017 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <bitcoin/client/dealer.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <string>
#include <boost/iostreams/stream.hpp>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/client/stream.hpp>

// A REQ socket always adds a delimiter.
// Server v1/v2 expect no delimiter and therefore will fail REQ clients.
// A delimiter frame is optional for a DEALER socket (v1/v2/v3 clients).
// In v3 we don't add the delimiter but the v3 server allows it.
// By v4 client we can always send the delimiter (expecting v3+ server).
#undef USE_ADDRESS_DELIMITER

namespace libbitcoin {
namespace client {

using namespace bc::chain;
using namespace bc::wallet;

typedef boost::iostreams::stream<byte_source<data_chunk>> byte_stream;

static const auto on_update_nop = [](const code&, uint16_t, size_t,
    const hash_digest&)
{
};

static int32_t unsigned_to_signed(uint32_t value)
{
    const auto maximum_unsigned = static_cast<uint32_t>(max_int32);
    const auto capped_signed = std::min(value, maximum_unsigned);
    return static_cast<int32_t>(capped_signed);
}

dealer::dealer(stream& out, unknown_handler on_unknown_command,
    uint32_t timeout_milliseconds, uint8_t resends)
  : last_request_index_(0),
    resends_(resends),
    timeout_milliseconds_(unsigned_to_signed(timeout_milliseconds)),
    on_unknown_(on_unknown_command),
    on_update_(on_update_nop),
    out_(out)
{
}

dealer::~dealer()
{
    clear(error::channel_timeout);
}

bool dealer::empty() const
{
    return pending_.empty();
}

void dealer::clear(const code& code)
{
    for (const auto& request: pending_)
        request.second.on_error(code);

    pending_.clear();
}

void dealer::set_on_update(update_handler on_update)
{
    on_update_ = on_update;
}

// Send, kill or ignore pending messages as necessary.
// Return maximum time before next required refresh in milliseconds.
// Subscriptions notification handlers are not registered in pending.
int32_t dealer::refresh()
{
    auto interval = timeout_milliseconds_;

    // Use explicit iteration to allow for erase in loop.
    auto request = pending_.begin();
    while (request != pending_.end())
    {
        const auto milliseconds_remain = remaining(request->second.deadline);

        if (milliseconds_remain > 0)
        {
            // Not timed out, go to sleep for no more than remaining time.
            interval = milliseconds_remain;
            ++request;
        }
        else if (request->second.resends < resends_)
        {
            // Resend is helpful in the case where the server is overloaded.
            // A zmq router drops messages as it reaches the high water mark.
            request->second.resends++;
            request->second.deadline = asio::steady_clock::now() +
                asio::milliseconds(timeout_milliseconds_);

            // Timed out and not done, go to sleep for no more than timeout.
            interval = std::min(interval, timeout_milliseconds_);

            // Resend the request message due to response timeout.
            send(request->second.message);
            ++request;
        }
        else
        {
            // Timed out and exceeded retries, handle error and dequeue.
            request->second.on_error(error::channel_timeout);
            request = pending_.erase(request);
        }
    }

    // We emit as int32 because of poller.wait.
    return interval;
}

// Return time to deadline in milliseconds.
int32_t dealer::remaining(const asio::time_point& deadline)
{
    // Convert bounds to the larger type of the conversion.
    static constexpr auto maximum = static_cast<int64_t>(max_int32);
    static constexpr auto minimum = static_cast<int64_t>(min_int32);

    // Get the remaining time in milliseconds (may be negative).
    const auto remainder = deadline - asio::steady_clock::now();
    const auto count = std::chrono::duration_cast<asio::milliseconds>(
        remainder).count();

    // Prevent overflow and underflow in the conversion to int32.
    const auto capped = std::min(count, maximum);
    const auto bounded = std::max(capped, minimum);
    return static_cast<int32_t>(bounded);
}

// Create a message with identity and send it via the message stream.
// This is invoked by derived class message senders, such as the proxy.
bool dealer::send_request(const std::string& command,
    const data_chunk& payload, error_handler on_error, decoder on_reply)
{
    const auto now = asio::steady_clock::now();
    const auto id = ++last_request_index_;
    auto& request = pending_[id];
    request.message = obelisk_message{ command, id, payload };
    request.on_error = on_error;
    request.on_reply = on_reply;
    request.resends = 0;
    request.deadline = now + asio::milliseconds(timeout_milliseconds_);
    return send(request.message);
}

// Send or resend an existing message by writing it to the message stream.
bool dealer::send(const obelisk_message& message)
{
    data_stack data;

#ifdef USE_ADDRESS_DELIMITER
    data.push_back({});
#endif

    data.push_back(to_chunk(message.command));
    data.push_back(to_chunk(to_little_endian(message.id)));
    data.push_back(message.payload);

    // This creates a message and sends it on the socket.
    return out_.write(data);
}

// Stream interface, not utilized on this class.
bool dealer::read(stream& )
{
    return false;
}

// stream interface.
bool dealer::write(const data_stack& data)
{
    if (data.size() < 3 || data.size() > 4)
        return false;

    obelisk_message message;
    auto it = data.begin();

    // See note in send().
    // Forward compatibility with a future server would send the delimiter.
    // Strip the delimiter if the server includes it.
    if (data.size() == 4)
        ++it;

    // Copy the first token into command.
    message.command = std::string(it->begin(), it->end());

    // Require the second token is 4 bytes.
    if ((++it)->size() == sizeof(uint32_t))
    {
        // Copy the second token into id.
        message.id = from_little_endian_unsafe<uint32_t>(it->begin());

        // Copy the third token into payload.
        message.payload = *(++it);
    }

    return receive(message);
}

// Handle a message, call from write.
bool dealer::receive(const obelisk_message& message)
{
    // Subscription updates are not tracked in pending.
    if (message.command == "notification.address" ||
        message.command == "notification.stealth")
    {

        // Currently these message formats are the same.
        decode_update(message);
        return true;
    }

    const auto command = pending_.find(message.id);
    if (command == pending_.end())
    {
        on_unknown_(message.command);
        return false;
    }

    decode_reply(message, command->second.on_error, command->second.on_reply);
    pending_.erase(command);
    return true;
}

void dealer::decode_reply(const obelisk_message& message,
    error_handler& on_error, decoder& on_reply)
{
    byte_stream istream(message.payload);
    istream_reader source(istream);

    const auto ec = source.read_error_code();
    if (ec)
        on_error(ec);
    else if (!on_reply(source))
        on_error(error::bad_stream);
}

void dealer::decode_update(const obelisk_message& message)
{
    byte_stream istream(message.payload);
    istream_reader source(istream);

    // [ code:4 ]     <- if this is nonzero then rest may be empty.
    // [ sequence:2 ] <- if out of order there was a lost message.
    // [ height:4 ]   <- 0 for unconfirmed or error tx (cannot notify genesis).
    // [ tx_hash:32 ] <- may be null_hash on errors.

    const auto ec = source.read_error_code();

    if (ec)
    {
        on_update_(ec, 0, 0, {});
        return;
    }

    const auto sequence = source.read_2_bytes_little_endian();
    const size_t height = source.read_4_bytes_little_endian();
    const auto tx_hash = source.read_hash();

    if (!source.is_exhausted())
    {
        // This is incorrect, we need an error handler member.
        on_unknown_(message.command);
        return;
    }

    // Caller must differentiate type of update if subscribed to multiple.
    on_update_(ec, sequence, height, tx_hash);
}

} // namespace client
} // namespace libbitcoin
