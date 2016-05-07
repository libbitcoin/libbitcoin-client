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
#include <bitcoin/client/obelisk/obelisk_router.hpp>

#include <cstdint>
#include <string>
#include <boost/iostreams/stream.hpp>
#include <bitcoin/bitcoin.hpp>

namespace libbitcoin {
namespace client {

using std::placeholders::_1;
using namespace std::chrono;
using namespace boost::iostreams;
using namespace bc::chain;
using namespace bc::wallet;

static constexpr period_ms default_timeout_ms(2000);

// This class is not thread safe.
obelisk_router::obelisk_router(std::shared_ptr<message_stream> out)
  : retries_(0),
    last_request_index_(0),
    timeout_ms_(default_timeout_ms),
    on_update_(on_update_nop),
    on_unknown_(on_unknown_nop),
    on_stealth_update_(on_stealth_update_nop),
    out_(out)
{
}

obelisk_router::~obelisk_router()
{
    for (const auto &request: pending_requests_)
        request.second.on_error(error::channel_stopped);
}

void obelisk_router::set_on_update(update_handler on_update)
{
    on_update_ = std::move(on_update);
}

void obelisk_router::set_on_stealth_update(stealth_update_handler on_update)
{
    on_stealth_update_ = std::move(on_update);
}

void obelisk_router::set_on_unknown(unknown_handler on_unknown)
{
    on_unknown_ = std::move(on_unknown);
}

void obelisk_router::set_retries(uint8_t retries)
{
    retries_ = retries;
}

void obelisk_router::set_timeout(period_ms timeout)
{
    timeout_ms_ = timeout;
}

uint64_t obelisk_router::outstanding_call_count() const
{
    return pending_requests_.size();
}

void obelisk_router::write(const data_stack& data)
{
    if (data.size() != 3)
        return;

    obelisk_message message;

    auto it = data.begin();
    message.command = std::string(it->begin(), it->end());

    if ((++it)->size() == sizeof(uint32_t))
    {
        message.id = from_little_endian_unsafe<uint32_t>(it->begin());
        message.payload = *(++it);
    }

    receive(message);
}

period_ms obelisk_router::wakeup()
{
    period_ms next_wakeup(0);
    const auto now = steady_clock::now();

    // Use explicit iteration to allow for erase in loop.
    auto request = pending_requests_.begin();
    while (request != pending_requests_.end())
    {
        const auto elapsed = now - request->second.last_action;
        const auto elapsed_ms = duration_cast<period_ms>(elapsed);

        if (timeout_ms_ > elapsed_ms)
        {
            next_wakeup = min_sleep(next_wakeup, timeout_ms_ - elapsed_ms);
            ++request;
        }
        else if (request->second.retries >= retries_)
        {
            request->second.on_error(error::channel_timeout);
            request = pending_requests_.erase(request);
        }
        else
        {
            request->second.retries++;
            request->second.last_action = now;
            next_wakeup = min_sleep(next_wakeup, timeout_ms_);
            send(request->second.message);
            ++request;
        } 
    }

    return next_wakeup;
}

void obelisk_router::send_request(const std::string& command,
    const data_chunk& payload, error_handler on_error, decoder on_reply)
{
    const auto id = ++last_request_index_;
    auto& request = pending_requests_[id];
    request.message = obelisk_message{ command, id, payload };
    request.on_error = std::move(on_error);
    request.on_reply = std::move(on_reply);
    request.retries = 0;
    request.last_action = steady_clock::now();
    send(request.message);
}

void obelisk_router::send(const obelisk_message& message)
{
    if (!out_)
        return;

    data_stack data;
    data.push_back(to_chunk(message.command));
    data.push_back(to_chunk(to_little_endian(message.id)));
    data.push_back(message.payload);
    out_->write(data);
}

void obelisk_router::receive(const obelisk_message& message)
{
    if (message.command == "address.update")
    {
        decode_update(message);
        return;
    }

    if (message.command == "address.stealth_update")
    {
        decode_stealth_update(message);
        return;
    }

    const auto command = pending_requests_.find(message.id);
    if (command == pending_requests_.end())
    {
        on_unknown_(message.command);
        return;
    }

    decode_reply(message, command->second.on_error, command->second.on_reply);
    pending_requests_.erase(command);
}

void obelisk_router::decode_update(const obelisk_message& message)
{
    stream<byte_source<data_chunk>> istream(message.payload);
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
        on_unknown_(message.command);
        return;
    }

    on_update_(address, height, block_hash, tx);
}

void obelisk_router::decode_stealth_update(const obelisk_message& message)
{
    stream<byte_source<data_chunk>> istream(message.payload);
    istream_reader source(istream);

    // This message does not have an error_code at the beginning.
    static constexpr size_t size = 4;
    binary prefix(size * bc::byte_bits, source.read_bytes<size>());
    const auto height = source.read_4_bytes_little_endian();
    const auto block_hash = source.read_hash();
    transaction tx;

    if (!tx.from_data(source) || !source.is_exhausted())
    {
        on_unknown_(message.command);
        return;
    }

    on_stealth_update_(prefix, height, block_hash, tx);
}

void obelisk_router::decode_reply(const obelisk_message& message,
    error_handler& on_error, decoder& on_reply)
{
    stream<byte_source<data_chunk>> istream(message.payload);
    istream_reader source(istream);
    code ec = static_cast<error::error_code_t>(
        source.read_4_bytes_little_endian());

    if (ec)
        on_error(ec);
    else if (!on_reply(source))
        on_error(error::bad_stream);
}

void obelisk_router::on_unknown_nop(const std::string&)
{
}

void obelisk_router::on_update_nop(const wallet::payment_address&,
    size_t, const hash_digest&, const chain::transaction&)
{
}

void obelisk_router::on_stealth_update_nop(const binary&,
    size_t, const hash_digest&, const chain::transaction&)
{
}

} // namespace client
} // namespace libbitcoin
