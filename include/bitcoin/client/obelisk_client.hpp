/**
 * Copyright (c) 2011-2019 libbitcoin developers (see AUTHORS)
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

#include <bitcoin/system.hpp>
#include <bitcoin/client/define.hpp>
#include <bitcoin/client/history.hpp>
#include <bitcoin/client/stealth.hpp>
#include <bitcoin/protocol.hpp>

namespace libbitcoin {
namespace client {

/// Structure used for passing connection settings for a server.
struct BCC_API connection_settings
{
    int32_t retries;
    system::config::endpoint server;
    system::config::endpoint block_server;
    system::config::endpoint transaction_server;
    system::config::authority socks;
    system::config::sodium server_public_key;
    system::config::sodium client_private_key;
};

/// Client implements a router-dealer interface to communicate with
/// the server over either public or secure sockets.
class BCC_API obelisk_client
{
public:
    static const auto null_subscription = bc::max_uint32;

    typedef std::function<void(const std::string&, uint32_t,
        const system::data_chunk&)> command_handler;
    typedef std::unordered_map<std::string, command_handler> command_map;

    // Subscription/notification handler types.
    //-------------------------------------------------------------------------

    typedef std::function<void(const system::code&, uint16_t, size_t,
        const system::hash_digest&)> update_handler;
    typedef std::function<void(const system::chain::block&)>
        block_update_handler;
    typedef std::function<void(const system::chain::transaction&)>
        transaction_update_handler;

    // Fetch handler types.
    //-------------------------------------------------------------------------

    typedef std::function<void(const system::code&)> result_handler;
    typedef std::function<void(const system::code&, size_t)> height_handler;
    typedef std::function<void(const system::code&, size_t, size_t)> transaction_index_handler;
    typedef std::function<void(const system::code&, const system::chain::block&)> block_handler;
    typedef std::function<void(const system::code&, const system::chain::header&)> block_header_handler;
    typedef std::function<void(const system::code&, const system::chain::transaction&)> transaction_handler;
    typedef std::function<void(const system::code&, const system::chain::points_value&)> points_value_handler;
    typedef std::function<void(const system::code&, const client::history::list&)> history_handler;
    typedef std::function<void(const system::code&, const client::stealth::list&)> stealth_handler;
    typedef std::function<void(const system::code&, const system::hash_list&)> hash_list_handler;
    typedef std::function<void(const system::code&, const std::string&)> version_handler;

    // Used for mapping specific requests to specific handlers
    // (allowing support for different handlers for different client
    // API calls on a per-client instance basis).
    typedef std::unordered_map<uint32_t, result_handler> result_handler_map;
    typedef std::unordered_map<uint32_t, height_handler> height_handler_map;
    typedef std::unordered_map<uint32_t, transaction_index_handler> transaction_index_handler_map;
    typedef std::unordered_map<uint32_t, block_handler> block_handler_map;
    typedef std::unordered_map<uint32_t, block_header_handler> block_header_handler_map;
    typedef std::unordered_map<uint32_t, transaction_handler> transaction_handler_map;
    typedef std::unordered_map<uint32_t, history_handler> history_handler_map;
    typedef std::unordered_map<uint32_t, stealth_handler> stealth_handler_map;
    typedef std::unordered_map<uint32_t, std::pair<update_handler,
        system::data_chunk>> subscription_handler_map;
    typedef std::unordered_map<uint32_t, std::pair<result_handler,
        uint32_t>> unsubscription_handler_map;
    typedef std::unordered_map<uint32_t, hash_list_handler> hash_list_handler_map;
    typedef std::unordered_map<uint32_t, version_handler> version_handler_map;

    /// Construct an instance of the client.
    obelisk_client(int32_t retries=5);

    ~obelisk_client();

    /// Connect to the specified endpoint using the provided keys.
    bool connect(const system::config::endpoint& address,
        const system::config::authority& socks_proxy,
        const system::config::sodium& server_public_key,
        const system::config::sodium& client_private_key);

    /// Connect to the specified endpoint.
    bool connect(const system::config::endpoint& address);

    /// Connect using the provided settings.
    bool connect(const connection_settings& settings);

    /// Wait for server to respond to queries, until timeout.
    void wait(uint32_t timeout_milliseconds=30000);

    /// Monitor for subscription notifications, until timeout.
    void monitor(uint32_t timeout_milliseconds=30000);

    // Fetchers.
    //-------------------------------------------------------------------------

    void server_version(version_handler handler);

    void transaction_pool_broadcast(result_handler handler,
        const system::chain::transaction& tx);

    void transaction_pool_validate2(result_handler handler,
        const system::chain::transaction& tx);

    void transaction_pool_fetch_transaction(transaction_handler handler,
        const system::hash_digest& tx_hash);

    void transaction_pool_fetch_transaction2(transaction_handler handler,
        const system::hash_digest& tx_hash);

    void blockchain_broadcast(result_handler handler,
        const system::chain::block& block);

    void blockchain_validate(result_handler handler,
        const system::chain::block& block);

    void blockchain_fetch_transaction(transaction_handler handler,
        const system::hash_digest& tx_hash);

    void blockchain_fetch_transaction2(transaction_handler handler,
        const system::hash_digest& tx_hash);

    void blockchain_fetch_last_height(height_handler handler);

    void blockchain_fetch_block(block_handler handler, uint32_t height);

    void blockchain_fetch_block(block_handler handler,
        const system::hash_digest& block_hash);

    void blockchain_fetch_block_header(block_header_handler handler,
        uint32_t height);

    void blockchain_fetch_block_header(block_header_handler handler,
        const system::hash_digest& block_hash);

    void blockchain_fetch_transaction_index(transaction_index_handler handler,
        const system::hash_digest& tx_hash);

    void blockchain_fetch_block_height(height_handler handler,
        const system::hash_digest& block_hash);

    void blockchain_fetch_block_transaction_hashes(
        hash_list_handler handler, uint32_t height);

    void blockchain_fetch_block_transaction_hashes(
        hash_list_handler handler, const system::hash_digest& block_hash);

    void blockchain_fetch_stealth_transaction_hashes(
        hash_list_handler handler, uint32_t height);

    void blockchain_fetch_stealth_transaction_hashes(
        hash_list_handler handler, const system::hash_digest& block_hash);

    void blockchain_fetch_history4(history_handler handler,
        const system::hash_digest& key, uint32_t from_height=0);

    void blockchain_fetch_stealth2(stealth_handler handler,
        const system::binary& prefix, uint32_t from_height=0);

    void blockchain_fetch_unspent_outputs(points_value_handler handler,
        const system::hash_digest& key, uint64_t satoshi,
        system::wallet::select_outputs::algorithm algorithm);

    // Subscribers.
    //-------------------------------------------------------------------------

    // Subscribe to a payment key.  Return value can be used to unsubscribe.
    uint32_t subscribe_key(update_handler handler,
        const system::hash_digest& key);

    // Subscribe to a stealth prefix.  Return value can be used to unsubscribe.
    uint32_t subscribe_stealth(update_handler handler,
        const system::binary& stealth_prefix);

    bool subscribe_block(const system::config::endpoint& address,
        block_update_handler on_update);

    bool subscribe_transaction(const system::config::endpoint& address,
        transaction_update_handler on_update);

    // Unsubscribers.
    //-------------------------------------------------------------------------

    bool unsubscribe_key(result_handler handler, uint32_t subscription);

    bool unsubscribe_stealth(result_handler handler, uint32_t subscription);

private:
    // Attach handlers for all supported client-server operations.
    void attach_handlers();

    // Used to handle a request immediately, on early detection of error.
    void handle_immediate(const std::string& command, uint32_t id,
        const system::code& ec);

    // Determines if any requests have not been handled.
    bool requests_outstanding();

    // Determines if any notification requests have not been handled.
    bool subscribe_requests_outstanding();

    // Calls all remaining/expired handlers with the specified error.
    void clear_outstanding_requests(const system::code& ec);

    // Calls all remaining/expired notification handlers with the specified
    // error.
    void clear_outstanding_subscribe_requests(const system::code& ec);

    // Sends an outgoing request via the internal router.
    bool send_request(const std::string& command, uint32_t id,
        const system::data_chunk& payload, bool subscription=false);

    // Forward incoming client router requests to the server.
    void forward_message(protocol::zmq::socket& source,
        protocol::zmq::socket& sink);

    // Process server responses.
    void process_response(protocol::zmq::socket& socket);

    // After notifying the server of unsubscribe, this terminates any client
    // side monitoring state for the subscription.
    bool terminate_unsubscriber(uint32_t subscription);

    protocol::zmq::context context_;

    // Sockets that connect to external libbitcoin services.
    protocol::zmq::socket socket_;
    protocol::zmq::socket subscribe_socket_;
    protocol::zmq::socket block_socket_;
    protocol::zmq::socket transaction_socket_;

    // Internal socket pair for client request forwarding to router
    // (that then forwards to server).
    protocol::zmq::socket dealer_;
    protocol::zmq::socket router_;

    // Internal socket pair for client subscription request forwarding to router
    // (that then forwards to server).
    protocol::zmq::socket subscribe_dealer_;
    protocol::zmq::socket subscribe_router_;

    block_update_handler on_block_update_;
    transaction_update_handler on_transaction_update_;
    int32_t retries_;
    bool secure_;
    system::config::endpoint worker_;
    system::config::endpoint subscribe_worker_;
    uint32_t last_request_index_;
    command_map command_handlers_;
    result_handler_map result_handlers_;
    height_handler_map height_handlers_;
    transaction_index_handler_map transaction_index_handlers_;
    block_handler_map block_handlers_;
    block_header_handler_map block_header_handlers_;
    transaction_handler_map transaction_handlers_;
    history_handler_map history_handlers_;
    stealth_handler_map stealth_handlers_;
    subscription_handler_map subscription_handlers_;
    unsubscription_handler_map unsubscription_handlers_;
    hash_list_handler_map hash_list_handlers_;
    version_handler_map version_handlers_;

    // Protects subscription_handlers_
    system::upgrade_mutex subscription_lock_;
};

} // namespace client
} // namespace libbitcoin

#endif
