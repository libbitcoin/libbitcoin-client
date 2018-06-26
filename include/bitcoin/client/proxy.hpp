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
#ifndef LIBBITCOIN_CLIENT_PROXY_HPP
#define LIBBITCOIN_CLIENT_PROXY_HPP

#include <cstdint>
#include <functional>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/client/dealer.hpp>
#include <bitcoin/client/define.hpp>
#include <bitcoin/client/history.hpp>
#include <bitcoin/client/stealth.hpp>
#include <bitcoin/client/stream.hpp>

namespace libbitcoin {
namespace client {

/// Decodes and encodes messages in the obelisk protocol.
/// This class is a pure proxy; it does not talk directly to zeromq.
class BCC_API proxy
  : public dealer
{
public:
    /// Resend is unrelated to connections.
    /// Timeout is capped at max_int32 (vs. max_uint32).
    proxy(stream& out, unknown_handler on_unknown_command,
        uint32_t timeout_milliseconds, uint8_t resends,
        const bc::settings& bitcoin_settings);

    // Fetch handler types.
    //-------------------------------------------------------------------------

    typedef std::function<void(size_t)> height_handler;
    typedef std::function<void(const code&)> result_handler;
    typedef std::function<void(size_t, size_t)> transaction_index_handler;
    typedef std::function<void(const chain::header&)> block_header_handler;
    typedef std::function<void(const chain::transaction&)> transaction_handler;
    typedef std::function<void(const chain::points_value&)> points_value_handler;
    typedef std::function<void(const client::history::list&)> history_handler;
    typedef std::function<void(const client::stealth::list&)> stealth_handler;

    // Fetchers.
    //-------------------------------------------------------------------------

    void transaction_pool_broadcast(error_handler on_error,
        result_handler on_reply, const chain::transaction& tx);

    void transaction_pool_validate2(error_handler on_error,
        result_handler on_reply, const chain::transaction& tx);

    void transaction_pool_fetch_transaction(error_handler on_error,
        transaction_handler on_reply, const hash_digest& tx_hash);

    void transaction_pool_fetch_transaction2(error_handler on_error,
        transaction_handler on_reply, const hash_digest& tx_hash);

    void blockchain_broadcast(error_handler on_error,
        result_handler on_reply, const chain::block& block);

    void blockchain_validate(error_handler on_error,
        result_handler on_reply, const chain::block& block);

    void blockchain_fetch_transaction(error_handler on_error,
        transaction_handler on_reply, const hash_digest& tx_hash);

    void blockchain_fetch_transaction2(error_handler on_error,
        transaction_handler on_reply, const hash_digest& tx_hash);

    void blockchain_fetch_last_height(error_handler on_error,
        height_handler on_reply);

    void blockchain_fetch_block_header(error_handler on_error,
        block_header_handler on_reply, uint32_t height);

    void blockchain_fetch_block_header(error_handler on_error,
        block_header_handler on_reply, const hash_digest& block_hash);

    void blockchain_fetch_transaction_index(error_handler on_error,
        transaction_index_handler on_reply, const hash_digest& tx_hash);

    void blockchain_fetch_stealth2(error_handler on_error,
        stealth_handler on_reply, const binary& prefix,
        uint32_t from_height=0);

    void blockchain_fetch_history3(error_handler on_error,
        history_handler on_reply, const wallet::payment_address& address,
        uint32_t from_height=0);

    void blockchain_fetch_unspent_outputs(error_handler on_error,
        points_value_handler on_reply, const wallet::payment_address& address,
        uint64_t satoshi, wallet::select_outputs::algorithm algorithm);

    // Subscribers.
    //-------------------------------------------------------------------------

    void subscribe_address(error_handler on_error, result_handler on_reply,
        const short_hash& address_hash);

    void subscribe_stealth(error_handler on_error, result_handler on_reply,
        const binary& stealth_prefix);

private:

    // Response handlers.
    //-------------------------------------------------------------------------
    static bool decode_empty(reader& payload, result_handler& handler);
    static bool decode_transaction(reader& payload,
        transaction_handler& handler);
    static bool decode_height(reader& payload, height_handler& handler);
    static bool decode_block_header(reader& payload,
        const bc::settings& bitcoin_settings, block_header_handler& handler);
    static bool decode_transaction_index(reader& payload,
        transaction_index_handler& handler);
    static bool decode_stealth(reader& payload, stealth_handler& handler);
    static bool decode_history(reader& payload, history_handler& handler);

    // Utilities.
    //-------------------------------------------------------------------------
    static stealth::list expand(chain::stealth_record::list& record);
    static history::list expand(chain::payment_record::list& record);

    const bc::settings& bitcoin_settings_;
};

} // namespace client
} // namespace libbitcoin

#endif
