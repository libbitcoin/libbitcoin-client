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

#include <boost/iostreams/stream.hpp>
#include <bitcoin/bitcoin.hpp>

namespace libbitcoin {
namespace client {

using std::placeholders::_1;

BCC_API obelisk_router::obelisk_router(std::shared_ptr<message_stream> out)
  : last_request_id_(0),
    timeout_(std::chrono::seconds(2)), retries_(0),
    on_unknown_(on_unknown_nop), on_update_(on_update_nop),
    on_stealth_update_(on_stealth_update_nop), out_(out)
{
}

obelisk_router::~obelisk_router()
{
}

BCC_API void obelisk_router::set_on_update(update_handler on_update)
{
    on_update_ = std::move(on_update);
}

BCC_API void obelisk_router::set_on_stealth_update(
    stealth_update_handler on_update)
{
    on_stealth_update_ = std::move(on_update);
}

BCC_API void obelisk_router::set_on_unknown(unknown_handler on_unknown)
{
    on_unknown_ = std::move(on_unknown);
}

BCC_API void obelisk_router::set_retries(uint8_t retries)
{
    retries_ = retries;
}

BCC_API void obelisk_router::set_timeout(period_ms timeout)
{
    timeout_ = timeout;
}

BCC_API uint64_t obelisk_router::outstanding_call_count() const
{
    return pending_requests_.size();
}

BCC_API void obelisk_router::write(const data_stack& data)
{
    if (data.size() == 3)
    {
        auto success = true;
        obelisk_message message;

        auto it = data.begin();

        if (success)
        {
            // read command
            message.command = std::string((*it).begin(), (*it).end());
            it++;
        }

        if (success)
        {
            // read id
            if ((*it).size() == sizeof(uint32_t))
            {
                message.id = from_little_endian_unsafe<uint32_t>(
                    (*it).begin());
            }
            else
            {
                success = false;
            }

            it++;
        }

        if (success)
        {
            // read payload
            message.payload = (*it);
            it++;
        }

        receive(message);
    }
}

BCC_API period_ms obelisk_router::wakeup()
{
    period_ms next_wakeup(0);
    const auto now = std::chrono::steady_clock::now();

    auto i = pending_requests_.begin();
    while (i != pending_requests_.end())
    {
        auto request = i++;
        auto elapsed = std::chrono::duration_cast<period_ms>(
            now - request->second.last_action);
        if (timeout_ <= elapsed)
        {
            if (request->second.retries < retries_)
            {
                // Resend:
                ++request->second.retries;
                request->second.last_action = now;
                next_wakeup = min_sleep(next_wakeup, timeout_);
                send(request->second.message);
            }
            else
            {
                // Cancel:
                auto ec = std::make_error_code(std::errc::timed_out);
                request->second.on_error(ec);
                pending_requests_.erase(request);
            }
        }
        else
            next_wakeup = min_sleep(next_wakeup, timeout_ - elapsed);
    }
    return next_wakeup;
}

void obelisk_router::send_request(const std::string& command,
    const data_chunk& payload,
    error_handler on_error, decoder on_reply)
{
    const auto id = ++last_request_id_;
    auto& request = pending_requests_[id];
    request.message = obelisk_message{command, id, payload};
    request.on_error = std::move(on_error);
    request.on_reply = std::move(on_reply);
    request.retries = 0;
    request.last_action = std::chrono::steady_clock::now();
    send(request.message);
}

void obelisk_router::send(const obelisk_message& message)
{
    if (out_)
    {
        data_stack data;
        data.push_back(to_chunk(message.command));
        data.push_back(to_chunk(to_little_endian(message.id)));
        data.push_back(message.payload);
        out_->write(data);
    }
}

void obelisk_router::receive(const obelisk_message& message)
{
    if ("address.update" == message.command)
    {
        decode_update(message);
        return;
    }

    if ("address.stealth_update" == message.command)
    {
        decode_stealth_update(message);
        return;
    }

    const auto i = pending_requests_.find(message.id);
    if (i == pending_requests_.end())
    {
        on_unknown_(message.command);
        return;
    }

    decode_reply(message, i->second.on_error, i->second.on_reply);
    pending_requests_.erase(i);
}

void obelisk_router::decode_update(const obelisk_message& message)
{
    auto success = true;
    boost::iostreams::stream<byte_source<data_chunk>> istream(message.payload);
    istream_reader source(istream);

    // This message does not have an error_code at the beginning.
    const auto version_byte = source.read_byte();
    const auto address_hash = source.read_short_hash();
    const wallet::payment_address address(address_hash, version_byte);

    const auto height = source.read_4_bytes_little_endian();
    const auto block_hash = source.read_hash();
    chain::transaction tx;
    success = tx.from_data(source);

    if (success)
        success = source.is_exhausted();

    if (success)
        on_update_(address, height, block_hash, tx);
    else
        on_unknown_(message.command);
}

void obelisk_router::decode_stealth_update(const obelisk_message& message)
{
    auto success = true;
    boost::iostreams::stream<byte_source<data_chunk>> istream(message.payload);
    istream_reader source(istream);

    // This message does not have an error_code at the beginning.
    data_chunk raw_prefix;
    raw_prefix.push_back(source.read_byte());
    raw_prefix.push_back(source.read_byte());
    raw_prefix.push_back(source.read_byte());
    raw_prefix.push_back(source.read_byte());
    binary prefix(32, raw_prefix);

    const auto height = source.read_4_bytes_little_endian();
    const auto block_hash = source.read_hash();
    chain::transaction tx;

    success = tx.from_data(source);

    if (success)
        success = source.is_exhausted();

    if (success)
        on_stealth_update_(prefix, height, block_hash, tx);
    else
        on_unknown_(message.command);
}

void obelisk_router::decode_reply(const obelisk_message& message,
    error_handler& on_error, decoder& on_reply)
{
    code ec;
    boost::iostreams::stream<byte_source<data_chunk>> istream(message.payload);
    istream_reader source(istream);

    const auto value = source.read_4_bytes_little_endian();

    if (value != 0)
        ec = static_cast<error::error_code_t>(value);

    bool success = source;

    if (success)
    {
        success = on_reply(source);

        if (!success)
            ec = std::make_error_code(std::errc::bad_message);
    }

    if (ec)
        on_error(ec);
}

BCC_API void obelisk_router::on_unknown_nop(const std::string&)
{
}

BCC_API void obelisk_router::on_update_nop(const wallet::payment_address&,
    size_t, const hash_digest&, const chain::transaction&)
{
}

BCC_API void obelisk_router::on_stealth_update_nop(const binary&,
    size_t, const hash_digest&, const chain::transaction&)
{
}

} // namespace client
} // namespace libbitcoin
