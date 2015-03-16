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

#ifndef LIBBITCOIN_CLIENT_SERVER_CODEC_IMPL_HPP
#define LIBBITCOIN_CLIENT_SERVER_CODEC_IMPL_HPP

#include <bitcoin/bitcoin.hpp>
#include <bitcoin/protocol.hpp>
#include <bitcoin/client/define.hpp>
#include <bitcoin/client/random_number_generator.hpp>
#include <bitcoin/client/response_stream.hpp>
#include <bitcoin/client/request_stream.hpp>
#include <bitcoin/client/server_codec.hpp>
#include <bitcoin/client/sleeper.hpp>

namespace libbitcoin {
namespace client {

class server_codec_impl
  : public response_stream, public server_codec, public sleeper
{
public:

    typedef std::function<void (
        const bc::protocol::response&)> response_handler;

    BCC_API static void on_unknown_nop(const bc::protocol::response&);

    BCC_API server_codec_impl(
        std::shared_ptr<request_stream> outgoing,
        std::shared_ptr<random_number_generator<uint32_t>> generator,
        period_ms timeout = std::chrono::seconds(2),
        uint8_t retries = 0,
        const response_handler& on_unknown_response = on_unknown_nop);

    BCC_API virtual ~server_codec_impl();

    BCC_API virtual void set_retries(uint8_t retries);

    BCC_API virtual void set_timeout(period_ms timeout);

    BCC_API virtual void set_unknown_response_handler(
        response_handler& handler);

    BCC_API virtual uint64_t outstanding_call_count() const;

    BCC_API virtual void write(const bc::protocol::response& response);

    // response_stream interface
    BCC_API virtual void write(
        const std::shared_ptr<bc::protocol::response>& response);

    // sleeper interface
    BCC_API virtual period_ms wakeup() override;

    // server_codec interface
    BCC_API virtual void get_block_headers(
        error_handler&& on_error,
        block_headers_handler&& on_reply,
        std::shared_ptr<bc::protocol::block_id> start
            = std::shared_ptr<bc::protocol::block_id>(),
        uint32_t results_per_page = MAX_UINT32);

    BCC_API  virtual void post_block(
        error_handler&& on_error,
        post_handler&& on_reply,
        const bc::protocol::block& block);

    BCC_API virtual void validate_block(
        error_handler&& on_error,
        validate_handler&& on_reply,
        const bc::protocol::block& block);

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
        uint32_t results_per_page = MAX_UINT32);

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
        uint32_t results_per_page = MAX_UINT32);

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
        uint32_t results_per_page = MAX_UINT32);

    BCC_API virtual void post_transaction(
        error_handler&& on_error,
        post_handler&& on_reply,
        const bc::protocol::tx& transaction);

    BCC_API virtual void validate_transaction(
        error_handler&& on_error,
        validate_handler&& on_reply,
        const bc::protocol::tx& transaction);

protected:

    typedef std::function<void (
        const bc::protocol::response&, error_handler&)> response_interpreter;

    BCC_API virtual void send(
        error_handler&& on_error,
        response_interpreter&& on_reply,
        const std::shared_ptr<bc::protocol::request>& request);

    BCC_API virtual void broadcast_request(
        const std::shared_ptr<bc::protocol::request>& request);

private:

    bc::protocol::transactions_request* get_transactions_request(
        const bc::protocol::filter_list& query,
        bc::protocol::transaction_results result_type
            = bc::protocol::transaction_results::TX_HASH,
        bc::protocol::locations location_type
            = bc::protocol::locations::NONE,
        std::shared_ptr<bc::protocol::block_id> start
            = std::shared_ptr<bc::protocol::block_id>(),
        uint32_t results_per_page = MAX_UINT32);

    static void handle_block_headers(
        const bc::protocol::response& response,
        error_handler& on_error,
        block_headers_handler& on_reply);

    static void handle_post_block(
        const bc::protocol::response& response,
        error_handler& on_error,
        post_handler& on_reply);

    static void handle_validate_block(
        const bc::protocol::response& response,
        error_handler& on_error,
        validate_handler& on_reply);

    static void handle_transactions(
        const bc::protocol::response& response,
        error_handler& on_error,
        transaction_results_handler& on_reply);

    static void handle_transaction_hashes(
        const bc::protocol::response& response,
        error_handler& on_error,
        transaction_hash_results_handler& on_reply);

    static void handle_utxos(
        const bc::protocol::response& response,
        error_handler& on_error,
        utxo_results_handler& on_reply);

    static void handle_post_transaction(
        const bc::protocol::response& response,
        error_handler& on_error,
        post_handler& on_reply);

    static void handle_validate_transaction(
        const bc::protocol::response& response,
        error_handler& on_error,
        validate_handler& on_reply);


    // Request management:
    struct pending_request
    {
        std::shared_ptr<bc::protocol::request> request;
        error_handler on_error;
        response_interpreter on_reply;
        uint8_t retries;
        std::chrono::steady_clock::time_point last_action;
    };

    std::map<uint32_t, pending_request> pending_requests_;

    // Outgoing message stream:
    std::shared_ptr<request_stream> stream_;

    // Generator:
    std::shared_ptr<random_number_generator<uint32_t>> generator_;

    // Timeout parameters:
    period_ms timeout_;
    uint8_t retries_;

    // Unknown response handler:
    response_handler on_unknown_response_;
};

}
}

#endif
