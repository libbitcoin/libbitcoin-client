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
#include <bitcoin/client/obelisk_client.hpp>

#include <algorithm>
#include <cstdint>
#include <thread>
#include <bitcoin/protocol.hpp>

using namespace bc;
using namespace bc::config;
using namespace bc::protocol;
using namespace std::this_thread;

namespace libbitcoin {
namespace client {

static uint32_t to_milliseconds(uint16_t seconds)
{
    const auto milliseconds = static_cast<uint32_t>(seconds) * 1000;
    return std::min(milliseconds, max_uint32);
}

static const auto on_unknown = [](const std::string&){};

// TODO: eliminate the stream, using server model.
// TODO: eliminate dual return handlers in favor of always return code.
// The client is a DEALER so that it can receive asynchronous replies.
// A REQ client could only process queries, not subscriptions.

// TODO: the client should broker requests from multiple requesters.
// TODO: IOW it should connect as reqs=>router->dealer->(server router).
// TODO: this eliminates the hack client::dealer implementation.

// Retries is overloaded as configuration for resends as well.
// Timeout is capped at ~ 25 days by signed/millsecond conversions.
obelisk_client::obelisk_client(uint16_t timeout_seconds, uint8_t retries,
    const bc::settings& bitcoin_settings)
  : socket_(context_, zmq::socket::role::dealer),
    block_socket_(context_, zmq::socket::role::subscriber),
    transaction_socket_(context_, zmq::socket::role::subscriber),
    stream_(socket_),
    retries_(retries),
    bitcoin_settings_(bitcoin_settings),
    proxy(stream_, on_unknown, to_milliseconds(timeout_seconds), retries,
        bitcoin_settings)
{
}

obelisk_client::obelisk_client(const connection_type& channel,
    const bc::settings& bitcoin_settings)
  : obelisk_client(channel.timeout_seconds, channel.retries, bitcoin_settings)
{
}

bool obelisk_client::connect(const connection_type& channel)
{
    return connect(channel.server, channel.socks, channel.server_public_key,
        channel.client_private_key);
}

bool obelisk_client::connect(const endpoint& address,
    const authority& socks_proxy, const sodium& server_public_key,
    const sodium& client_private_key)
{
    // Only apply the client (and server) key if server key is configured.
    if (server_public_key)
    {
        if (!socket_.set_curve_client(server_public_key))
            return false;

        // Generates arbitrary client keys if private key is not configured.
        if (!socket_.set_certificate({ client_private_key }))
            return false;
    }

    // Ignore the setting if socks.port is zero (invalid).
    if (socks_proxy && !socket_.set_socks_proxy(socks_proxy))
        return false;

    return connect(address);
}

bool obelisk_client::connect(const endpoint& address)
{
    const auto host_address = address.to_string();

    for (auto attempt = 0; attempt < 1 + retries_; ++attempt)
    {
        if (socket_.connect(host_address) == error::success)
            return true;

        // Arbitrary delay between connection attempts.
        sleep_for(asio::milliseconds(100));
    }

    return false;
}

// Used by fetch-* commands, fires reply, unknown or error handlers.
void obelisk_client::wait()
{
    zmq::poller poller;
    poller.add(socket_);
    auto delay = refresh();

    while (!empty() && poller.wait(delay).contains(socket_.id()))
    {
        stream_.read(*this);
        delay = refresh();
    }

    // Invoke error handlers for any still pending.
    clear(error::channel_timeout);
}

bool obelisk_client::subscribe_block(const config::endpoint& address,
    block_update_handler on_update)
{
    const auto host_address = address.to_string();
    if (block_socket_.connect(host_address) == error::success)
    {
        on_block_update_ = on_update;
        return true;
    }

    return false;
}

bool obelisk_client::subscribe_transaction(
    const config::endpoint& address, transaction_update_handler on_update)
{
    const auto host_address = address.to_string();
    if (transaction_socket_.connect(host_address) == error::success)
    {
        on_transaction_update_ = on_update;
        return true;
    }

    return false;
}

// Used by watch-* and subscribe-* commands, fires registered update
// or unknown handlers.
void obelisk_client::monitor(uint32_t timeout_seconds)
{
    const auto deadline = asio::steady_clock::now() +
        asio::seconds(timeout_seconds);

    zmq::poller poller;
    poller.add(socket_);
    poller.add(block_socket_);
    poller.add(transaction_socket_);
    auto delay = remaining(deadline);

    while (delay > 0)
    {
        const auto identifiers = poller.wait(delay);
        if (identifiers.contains(socket_.id()))
        {
            stream_.read(*this);
        }

        if (identifiers.contains(block_socket_.id()))
        {
            zmq::message message;
            uint16_t sequence;
            uint32_t height;
            data_chunk data;

            block_socket_.receive(message);

            message.dequeue(sequence);
            message.dequeue(height);
            message.dequeue(data);

            bc::chain::block block(bitcoin_settings_);
            block.from_data(data, true);

            on_block_update_(block);
        }

        if (identifiers.contains(transaction_socket_.id()))
        {
            zmq::message message;
            uint16_t sequence;
            data_chunk data;

            transaction_socket_.receive(message);

            message.dequeue(sequence);
            message.dequeue(data);

            bc::chain::transaction transaction;
            transaction.from_data(data, true, true);

            on_transaction_update_(transaction);
        }

        delay = remaining(deadline);
    }
}

} // namespace client
} // namespace libbitcoin
