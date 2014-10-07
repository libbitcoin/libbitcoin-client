/*
 * Copyright (c) 2011-2014 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin_client.
 *
 * libbitcoin_client is free software: you can redistribute it and/or
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
#include <bitcoin/client/server_codec_impl.hpp>

namespace libbitcoin {
namespace client {

void server_codec_impl::on_unknown_nop(const bc::protocol::response&)
{
}

server_codec_impl::server_codec_impl(
    std::shared_ptr<request_stream> outgoing,
    std::shared_ptr<random_number_generator<uint32_t>> generator,
    period_ms timeout,
    uint8_t retries,
    const response_handler& on_unknown_response)
: stream_(outgoing), generator_(generator), timeout_(timeout),
  retries_(retries), on_unknown_response_(on_unknown_response)
{
}

server_codec_impl::~server_codec_impl()
{
}

void server_codec_impl::set_retries(uint8_t retries)
{
    retries_ = retries;
}

void server_codec_impl::set_timeout(period_ms timeout)
{
    timeout_ = timeout;
}

void server_codec_impl::set_unknown_response_handler(response_handler& handler)
{
    on_unknown_response_ = handler;
}

uint64_t server_codec_impl::outstanding_call_count() const
{
    return pending_requests_.size();
}

// response_stream interface
void server_codec_impl::write(const bc::protocol::response& response)
{
    auto entry = pending_requests_.find(response.id());

    if (entry != pending_requests_.end())
    {
        entry->second.on_reply(response, entry->second.on_error);
        pending_requests_.erase(entry);
    }
    else
    {
        on_unknown_response_(response);
    }
}

// sleeper interface
period_ms server_codec_impl::wakeup(bool enable_sideeffects)
{
    period_ms next_wakeup(0);
    auto now = std::chrono::steady_clock::now();

    auto i = pending_requests_.begin();
    while (i != pending_requests_.end())
    {
        auto request = i++;
        auto elapsed = std::chrono::duration_cast<period_ms>(
            now - request->second.last_action);

        if (timeout_ <= elapsed)
        {
            if (enable_sideeffects)
            {
                if (request->second.retries < retries_)
                {
                    // Resend:
                    ++request->second.retries;
                    request->second.last_action = now;
                    next_wakeup = min_sleep(next_wakeup, timeout_);
                    broadcast_request(request->second.request);
                }
                else
                {
                    // Cancel:
                    auto ec = std::make_error_code(std::errc::timed_out);
                    request->second.on_error(ec);
                    pending_requests_.erase(request);
                }
            }
        }
        else
        {
            next_wakeup = min_sleep(next_wakeup, timeout_ - elapsed);
        }
    }
    return next_wakeup;
}

// server_codec interface
void server_codec_impl::get_block_headers(
    error_handler&& on_error,
    block_headers_handler&& on_reply,
    std::shared_ptr<bc::protocol::block_id> start,
    uint32_t results_per_page)
{
    std::unique_ptr<bc::protocol::block_headers_request> get_block_headers(
        new bc::protocol::block_headers_request());

    if (start)
    {
        std::unique_ptr<bc::protocol::block_id> starting_block(
            new bc::protocol::block_id(*(start.get())));

        get_block_headers->set_allocated_start(starting_block.release());
    }

    if (results_per_page < MAX_UINT32)
    {
        get_block_headers->set_results_per_page(results_per_page);
    }

    std::shared_ptr<bc::protocol::request> request
        = std::make_shared<bc::protocol::request>();

    request->set_allocated_get_block_headers(
        get_block_headers.release());

    send(
        std::move(on_error),
        std::bind(
            handle_block_headers,
            std::placeholders::_1,
            std::placeholders::_2,
            std::move(on_reply)),
        request);
}

void server_codec_impl::post_block(
    error_handler&& on_error,
    post_handler&& on_reply,
    const bc::protocol::block& block)
{
    std::unique_ptr<bc::protocol::block> post_block(
        new bc::protocol::block(block));

    std::shared_ptr<bc::protocol::request> request
        = std::make_shared<bc::protocol::request>();

    request->set_allocated_post_block(
        post_block.release());

    send(
        std::move(on_error),
        std::bind(
            handle_post_block,
            std::placeholders::_1,
            std::placeholders::_2,
            std::move(on_reply)),
        request);
}

void server_codec_impl::validate_block(
    error_handler&& on_error,
    validate_handler&& on_reply,
    const bc::protocol::block& block)
{
    std::unique_ptr<bc::protocol::block> validate_block(
        new bc::protocol::block(block));

    std::shared_ptr<bc::protocol::request> request
        = std::make_shared<bc::protocol::request>();

    request->set_allocated_validate_block(
        validate_block.release());

    send(
        std::move(on_error),
        std::bind(
            handle_validate_block,
            std::placeholders::_1,
            std::placeholders::_2,
            std::move(on_reply)),
        request);
}

void server_codec_impl::get_transactions(
    error_handler&& on_error,
    transaction_results_handler&& on_reply,
    const bc::protocol::filter_list& query,
    bc::protocol::transaction_results result_type,
    bc::protocol::locations location_type,
    std::shared_ptr<bc::protocol::block_id> start,
    uint32_t results_per_page)
{
    std::shared_ptr<bc::protocol::request> request
        = std::make_shared<bc::protocol::request>();

    request->set_allocated_get_transactions(
        get_transactions_request(
            query,
            result_type,
            location_type,
            start,
            results_per_page));

    send(
        std::move(on_error),
        std::bind(
            handle_transactions,
            std::placeholders::_1,
            std::placeholders::_2,
            std::move(on_reply)),
        request);
}

void server_codec_impl::get_transaction_hashes(
    error_handler&& on_error,
    transaction_hash_results_handler&& on_reply,
    const bc::protocol::filter_list& query,
    bc::protocol::transaction_results result_type,
    bc::protocol::locations location_type,
    std::shared_ptr<bc::protocol::block_id> start,
    uint32_t results_per_page)
{
    std::shared_ptr<bc::protocol::request> request
        = std::make_shared<bc::protocol::request>();

    request->set_allocated_get_transactions(
        get_transactions_request(
            query,
            result_type,
            location_type,
            start,
            results_per_page));

    send(
        std::move(on_error),
        std::bind(
            handle_transaction_hashes,
            std::placeholders::_1,
            std::placeholders::_2,
            std::move(on_reply)),
        request);
}

void server_codec_impl::get_utxos(
    error_handler&& on_error,
    utxo_results_handler&& on_reply,
    const bc::protocol::filter_list& query,
    bc::protocol::transaction_results result_type,
    bc::protocol::locations location_type,
    std::shared_ptr<bc::protocol::block_id> start,
    uint32_t results_per_page)
{
    std::shared_ptr<bc::protocol::request> request
        = std::make_shared<bc::protocol::request>();

    request->set_allocated_get_transactions(
        get_transactions_request(
            query,
            result_type,
            location_type,
            start,
            results_per_page));

    send(
        std::move(on_error),
        std::bind(
            handle_utxos,
            std::placeholders::_1,
            std::placeholders::_2,
            std::move(on_reply)),
        request);
}

void server_codec_impl::post_transaction(
    error_handler&& on_error,
    post_handler&& on_reply,
    const bc::protocol::tx& transaction)
{
    std::unique_ptr<bc::protocol::tx> post_transaction(
        new bc::protocol::tx(transaction));

    std::shared_ptr<bc::protocol::request> request
        = std::make_shared<bc::protocol::request>();

    request->set_allocated_post_transaction(
        post_transaction.release());

    send(
        std::move(on_error),
        std::bind(
            handle_post_transaction,
            std::placeholders::_1,
            std::placeholders::_2,
            std::move(on_reply)),
        request);
}

void server_codec_impl::validate_transaction(
    error_handler&& on_error,
    validate_handler&& on_reply,
    const bc::protocol::tx& transaction)
{
    std::unique_ptr<bc::protocol::tx> validate_transaction(
        new bc::protocol::tx(transaction));

    std::shared_ptr<bc::protocol::request> request
        = std::make_shared<bc::protocol::request>();

    request->set_allocated_validate_transaction(
        validate_transaction.release());

    send(
        std::move(on_error),
        std::bind(
            handle_validate_transaction,
            std::placeholders::_1,
            std::placeholders::_2,
            std::move(on_reply)),
        request);
}

void server_codec_impl::send(
    error_handler&& on_error,
    response_interpreter&& on_reply,
    const std::shared_ptr<bc::protocol::request>& request)
{
    // set unique message identifier
    uint32_t id = (generator_) ? (*generator_.get())() : pending_requests_.size();

    // create pending request entry

    auto it = pending_requests_.find(id);

    while (it != pending_requests_.end())
    {
        id = (id < MAX_UINT32) ? id + 1 : 0;
        pending_requests_.find(id);
    }

    pending_request& entry = pending_requests_[id];
    entry.request = request;
    entry.on_error = std::move(on_error);
    entry.on_reply = std::move(on_reply);
    entry.retries = 0;
    entry.last_action = std::chrono::steady_clock::now();

    // send request
    broadcast_request(request);
}

void server_codec_impl::broadcast_request(
    const std::shared_ptr<bc::protocol::request>& request)
{
    if (request && stream_)
    {
        stream_->write(request);
    }
}

void server_codec_impl::handle_block_headers(
    const bc::protocol::response& response,
    error_handler& on_error,
    block_headers_handler& on_reply)
{
    if (response.has_get_block_headers_response())
    {
        const bc::protocol::response_block_headers& payload
            = response.get_block_headers_response();

        on_reply(payload.headers(), payload.next(), payload.top());
    }
    else
    {
        on_error(std::make_error_code(std::errc::bad_message));
    }
}

void server_codec_impl::handle_post_block(
    const bc::protocol::response& response,
    error_handler& on_error,
    post_handler& on_reply)
{
    if (response.has_post_block_succeeded())
    {
        on_reply(response.post_block_succeeded());
    }
    else
    {
        on_error(std::make_error_code(std::errc::bad_message));
    }
}

void server_codec_impl::handle_validate_block(
    const bc::protocol::response& response,
    error_handler& on_error,
    validate_handler& on_reply)
{
    if (response.has_validate_block_succeeded())
    {
        on_reply(response.validate_block_succeeded());
    }
    else
    {
        on_error(std::make_error_code(std::errc::bad_message));
    }
}

void server_codec_impl::handle_transactions(
    const bc::protocol::response& response,
    error_handler& on_error,
    transaction_results_handler& on_reply)
{
    if (response.has_get_transactions_response())
    {
        const bc::protocol::response_transactions& payload
            = response.get_transactions_response();

        on_reply(payload.transactions(), payload.next(), payload.top());
    }
    else
    {
        on_error(std::make_error_code(std::errc::bad_message));
    }
}

void server_codec_impl::handle_transaction_hashes(
    const bc::protocol::response& response,
    error_handler& on_error,
    transaction_hash_results_handler& on_reply)
{
    if (response.has_get_transactions_response())
    {
        const bc::protocol::response_transactions& payload
            = response.get_transactions_response();

        on_reply(payload.hashes(), payload.next(), payload.top());
    }
    else
    {
        on_error(std::make_error_code(std::errc::bad_message));
    }
}

void server_codec_impl::handle_utxos(
    const bc::protocol::response& response,
    error_handler& on_error,
    utxo_results_handler& on_reply)
{
    if (response.has_get_transactions_response())
    {
        const bc::protocol::response_transactions& payload
            = response.get_transactions_response();

        on_reply(payload.utxos(), payload.next(), payload.top());
    }
    else
    {
        on_error(std::make_error_code(std::errc::bad_message));
    }
}

void server_codec_impl::handle_post_transaction(
    const bc::protocol::response& response,
    error_handler& on_error,
    post_handler& on_reply)
{
    if (response.has_post_transaction_succeeded())
    {
        on_reply(response.post_transaction_succeeded());
    }
    else
    {
        on_error(std::make_error_code(std::errc::bad_message));
    }
}

void server_codec_impl::handle_validate_transaction(
    const bc::protocol::response& response,
    error_handler& on_error,
    validate_handler& on_reply)
{
    if (response.has_validate_transaction_succeeded())
    {
        on_reply(response.validate_transaction_succeeded());
    }
    else
    {
        on_error(std::make_error_code(std::errc::bad_message));
    }
}

bc::protocol::transactions_request* server_codec_impl::get_transactions_request(
    const bc::protocol::filter_list& query,
    bc::protocol::transaction_results result_type,
    bc::protocol::locations location_type,
    std::shared_ptr<bc::protocol::block_id> start,
    uint32_t results_per_page)
{
    std::unique_ptr<bc::protocol::transactions_request> get_transactions(
        new bc::protocol::transactions_request());

    if (start)
    {
        std::unique_ptr<bc::protocol::block_id> starting_block(
            new bc::protocol::block_id(*(start.get())));

        get_transactions->set_allocated_start(starting_block.release());
    }

    if (results_per_page < MAX_UINT32)
    {
        get_transactions->set_results_per_page(results_per_page);
    }

    get_transactions->set_result_type(result_type);

    get_transactions->set_location_type(location_type);

    for (auto clause: query)
    {
        bc::protocol::filter* current = get_transactions->add_query();
        (*current) = clause;
    }

    return get_transactions.release();
}

}
}
