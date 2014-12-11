/*
 * Copyright (c) 2011-2014 libbitcoin developers (see AUTHORS)
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

#ifndef LIBBITCOIN_CLIENT_SERVER_CODEC_HPP
#define LIBBITCOIN_CLIENT_SERVER_CODEC_HPP

#include <functional>
#include <memory>
#include <bitcoin/protocol.hpp>
#include <bitcoin/client/define.hpp>

namespace libbitcoin {
namespace client {

class server_codec
{
public:

    // Message reply handlers:
    typedef std::function<void (const std::error_code&)> error_handler;

    typedef std::function<void (bool)> post_handler;

    typedef std::function<void (bool)> validate_handler;

    typedef std::function<void (const bc::protocol::block_header_list& headers,
        const bc::protocol::block_id& next,
        const bc::protocol::block_id& top)> block_headers_handler;

//    typedef std::function<void (const bc::protocol::response_block_headers&)>
//        block_headers_handler;

    typedef std::function<void (
        const bc::protocol::transaction_result_list& transactions,
        const bc::protocol::block_id& next,
        const bc::protocol::block_id& top)> transaction_results_handler;

    typedef std::function<void (
        const bc::protocol::transaction_hash_result_list& hashes,
        const bc::protocol::block_id& next,
        const bc::protocol::block_id& top)> transaction_hash_results_handler;

    typedef std::function<void (
        const bc::protocol::utxo_result_list& utxos,
        const bc::protocol::block_id& next,
        const bc::protocol::block_id& top)> utxo_results_handler;

    BCC_API virtual ~server_codec() {};

    // Message calls:
    BCC_API virtual void get_block_headers(
        error_handler&& on_error,
        block_headers_handler&& on_reply,
        std::shared_ptr<bc::protocol::block_id> start
            = std::shared_ptr<bc::protocol::block_id>(),
        uint32_t results_per_page = MAX_UINT32) = 0;

    BCC_API virtual void post_block(
        error_handler&& on_error,
        post_handler&& on_reply,
        const bc::protocol::block& block) = 0;

    BCC_API virtual void validate_block(
        error_handler&& on_error,
        validate_handler&& on_reply,
        const bc::protocol::block& block) = 0;

    BCC_API virtual void get_transactions(
        error_handler&& on_error,
        transaction_results_handler&& on_reply,
        const bc::protocol::filter_list& query,
        bc::protocol::transaction_results result_type
            = bc::protocol::transaction_results::TX_HASH,
        bc::protocol::locations location_type
            = bc::protocol::locations::NONE,
        std::shared_ptr<bc::protocol::block_id> start
            = std::shared_ptr<bc::protocol::block_id>(),
        uint32_t results_per_page = MAX_UINT32) = 0;

    BCC_API virtual void get_transaction_hashes(
        error_handler&& on_error,
        transaction_hash_results_handler&& on_reply,
        const bc::protocol::filter_list& query,
        bc::protocol::transaction_results result_type
            = bc::protocol::transaction_results::TX_HASH,
        bc::protocol::locations location_type
            = bc::protocol::locations::NONE,
        std::shared_ptr<bc::protocol::block_id> start
            = std::shared_ptr<bc::protocol::block_id>(),
        uint32_t results_per_page = MAX_UINT32) = 0;

    BCC_API virtual void get_utxos(
        error_handler&& on_error,
        utxo_results_handler&& on_reply,
        const bc::protocol::filter_list& query,
        bc::protocol::transaction_results result_type
            = bc::protocol::transaction_results::TX_HASH,
        bc::protocol::locations location_type
            = bc::protocol::locations::NONE,
        std::shared_ptr<bc::protocol::block_id> start
            = std::shared_ptr<bc::protocol::block_id>(),
        uint32_t results_per_page = MAX_UINT32) = 0;

    BCC_API virtual void post_transaction(
        error_handler&& on_error,
        post_handler&& on_reply,
        const bc::protocol::tx& transaction) = 0;

    BCC_API virtual void validate_transaction(
        error_handler&& on_error,
        validate_handler&& on_reply,
        const bc::protocol::tx& transaction) = 0;
};

}
}

#endif

