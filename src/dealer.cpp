/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin-client.
 *
 * libbitcoin-client is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <bitcoin/client/dealer.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <string>
#include <boost/iostreams/stream.hpp>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/client/stream.hpp>

namespace libbitcoin {
namespace client {

using namespace std::chrono;
using namespace bc::chain;
using namespace bc::wallet;

typedef boost::iostreams::stream<byte_source<data_chunk>> byte_stream;

static const auto on_update_nop = [](const payment_address&, size_t,
    const hash_digest&, const transaction&)
{
};

static const auto on_stealth_update_nop = [](const binary&, size_t,
    const hash_digest&, const transaction&)
{
};

dealer::dealer(stream& out, unknown_handler on_unknown_command,
    uint32_t timeout_ms, uint8_t resends)
  : last_request_index_(0),
    resends_(resends),
    timeout_ms_(std::min(timeout_ms, static_cast<uint32_t>(max_int32))),
    on_unknown_(on_unknown_command),
    on_update_(on_update_nop),
    on_stealth_update_(on_stealth_update_nop),
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
    on_update_ = std::move(on_update);
}

void dealer::set_on_stealth_update(stealth_update_handler on_update)
{
    on_stealth_update_ = std::move(on_update);
}

// Send, kill or ignore pending messages as necessary.
// Return maximum time before next required refresh in milliseconds.
// Subscriptions notification handlers are not registered in pending.
int32_t dealer::refresh()
{
    auto interval = timeout_ms_;

    // Use explicit iteration to allow for erase in loop.
    auto request = pending_.begin();
    while (request != pending_.end())
    {
        const auto remainder_ms = remaining(request->second.deadline);

        if (remainder_ms > 0)
        {
            // Not timed out, go to sleep for no more than remaining time.
            interval = remainder_ms;
            ++request;
        }
        else if (request->second.resends < resends_)
        {
            // Resend doesn't make sense unless reconnecting.

            request->second.resends++;
            request->second.deadline = steady_clock::now() + 
                milliseconds(timeout_ms_);

            // Timed out and not done, go to sleep for no more than timeout.
            interval = std::min(interval, timeout_ms_);

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
int32_t dealer::remaining(const time& deadline)
{
    // Convert bounds to the larger type of the conversion.
    static constexpr auto maximum = static_cast<int64_t>(max_int32);
    static constexpr auto minimum = static_cast<int64_t>(min_int32);

    // Get the remaining time in milliseconds (may be negative).
    const auto remainder = deadline - steady_clock::now();
    const auto count = duration_cast<milliseconds>(remainder).count();

    // Prevent overflow and underflow in the conversion to int32.
    const auto capped = std::min(count, maximum);
    const auto bounded = std::max(capped, minimum);
    return static_cast<int32_t>(bounded);
}

// Create a mssage with identity and send it via the message stream.
// This is invoked by derived class message senders, such as the proxy.
void dealer::send_request(const std::string& command,
    const data_chunk& payload, error_handler on_error, decoder on_reply)
{
    const auto id = ++last_request_index_;
    auto& request = pending_[id];
    request.message = obelisk_message{ command, id, payload };
    request.on_error = std::move(on_error);
    request.on_reply = std::move(on_reply);
    request.resends = 0;
    request.deadline = steady_clock::now() + milliseconds(timeout_ms_);
    send(request.message);
}

// Send or resend an existing message by writing it to the message stream.
void dealer::send(const obelisk_message& message)
{
    data_stack data;
    data.push_back(to_chunk(message.command));
    data.push_back(to_chunk(to_little_endian(message.id)));
    data.push_back(message.payload);

    // This creates a message and sends it on the socket.
    out_.write(data);
}

// Stream interface, not utilized on this class.
bool dealer::read(stream& stream)
{
    return false;
}

// stream interface.
void dealer::write(const data_stack& data)
{
    // Require exactly three tokens.
    if (data.size() != 3)
        return;

    auto it = data.begin();
    obelisk_message message;

    // Copy the first token into command.
    message.command = std::string(it->begin(), it->end());

    // Require the second token is 4 bytes.
    if ((++it)->size() == sizeof(uint32_t))
    {
        // Copy the second token into id.
        message.id = from_little_endian_unsafe<uint32_t>(it->begin());

        // Copy the third token into id.
        message.payload = *(++it);
    }

    receive(message);
}

// Handle a message, call from write.
void dealer::receive(const obelisk_message& message)
{
    // Subscription updates are not tracked in pending.
    if (message.command == "address.update")
    {
        decode_update(message);
        return;
    }

    // Subscription updates are not tracked in pending.
    if (message.command == "address.stealth_update")
    {
        decode_stealth_update(message);
        return;
    }

    const auto command = pending_.find(message.id);
    if (command == pending_.end())
    {
        on_unknown_(message.command);
        return;
    }

    decode_reply(message, command->second.on_error, command->second.on_reply);
    pending_.erase(command);
}

void dealer::decode_reply(const obelisk_message& message,
    error_handler& on_error, decoder& on_reply)
{
    byte_stream istream(message.payload);
    istream_reader source(istream);
    code ec = static_cast<error::error_code_t>(
        source.read_4_bytes_little_endian());

    if (ec)
        on_error(ec);
    else if (!on_reply(source))
        on_error(error::bad_stream);
}

void dealer::decode_update(const obelisk_message& message)
{
    byte_stream istream(message.payload);
    istream_reader source(istream);

    // This message does not have an error_code at the beginning.
    const auto version_byte = source.read_byte();
    const auto address_hash = source.read_short_hash();
    const payment_address address(address_hash, version_byte);
    const auto height = source.read_4_bytes_little_endian();
    const auto block_hash = source.read_hash();
    transaction tx;

    if (!tx.from_data(source) || !source.is_exhausted())
    {
        // This is incorrect, we need an error handler member.
        on_unknown_(message.command);
        return;
    }

    on_update_(address, height, block_hash, tx);
}

void dealer::decode_stealth_update(const obelisk_message& message)
{
    byte_stream istream(message.payload);
    istream_reader source(istream);

    // This message does not have an error_code at the beginning.
    static constexpr size_t prefix_size = sizeof(uint32_t);
    binary prefix(prefix_size * byte_bits, source.read_bytes<prefix_size>());
    const auto height = source.read_4_bytes_little_endian();
    const auto block_hash = source.read_hash();
    transaction tx;

    if (!tx.from_data(source) || !source.is_exhausted())
    {
        // This is incorrect, we need an error handler member.
        on_unknown_(message.command);
        return;
    }

    on_stealth_update_(prefix, height, block_hash, tx);
}

} // namespace client
} // namespace libbitcoin
