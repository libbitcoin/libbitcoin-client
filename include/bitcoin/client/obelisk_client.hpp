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
#ifndef LIBBITCOIN_CLIENT_OBELISK_CLIENT_HPP
#define LIBBITCOIN_CLIENT_OBELISK_CLIENT_HPP

#include <cstdint>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/protocol.hpp>
#include <bitcoin/client/define.hpp>
#include <bitcoin/client/proxy.hpp>
#include <bitcoin/client/socket_stream.hpp>

namespace libbitcoin {
namespace client {

/// Structure used for passing connection settings for a server.
struct BCC_API connection_type
{
    uint8_t retries;
    uint16_t timeout_seconds;
    config::endpoint server;
    config::endpoint block_server;
    config::endpoint transaction_server;
    config::authority socks;
    config::sodium server_public_key;
    config::sodium client_private_key;
};

/// Client proxy with session management.
class BCC_API obelisk_client
  : public proxy
{
public:
    typedef std::function<void(const chain::block&)> block_update_handler;
    typedef std::function<void(const chain::transaction&)>
        transaction_update_handler;

    /// Construct an instance of the client using timeout/retries from channel.
    obelisk_client(const connection_type& channel,
        const bc::settings& bitcoin_settings);

    /// Construct an instance of the client using the specified parameters.
    obelisk_client(uint16_t timeout_seconds, uint8_t retries,
        const bc::settings& bitcoin_settings);

    /// Connect to the specified endpoint using the provided keys.
    virtual bool connect(const config::endpoint& address,
        const config::authority& socks_proxy,
        const config::sodium& server_public_key,
        const config::sodium& client_private_key);

    /// Connect to the specified endpoint.
    virtual bool connect(const config::endpoint& address);

    /// Connect to the specified endpoint using the provided channel config.
    virtual bool connect(const connection_type& channel);

    /// Wait for server to respond, until timeout.
    void wait();

    bool subscribe_block(const config::endpoint& address,
        block_update_handler on_update);

    bool subscribe_transaction(const config::endpoint& address,
        transaction_update_handler on_update);

    /// Monitor for subscription notifications, until timeout.
    void monitor(uint32_t timeout_seconds);

private:
    protocol::zmq::context context_;
    protocol::zmq::socket socket_;
    protocol::zmq::socket block_socket_;
    protocol::zmq::socket transaction_socket_;
    block_update_handler on_block_update_;
    transaction_update_handler on_transaction_update_;
    socket_stream stream_;
    const uint8_t retries_;
    const bc::settings& bitcoin_settings_;
};

} // namespace client
} // namespace libbitcoin

#endif
