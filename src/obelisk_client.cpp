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
#include <bitcoin/client/obelisk_client.hpp>

#include <algorithm>
#include <thread>

#include <bitcoin/protocol/zmq/message.hpp>

using namespace bc::protocol;
using namespace bc::system;
using namespace bc::system::chain;
using namespace bc::system::config;
using namespace bc::system::wallet;
using namespace std::chrono;
using namespace std::this_thread;
using namespace std::placeholders;

namespace libbitcoin {
namespace client {

static const config::endpoint public_worker("inproc://public_client");
static const config::endpoint secure_worker("inproc://secure_client");

static const config::endpoint public_subscribe_worker(
    "inproc://public_subscribe_client");
static const config::endpoint secure_subscribe_worker(
    "inproc://secure_subscribe_client");

obelisk_client::obelisk_client(int32_t retries)
  : socket_(context_, zmq::socket::role::dealer),
    subscribe_socket_(context_, zmq::socket::role::dealer),
    block_socket_(context_, zmq::socket::role::subscriber),
    transaction_socket_(context_, zmq::socket::role::subscriber),
    dealer_(context_, zmq::socket::role::dealer),
    router_(context_, zmq::socket::role::router),
    subscribe_dealer_(context_, zmq::socket::role::dealer),
    subscribe_router_(context_, zmq::socket::role::router),
    retries_(retries),
    last_request_index_(0),
    secure_(false),
    worker_(public_worker),
    subscribe_worker_(public_subscribe_worker)
{
    attach_handlers();
}

obelisk_client::~obelisk_client()
{
    dealer_.stop();
    router_.stop();
    subscribe_dealer_.stop();
    subscribe_router_.stop();
    socket_.stop();
    subscribe_socket_.stop();
    block_socket_.stop();
    transaction_socket_.stop();
}

bool obelisk_client::connect(const connection_settings& settings)
{
    retries_ = settings.retries;
    return connect(settings.server, settings.socks, settings.server_public_key,
        settings.client_private_key);
}

bool obelisk_client::connect(const endpoint& address,
    const authority& socks_proxy, const sodium& server_public_key,
    const sodium& client_private_key)
{
    // Ignore the setting if socks.port is zero (invalid).
    if (socks_proxy && (!socket_.set_socks_proxy(socks_proxy) ||
        !subscribe_socket_.set_socks_proxy(socks_proxy)))
        return false;

    // Only apply the client (and server) key if server key is configured.
    if (server_public_key)
    {
        if (!socket_.set_curve_client(server_public_key) ||
            !subscribe_socket_.set_curve_client(server_public_key))
            return false;

        // Generates arbitrary client keys if private key is not configured.
        if (!socket_.set_certificate({ client_private_key }) ||
            !subscribe_socket_.set_certificate({ client_private_key }))
            return false;

        secure_ = true;
        worker_ = secure_worker;
        subscribe_worker_ = secure_subscribe_worker;
    }

    return connect(address);
}

bool obelisk_client::connect(const endpoint& address)
{
    const auto host_address = address.to_string();

    auto connect_socket = [&host_address](zmq::socket& socket,
        zmq::socket& dealer, zmq::socket& router, config::endpoint& worker)
    {
        if (socket.connect(host_address) == error::success)
        {
            // Bind internal router(s) to inproc worker
            auto ec = router.bind(worker);
            if (ec)
                return false;

            // Connect internal socket(s) to worker router
            ec = dealer.connect(worker);
            if (ec)
                return false;

            return true;
        }

        return false;
    };

    auto socket_connected = false;
    auto subscribe_connected = false;

    for (auto attempt = 0; attempt < 1 + retries_; ++attempt)
    {
        if (!socket_connected)
            socket_connected = connect_socket(socket_, dealer_, router_, worker_);

        // subscribe_socket connection could be deferred/unused until a
        // subscribe call is made.
        if (!subscribe_connected)
            subscribe_connected = connect_socket(subscribe_socket_, subscribe_dealer_,
                subscribe_router_, subscribe_worker_);

        if (socket_connected && subscribe_connected)
            return true;

        // Arbitrary delay between connection attempts.
        sleep_for(asio::milliseconds((attempt + 1) * 100));
    }

    return false;
}

void obelisk_client::forward_message(zmq::socket& source, zmq::socket& sink)
{
    // Forward incoming client router requests to the server.
    zmq::message packet;
    source.receive(packet);

    // Strip the router delimiter before forwarding.
    packet.dequeue();
    sink.send(packet);
}

void obelisk_client::process_response(zmq::socket& socket)
{
    // Process server responses.
    zmq::message message;
    socket.receive(message);

    // Strip the delimiter if the server includes it.
    if (message.size() == 4)
        message.dequeue();

    uint32_t id = 0;
    std::string command;
    data_chunk payload;

    message.dequeue(command);
    message.dequeue(id);
    message.dequeue(payload);

    const auto handler = command_handlers_.find(command);
    if (handler != command_handlers_.end())
        handler->second(command, id, payload);
}

// Used by query commands and fires handlers as needed.
void obelisk_client::wait(uint32_t timeout_milliseconds)
{
    zmq::poller poller;
    poller.add(socket_);
    poller.add(router_);

    static constexpr auto poll_timeout_milliseconds = 10;
    auto deadline = steady_clock::now() + milliseconds(timeout_milliseconds);

    while (!poller.terminated() && requests_outstanding() &&
        steady_clock::now() < deadline)
    {
        const auto identifiers = poller.wait(poll_timeout_milliseconds);

        // Forward incoming client router requests to the server.
        if (identifiers.contains(router_.id()))
            forward_message(router_, socket_);

        // Process server responses.
        if (identifiers.contains(socket_.id()))
            process_response(socket_);
    }

    // Timeout or otherwise notify any remaining requests.
    if (requests_outstanding())
        clear_outstanding_requests((steady_clock::now() >= deadline) ?
            error::channel_timeout : error::operation_failed);
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

// Used by watch-* and subscribe-* commands, fires registered update handlers.
void obelisk_client::monitor(uint32_t timeout_milliseconds)
{
    auto deadline = steady_clock::now() + milliseconds(timeout_milliseconds);

    zmq::poller poller;
    poller.add(subscribe_router_);
    poller.add(subscribe_socket_);
    poller.add(block_socket_);
    poller.add(transaction_socket_);

    // A timeout of 0 will still have a chance to complete.
    do
    {
        const auto identifiers = poller.wait(timeout_milliseconds);
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

            chain::block block;
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

            chain::transaction transaction;
            transaction.from_data(data, true, true);

            on_transaction_update_(transaction);
        }

        // Forward incoming client subscribe router requests to the server.
        if (identifiers.contains(subscribe_router_.id()))
            forward_message(subscribe_router_, subscribe_socket_);

        // Process server responses for subscribe calls.
        if (identifiers.contains(subscribe_socket_.id()))
            process_response(subscribe_socket_);

    } while (!poller.terminated() && subscribe_requests_outstanding() &&
        steady_clock::now() < deadline);

    clear_outstanding_subscribe_requests((steady_clock::now() >= deadline) ?
        error::channel_timeout : error::operation_failed);
}

// Create a message and send it to the internal router for forwarding
// to the server.
bool obelisk_client::send_request(const std::string& command,
    uint32_t id, const data_chunk& payload, bool subscription)
{
    zmq::message message;
    // First, add the required delimiter since we're sending to our
    // internal router socket.
    message.enqueue();
    message.enqueue(to_chunk(command));
    message.enqueue(to_chunk(to_little_endian(id)));
    message.enqueue(payload);

    return subscription ? !subscribe_dealer_.send(message) :
        !dealer_.send(message);
}

// Handlers.
//-----------------------------------------------------------------------------

void obelisk_client::attach_handlers()
{
    auto result_handler = [this](const std::string&, uint32_t id,
        const data_chunk& payload)
    {
        auto handler = result_handlers_.find(id);
        if (handler == result_handlers_.end())
            return;

        data_source istream(payload);
        istream_reader source(istream);
        handler->second(source.read_error_code());
        result_handlers_.erase(handler);
    };

    auto version_handler = [this](const std::string&, uint32_t id,
        const data_chunk& payload)
    {
        auto handler = version_handlers_.find(id);
        if (handler == version_handlers_.end())
            return;

        data_source istream(payload);
        istream_reader source(istream);
        const auto ec = source.read_error_code();
        const auto version = source.read_bytes();
        handler->second(ec, std::string(version.begin(), version.end()));
        version_handlers_.erase(handler);
    };

    auto transaction_handler = [this](const std::string&, uint32_t id,
        const data_chunk& payload)
    {
        auto handler = transaction_handlers_.find(id);
        if (handler == transaction_handlers_.end())
            return;

        data_source istream(payload);
        istream_reader source(istream);
        const auto ec = source.read_error_code();
        if (ec)
        {
            handler->second(ec, {});
            transaction_handlers_.erase(handler);
            return;
        }

        chain::transaction tx;
        if (!tx.from_data(source.read_bytes(), true, true))
        {
            handler->second(error::bad_stream, {});
            transaction_handlers_.erase(handler);
            return;
        }

        handler->second(ec, tx);
        transaction_handlers_.erase(handler);
    };

    auto height_handler = [this](const std::string&, uint32_t id,
        const data_chunk& payload)
    {
        auto handler = height_handlers_.find(id);
        if (handler == height_handlers_.end())
            return;

        data_source istream(payload);
        istream_reader source(istream);
        const auto ec = source.read_error_code();
        const size_t height = source.read_4_bytes_little_endian();
        handler->second(ec, height);
        height_handlers_.erase(handler);
    };

    auto block_header_handler = [this](const std::string&, uint32_t id,
        const data_chunk& payload)
    {
        auto handler = block_header_handlers_.find(id);
        if (handler == block_header_handlers_.end())
            return;

        data_source istream(payload);
        istream_reader source(istream);
        const auto ec = source.read_error_code();
        if (ec)
        {
            handler->second(ec, {});
            block_header_handlers_.erase(handler);
            return;
        }

        chain::header header;
        if (!header.from_data(source.read_bytes()))
        {
            handler->second(error::bad_stream, {});
            block_header_handlers_.erase(handler);
            return;
        }

        handler->second(ec, header);
        block_header_handlers_.erase(handler);
    };

    auto block_handler = [this](const std::string&, uint32_t id,
        const data_chunk& payload)
    {
        auto handler = block_handlers_.find(id);
        if (handler == block_handlers_.end())
            return;

        data_source istream(payload);
        istream_reader source(istream);
        const auto ec = source.read_error_code();
        if (ec)
        {
            handler->second(ec, {});
            block_handlers_.erase(handler);
            return;
        }

        chain::block block;
        if (!block.from_data(source.read_bytes()))
        {
            handler->second(error::bad_stream, {});
            block_handlers_.erase(handler);
            return;
        }

        handler->second(ec, block);
        block_handlers_.erase(handler);
    };

    auto compact_filter_handler = [this](const std::string&, uint32_t id,
        const data_chunk& payload)
    {
        auto handler = compact_filter_handlers_.find(id);
        if (handler == compact_filter_handlers_.end())
            return;

        data_source istream(payload);
        istream_reader source(istream);
        const auto ec = source.read_error_code();
        if (ec)
        {
            handler->second(ec, {});
            compact_filter_handlers_.erase(handler);
            return;
        }

        message::compact_filter response;
        if (!response.from_data(source.read_bytes()))
        {
            handler->second(error::bad_stream, {});
            compact_filter_handlers_.erase(handler);
            return;
        }

        handler->second(ec, response);
        compact_filter_handlers_.erase(handler);
    };

    auto compact_filter_checkpoint_handler = [this](const std::string&,
        uint32_t id, const data_chunk& payload)
    {
        auto handler = compact_filter_checkpoint_handlers_.find(id);
        if (handler == compact_filter_checkpoint_handlers_.end())
            return;

        data_source istream(payload);
        istream_reader source(istream);
        const auto ec = source.read_error_code();
        if (ec)
        {
            handler->second(ec, {});
            compact_filter_checkpoint_handlers_.erase(handler);
            return;
        }

        message::compact_filter_checkpoint response;
        const auto version = message::compact_filter_checkpoint::version_minimum;
        if (!response.from_data(version, source.read_bytes()))
        {
            handler->second(error::bad_stream, {});
            compact_filter_checkpoint_handlers_.erase(handler);
            return;
        }

        handler->second(ec, response);
        compact_filter_checkpoint_handlers_.erase(handler);
    };

    auto compact_filter_headers_handler = [this](const std::string&,
        uint32_t id, const data_chunk& payload)
    {
        auto handler = compact_filter_headers_handlers_.find(id);
        if (handler == compact_filter_headers_handlers_.end())
            return;

        data_source istream(payload);
        istream_reader source(istream);
        const auto ec = source.read_error_code();
        if (ec)
        {
            handler->second(ec, {});
            compact_filter_headers_handlers_.erase(handler);
            return;
        }

        message::compact_filter_headers response;
        const auto version = message::compact_filter_headers::version_minimum;
        if (!response.from_data(version, source.read_bytes()))
        {
            handler->second(error::bad_stream, {});
            compact_filter_headers_handlers_.erase(handler);
            return;
        }

        handler->second(ec, response);
        compact_filter_headers_handlers_.erase(handler);
    };

    auto transaction_index_handler = [this](const std::string&, uint32_t id,
        const data_chunk& payload)
    {
        auto handler = transaction_index_handlers_.find(id);
        if (handler == transaction_index_handlers_.end())
            return;

        data_source istream(payload);
        istream_reader source(istream);
        const auto ec = source.read_error_code();
        const auto block_height = source.read_4_bytes_little_endian();
        const auto index = source.read_4_bytes_little_endian();
        handler->second(ec, block_height, index);
        transaction_index_handlers_.erase(handler);
    };

    auto stealth_handler = [this](const std::string&, uint32_t id,
        const data_chunk& payload)
    {
        auto handler = stealth_handlers_.find(id);
        if (handler == stealth_handlers_.end())
            return;

        stealth_record stealth;
        stealth_record::list records;

        data_source istream(payload);
        istream_reader source(istream);
        const auto ec = source.read_error_code();
        while (!source.is_exhausted())
        {
            if (!stealth.from_data(source, true))
            {
                handler->second(ec, {});
                stealth_handlers_.erase(handler);
                return;
            }

            records.push_back(stealth);
        }

        // Expand records.
        stealth::list result;
        result.reserve(records.size());

        for (const auto& in: records)
            result.emplace_back(in.ephemeral_public_key(), in.public_key_hash(),
                in.transaction_hash());

        handler->second(ec, result);
        stealth_handlers_.erase(handler);
    };

    auto history_handler = [this](const std::string&, uint32_t id,
        const data_chunk& payload)
    {
        auto handler = history_handlers_.find(id);
        if (handler == history_handlers_.end())
            return;

        payment_record payment;
        payment_record::list records;

        data_source istream(payload);
        istream_reader source(istream);
        const auto ec = source.read_error_code();
        while (!source.is_exhausted())
        {
            if (!payment.from_data(source, true))
            {
                handler->second(ec, {});
                history_handlers_.erase(handler);
                return;
            }

            records.push_back(payment);
        }

        history::list result;
        result.reserve(records.size());
        std::unordered_multimap<uint64_t, size_t> output_checksums;

        // Process and remove all outputs.
        for (const auto& record: records)
        {
            if (!record.is_output())
                continue;

            output_point output{ record.hash(), record.index() };
            const auto temporary_checksum = output.checksum();
            result.emplace_back(
                output,
                record.height(),
                record.data(),
                input_point{ null_hash, chain::point::null_index },
                temporary_checksum);

            output_checksums.emplace(temporary_checksum, result.size() - 1);
        }

        // TODO: reduce to output set with distinct checksums, as a fault signal.
        ////std::sort(result.begin(), result.end());
        ////result.erase(std::unique(result.begin(), result.end()), result.end());

        // All outputs have been handled, process the spends.
        for (auto& record: records)
        {
            if (record.is_output())
                continue;

            auto found = false;
            const auto matches = output_checksums.equal_range(record.data());

            // Update outputs with the corresponding spends.
            // This relies on the lucky avoidance of checksum hash collisions :<.
            // Ordering is insufficient since the server may write concurrently.
            for (auto match = matches.first; match != matches.second; match++)
            {
                auto& history = result[match->second];

                // The temporary_checksum is a union with spend_height, so we must
                // guard against matching temporary_checksum unless spend is null.
                if (history.spend.is_null())
                {
                    // Move the spend to the row of its correlated output.
                    history.spend = input_point{ record.hash(), record.index() };
                    history.spend_height = record.height();
                    found = true;
                    break;
                }
            }

            // This will only happen if the history height cutoff comes between an
            // output and its spend. In this case we return just the spend.
            // This is not strictly sufficient because of checksum hash collisions,
            // So this miscorrelation must be discarded as a fault signal.
            if (!found)
            {
                result.emplace_back(
                    output_point{ null_hash, chain::point::null_index },
                    max_size_t,
                    max_uint64,
                    input_point{ record.hash(), record.index() },
                    record.height());
            }
        }

        result.shrink_to_fit();

        // Clear all remaining checksums from unspent rows.
        for (auto& history: result)
            if (history.spend.is_null())
                history.spend_height = max_uint64;

        handler->second(ec, result);
        history_handlers_.erase(handler);
    };

    // This handler locks subscription_handlers_ while running to avoid
    // subscription handler state from changing while running (called from
    // process_response).
    auto notification_handler = [this](const std::string&, uint32_t id,
        const data_chunk& payload)
    {
        // Critical Section.
        ///////////////////////////////////////////////////////////////////////////
        subscription_lock_.lock_upgrade();
        auto it = subscription_handlers_.find(id);
        if (it == subscription_handlers_.end())
        {
            subscription_lock_.unlock_upgrade();
            return;
        }

        auto& handler = it->second.first;
        // [ code:4 ]     <- if this is nonzero then rest may be empty.
        // [ sequence:2 ] <- if out of order there was a lost message.
        // [ height:4 ]   <- 0 for unconfirmed or error tx (cannot notify genesis).
        // [ tx_hash:32 ] <- may be null_hash on errors.

        data_source istream(payload);
        istream_reader source(istream);
        const auto ec = source.read_error_code();
        if (ec)
        {
            handler(ec, {}, {}, {});
            subscription_lock_.unlock_upgrade_and_lock();
            subscription_handlers_.erase(it);
            subscription_lock_.unlock();
            return;
        }

        const auto sequence = source.read_2_bytes_little_endian();
        const size_t height = source.read_4_bytes_little_endian();
        const auto tx_hash = source.read_hash();

        if (!source.is_exhausted())
        {
            handler(error::bad_stream, {}, {}, {});
            subscription_lock_.unlock_upgrade_and_lock();
            subscription_handlers_.erase(it);
            subscription_lock_.unlock();
            return;
        }

        subscription_lock_.unlock_upgrade();
        ///////////////////////////////////////////////////////////////////////////

        // Caller must differentiate type of update if subscribed to multiple.
        handler(ec, sequence, height, tx_hash);
    };

    // This handler locks subscription_handlers_ while running to avoid
    // (un)subscription handler state from changing while running (called from
    // process_response).
    auto unsubscribe_handler = [this](const std::string&, uint32_t id,
        const data_chunk& payload)
    {
        // Critical Section.
        ///////////////////////////////////////////////////////////////////////////
        subscription_lock_.lock_upgrade();
        auto it = unsubscription_handlers_.find(id);
        if (it == unsubscription_handlers_.end())
        {
            subscription_lock_.unlock_upgrade();
            return;
        }

        const auto handler = it->second.first;
        const auto subscription = it->second.second;
        subscription_lock_.unlock_upgrade();
        ///////////////////////////////////////////////////////////////////////////

        data_source istream(payload);
        istream_reader source(istream);
        handler(source.read_error_code());

        // Critical Section.
        ///////////////////////////////////////////////////////////////////////////
        subscription_lock_.lock();
        unsubscription_handlers_.erase(it);
        subscription_lock_.unlock();
        ///////////////////////////////////////////////////////////////////////////

        // Terminate any listener monitoring this subscription.
        terminate_unsubscriber(subscription);
    };

    auto hash_list_handler = [this](const std::string&, uint32_t id,
        const data_chunk& payload)
    {
        auto handler = hash_list_handlers_.find(id);
        if (handler == hash_list_handlers_.end())
            return;

        hash_list hashes;

        data_source istream(payload);
        istream_reader source(istream);
        const auto ec = source.read_error_code();
        while (!source.is_exhausted())
            hashes.push_back(source.read_hash());

        handler->second(ec, hashes);
        hash_list_handlers_.erase(handler);
    };

#define REGISTER_HANDLER(command, handler) \
    command_handlers_[command] = handler

    REGISTER_HANDLER("transaction_pool.broadcast", result_handler);
    REGISTER_HANDLER("transaction_pool.validate2", result_handler);
    REGISTER_HANDLER("transaction_pool.fetch_transaction", transaction_handler);
    REGISTER_HANDLER("transaction_pool.fetch_transaction2",
        transaction_handler);
    REGISTER_HANDLER("blockchain.broadcast", result_handler);
    REGISTER_HANDLER("blockchain.validate", result_handler);
    REGISTER_HANDLER("blockchain.fetch_transaction", transaction_handler);
    REGISTER_HANDLER("blockchain.fetch_transaction2", transaction_handler);
    REGISTER_HANDLER("blockchain.fetch_last_height", height_handler);
    REGISTER_HANDLER("blockchain.fetch_block", block_handler);
    REGISTER_HANDLER("blockchain.fetch_block_header", block_header_handler);
    REGISTER_HANDLER("blockchain.fetch_block_height", height_handler);
    REGISTER_HANDLER("blockchain.fetch_compact_filter", compact_filter_handler);
    REGISTER_HANDLER("blockchain.fetch_compact_filter_checkpoint", compact_filter_checkpoint_handler);
    REGISTER_HANDLER("blockchain.fetch_compact_filter_headers", compact_filter_headers_handler);
    REGISTER_HANDLER("blockchain.fetch_transaction_index",
        transaction_index_handler);
    REGISTER_HANDLER("blockchain.fetch_stealth2", stealth_handler);
    REGISTER_HANDLER("blockchain.fetch_history4", history_handler);
    REGISTER_HANDLER("blockchain.fetch_block_transaction_hashes", hash_list_handler);
    REGISTER_HANDLER("blockchain.fetch_stealth_transaction_hashes",
        hash_list_handler);
    REGISTER_HANDLER("subscribe.key", notification_handler);
    REGISTER_HANDLER("notification.key", notification_handler);
    REGISTER_HANDLER("subscribe.stealth", notification_handler);
    REGISTER_HANDLER("notification.stealth", notification_handler);
    REGISTER_HANDLER("unsubscribe.key", unsubscribe_handler);
    REGISTER_HANDLER("unsubscribe.stealth", unsubscribe_handler);
    REGISTER_HANDLER("server.version", version_handler);

#undef REGISTER_HANDLER
}

void obelisk_client::handle_immediate(const std::string& command, uint32_t id,
    const code& ec)
{
    auto command_handler = command_handlers_.find(command);
    if (command_handler == command_handlers_.end())
        return;

    const auto payload = build_chunk(
    {
        to_little_endian(static_cast<uint32_t>(ec.value()))
    });

    command_handler->second(command, id, payload);
}

bool obelisk_client::requests_outstanding()
{
    // We have requests outstanding if any of the handler maps are not
    // empty, except update/notification handlers.
    return
        !result_handlers_.empty() ||
        !height_handlers_.empty() ||
        !transaction_index_handlers_.empty() ||
        !block_handlers_.empty() ||
        !block_header_handlers_.empty() ||
        !transaction_handlers_.empty() ||
        !hash_list_handlers_.empty() ||
        !history_handlers_.empty() ||
        !stealth_handlers_.empty() ||
        !version_handlers_.empty() ||
        !compact_filter_handlers_.empty() ||
        !compact_filter_checkpoint_handlers_.empty() ||
        !compact_filter_headers_handlers_.empty();
}

// We have subscribe requests outstanding if the subscription handler map is not
// empty.
bool obelisk_client::subscribe_requests_outstanding()
{
    // Critical Section.
    ///////////////////////////////////////////////////////////////////////////
    system::unique_lock lock(subscription_lock_);
    return !subscription_handlers_.empty() || !unsubscription_handlers_.empty();
    ///////////////////////////////////////////////////////////////////////////
}

void obelisk_client::clear_outstanding_requests(const code& ec)
{
#define INVOKE_HANDLER_0 handler.second(ec)
#define INVOKE_HANDLER_1 handler.second(ec, {})
#define INVOKE_HANDLER_2 handler.second(ec, {}, {})

#define CLEAR_OUTSTANDING(handlers, ec, handler_version) \
    for (auto& handler: handlers) \
        INVOKE_HANDLER_##handler_version; \
    handlers.clear()

    // Clear the handler maps, but first fire the handlers with the
    // specified error.
    CLEAR_OUTSTANDING(result_handlers_, ec, 0);
    CLEAR_OUTSTANDING(height_handlers_, ec, 1);
    CLEAR_OUTSTANDING(transaction_index_handlers_, ec, 2);
    CLEAR_OUTSTANDING(block_handlers_, ec, 1);
    CLEAR_OUTSTANDING(block_header_handlers_, ec, 1);
    CLEAR_OUTSTANDING(transaction_handlers_, ec, 1);
    CLEAR_OUTSTANDING(hash_list_handlers_, ec, 1);
    CLEAR_OUTSTANDING(history_handlers_, ec, 1);
    CLEAR_OUTSTANDING(stealth_handlers_, ec, 1);
    CLEAR_OUTSTANDING(version_handlers_, ec, 1);

#undef CLEAR_OUTSTANDING
#undef INVOKE_HANDLER_0
#undef INVOKE_HANDLER_1
#undef INVOKE_HANDLER_2
}

void obelisk_client::clear_outstanding_subscribe_requests(const code& ec)
{
    // Critical Section.
    ///////////////////////////////////////////////////////////////////////////
    system::unique_lock lock(subscription_lock_);

    for (auto& it: subscription_handlers_)
        it.second.first(ec, {}, {}, {});
    for (auto& it: unsubscription_handlers_)
        it.second.first(ec);

    subscription_handlers_.clear();
    unsubscription_handlers_.clear();
    ///////////////////////////////////////////////////////////////////////////
}

// Fetchers.
//-----------------------------------------------------------------------------

void obelisk_client::server_version(version_handler handler)
{
    static const std::string command = "server.version";
    static const data_chunk empty{};
    const auto id = ++last_request_index_;
    version_handlers_[id] = handler;
    if (!send_request(command, id, empty))
        handle_immediate(command, id, error::network_unreachable);
}

// This will fail if a witness tx is sent to a < v3.4 (pre-witness) server.
void obelisk_client::transaction_pool_broadcast(result_handler handler,
    const chain::transaction& tx)
{
    static const std::string command = "transaction_pool.broadcast";
    const auto id = ++last_request_index_;
    result_handlers_[id] = handler;
    if (!send_request(command, id, tx.to_data(true, true)))
        handle_immediate(command, id, error::network_unreachable);
}

// This will fail if a witness tx is sent to a < v3.4 (pre-witness) server.
void obelisk_client::transaction_pool_validate2(result_handler handler,
    const chain::transaction& tx)
{
    static const std::string command = "transaction_pool.validate2";
    const auto id = ++last_request_index_;
    result_handlers_[id] = handler;
    if (!send_request(command, id, tx.to_data(true, true)))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::transaction_pool_fetch_transaction(
    transaction_handler handler, const hash_digest& tx_hash)
{
    static const std::string command = "transaction_pool.fetch_transaction";
    const auto data = build_chunk({ tx_hash });
    const auto id = ++last_request_index_;
    transaction_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::transaction_pool_fetch_transaction2(
     transaction_handler handler, const hash_digest& tx_hash)
{
    static const std::string command = "transaction_pool.fetch_transaction2";
    const auto data = build_chunk({ tx_hash });
    const auto id = ++last_request_index_;
    transaction_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::blockchain_broadcast(result_handler handler,
    const chain::block& block)
{
    static const std::string command = "blockchain.broadcast";
    const auto id = ++last_request_index_;
    result_handlers_[id] = handler;
    if (!send_request(command, id, block.to_data()))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::blockchain_validate(result_handler handler,
    const chain::block& block)
{
    static const std::string command = "blockchain.validate";
    const auto id = ++last_request_index_;
    result_handlers_[id] = handler;
    if (!send_request(command, id, block.to_data()))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::blockchain_fetch_transaction(
     transaction_handler handler, const hash_digest& tx_hash)
{
    static const std::string command = "blockchain.fetch_transaction";
    const auto data = build_chunk({ tx_hash });
    const auto id = ++last_request_index_;
    transaction_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::blockchain_fetch_transaction2(
     transaction_handler handler, const hash_digest& tx_hash)
{
    static const std::string command = "blockchain.fetch_transaction2";
    const auto data = build_chunk({ tx_hash });
    const auto id = ++last_request_index_;
    transaction_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::blockchain_fetch_last_height(height_handler handler)
{
    static const std::string command = "blockchain.fetch_last_height";
    const data_chunk data{};
    const auto id = ++last_request_index_;
    height_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::blockchain_fetch_block(block_handler handler,
    uint32_t height)
{
    static const std::string command = "blockchain.fetch_block";
    const auto data = build_chunk({ to_little_endian<uint32_t>(height) });
    const auto id = ++last_request_index_;
    block_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::blockchain_fetch_block(block_handler handler,
    const hash_digest& block_hash)
{
    static const std::string command = "blockchain.fetch_block";
    const auto data = build_chunk({ block_hash });
    const auto id = ++last_request_index_;
    block_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::blockchain_fetch_block_header(
    block_header_handler handler, uint32_t height)
{
    static const std::string command = "blockchain.fetch_block_header";
    const auto data = build_chunk({ to_little_endian<uint32_t>(height) });
    const auto id = ++last_request_index_;
    block_header_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::blockchain_fetch_block_header(block_header_handler handler,
    const hash_digest& block_hash)
{
    static const std::string command = "blockchain.fetch_block_header";
    const auto data = build_chunk({ block_hash });
    const auto id = ++last_request_index_;
    block_header_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::blockchain_fetch_transaction_index(
    transaction_index_handler handler, const hash_digest& tx_hash)
{
    static const std::string command = "blockchain.fetch_transaction_index";
    const auto data = build_chunk({ tx_hash });
    const auto id = ++last_request_index_;
    transaction_index_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::blockchain_fetch_stealth2(stealth_handler handler,
    const binary& prefix, uint32_t from_height)
{
    static const std::string command = "blockchain.fetch_stealth2";
    const auto bits = prefix.size();

    if (bits < stealth_address::min_filter_bits ||
        bits > stealth_address::max_filter_bits)
    {
        handler(error::operation_failed, {});
        return;
    }

    const auto data = build_chunk(
    {
        to_array(static_cast<uint8_t>(prefix.size())),
        prefix.blocks(),
        to_little_endian<uint32_t>(from_height)
    });

    const auto id = ++last_request_index_;
    stealth_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

// blockchain.fetch_history4 (v4.0) request accepts key instead of
//   address_hash and response differs.
// blockchain.fetch_history3 (v3.1) does not accept a version byte.
// blockchain.fetch_history2 (v3.0) ignored version and is obsoleted in v3.1.
// blockchain.fetch_history (v1/v2) used hash reversal and is obsoleted in v3.
void obelisk_client::blockchain_fetch_history4(history_handler handler,
    const hash_digest& key, uint32_t from_height)
{
    static const std::string command = "blockchain.fetch_history4";

    const auto data = build_chunk(
    {
        key,
        to_little_endian<uint32_t>(from_height)
    });

    const auto id = ++last_request_index_;
    history_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::blockchain_fetch_unspent_outputs(
    points_value_handler handler, const hash_digest& key,
    uint64_t satoshi, select_outputs::algorithm algorithm)
{
    static constexpr uint32_t from_height = 0;
    static const std::string command = "blockchain.fetch_history4";

    const auto data = build_chunk(
    {
        key,
        to_little_endian<uint32_t>(from_height)
    });

    auto select_from_history = [handler, satoshi, algorithm](
        const code&, const history::list& rows)
    {
        chain::points_value unspent;
        unspent.points.reserve(rows.size());

        for (const auto& row: rows)
            if (row.spend.is_null())
                unspent.points.emplace_back(row.output, row.value);

        unspent.points.shrink_to_fit();
        chain::points_value selected;
        select_outputs::select(selected, unspent, satoshi, algorithm);
        handler(error::success, selected);
    };

    const auto id = ++last_request_index_;
    history_handlers_[id] = select_from_history;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::blockchain_fetch_block_height(height_handler handler,
    const hash_digest& block_hash)
{
    static const std::string command = "blockchain.fetch_block_height";
    const auto data = build_chunk({ block_hash });
    const auto id = ++last_request_index_;
    height_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::blockchain_fetch_block_transaction_hashes(
    hash_list_handler handler, uint32_t height)
{
    static const std::string command = "blockchain.fetch_block_transaction_hashes";
    const auto data = build_chunk({ to_little_endian<uint32_t>(height) });
    const auto id = ++last_request_index_;
    hash_list_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::blockchain_fetch_block_transaction_hashes(
    hash_list_handler handler, const hash_digest& block_hash)
{
    static const std::string command = "blockchain.fetch_block_transaction_hashes";
    const auto data = build_chunk({ block_hash });
    const auto id = ++last_request_index_;
    hash_list_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::blockchain_fetch_compact_filter(
    compact_filter_handler handler, uint8_t filter_type, uint32_t height)
{
    static const std::string command = "blockchain.fetch_compact_filter";
    const auto data = build_chunk({
        to_array(filter_type),
        to_little_endian<uint32_t>(height)
    });

    const auto id = ++last_request_index_;
    compact_filter_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::blockchain_fetch_compact_filter(
    compact_filter_handler handler, uint8_t filter_type,
    const system::hash_digest& block_hash)
{
    static const std::string command = "blockchain.fetch_compact_filter";
    const auto data = build_chunk({
        to_array(filter_type),
        block_hash
    });

    const auto id = ++last_request_index_;
    compact_filter_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::blockchain_fetch_compact_filter_headers(
    compact_filter_headers_handler handler, uint8_t filter_type,
    uint32_t start_height, const system::hash_digest& stop_hash)
{
    static const std::string command = "blockchain.fetch_compact_filter_headers";
    const auto data = build_chunk({
        to_array(filter_type),
        to_little_endian<uint32_t>(start_height),
        stop_hash
    });

    const auto id = ++last_request_index_;
    compact_filter_headers_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::blockchain_fetch_compact_filter_headers(
    compact_filter_headers_handler handler, uint8_t filter_type,
    uint32_t start_height, uint32_t stop_height)
{
    static const std::string command = "blockchain.fetch_compact_filter_headers";
    const auto data = build_chunk({
        to_array(filter_type),
        to_little_endian<uint32_t>(start_height),
        to_little_endian<uint32_t>(stop_height)
    });

    const auto id = ++last_request_index_;
    compact_filter_headers_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::blockchain_fetch_compact_filter_checkpoint(
    compact_filter_checkpoint_handler handler, uint8_t filter_type,
    const system::hash_digest& stop_hash)
{
    static const std::string command = "blockchain.fetch_compact_filter_checkpoint";
    const auto data = build_chunk({
        to_array(filter_type),
        stop_hash
    });

    const auto id = ++last_request_index_;
    compact_filter_checkpoint_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

//void obelisk_client::blockchain_fetch_compact_filter_checkpoint(
//    compact_filter_checkpoint_handler handler, uint8_t filter_type,
//    uint32_t stop_height)
//{
//    static const std::string command = "blockchain.fetch_compact_filter_checkpoint";
//    const auto data = build_chunk({
//        to_array(filter_type),
//        to_little_endian<uint32_t>(stop_height)
//    });
//
//    const auto id = ++last_request_index_;
//    compact_filter_checkpoint_handlers_[id] = handler;
//    if (!send_request(command, id, data))
//        handle_immediate(command, id, error::network_unreachable);
//}

void obelisk_client::blockchain_fetch_stealth_transaction_hashes(
    hash_list_handler handler, uint32_t height)
{
    static const std::string command = "blockchain.fetch_stealth_transaction_hashes";
    const auto data = build_chunk({ to_little_endian<uint32_t>(height) });
    const auto id = ++last_request_index_;
    hash_list_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

void obelisk_client::blockchain_fetch_stealth_transaction_hashes(
    hash_list_handler handler, const hash_digest& block_hash)
{
    static const std::string command = "blockchain.fetch_stealth_transaction_hashes";
    const auto data = build_chunk({ block_hash });
    const auto id = ++last_request_index_;
    hash_list_handlers_[id] = handler;
    if (!send_request(command, id, data))
        handle_immediate(command, id, error::network_unreachable);
}

// Subscribers.
//-----------------------------------------------------------------------------

// subscribe.address is renamed to subscribe.key (v4.0), input key differs.
uint32_t obelisk_client::subscribe_key(update_handler handler,
    const hash_digest& key)
{
    static const std::string command = "subscribe.key";
    // [ key:32 ]
    const auto data = build_chunk({ key });

    // Critical Section.
    ///////////////////////////////////////////////////////////////////////////
    subscription_lock_.lock();
    const auto id = ++last_request_index_;
    subscription_handlers_[id] = { handler, data };
    subscription_lock_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    if (!send_request(command, id, data, true))
    {
        handle_immediate(command, id, error::network_unreachable);
        return null_subscription;
    }

    handler(error::success, {}, {}, {});
    return id;
}

// This overload supports a prefix for either stealth or payment address.
uint32_t obelisk_client::subscribe_stealth(update_handler handler,
    const binary& stealth_prefix)
{
    static const std::string command = "subscribe.stealth";
    const auto bits = stealth_prefix.size();

    if (bits < stealth_address::min_filter_bits ||
        bits > stealth_address::max_filter_bits)
    {
        handler(error::operation_failed, {}, {}, {});
        return null_subscription;
    }

    // [ prefix_bitsize:1 ]
    // [ prefix_blocks:...]
    const auto data = build_chunk(
    {
        to_array(static_cast<uint8_t>(bits)),
        stealth_prefix.blocks()
    });

    // Critical Section.
    ///////////////////////////////////////////////////////////////////////////
    subscription_lock_.lock();
    const auto id = ++last_request_index_;
    subscription_handlers_[id] = { handler, data };
    subscription_lock_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    if (!send_request(command, id, data, true))
    {
        handle_immediate(command, id, error::network_unreachable);
        return null_subscription;
    }

    handler(error::success, {}, {}, {});
    return id;
}

// unsubscribe.address is renamed to unsubscribe.key (v4.0), input key differs.
bool obelisk_client::unsubscribe_key(result_handler handler,
    uint32_t subscription)
{
    static const std::string command = "unsubscribe.key";

    data_chunk data;

    // Critical Section.
    ///////////////////////////////////////////////////////////////////////////
    subscription_lock_.lock_upgrade();
    auto it = subscription_handlers_.find(subscription);
    if (it == subscription_handlers_.end())
    {
        subscription_lock_.unlock_upgrade();
        return false;
    }

    subscription_lock_.unlock_upgrade_and_lock();
    const auto id = ++last_request_index_;
    unsubscription_handlers_[id] = { handler, subscription };
    data = it->second.second;
    subscription_lock_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    if (!send_request(command, id, data, true))
    {
        handle_immediate(command, id, error::network_unreachable);
        return false;
    }

    return true;
}

bool obelisk_client::unsubscribe_stealth(result_handler handler,
    uint32_t subscription)
{
    static const std::string command = "unsubscribe.stealth";

    data_chunk data;

    // Critical Section.
    ///////////////////////////////////////////////////////////////////////////
    subscription_lock_.lock_upgrade();
    auto it = subscription_handlers_.find(subscription);
    if (it == subscription_handlers_.end())
    {
        subscription_lock_.unlock_upgrade();
        return false;
    }

    subscription_lock_.unlock_upgrade_and_lock();
    const auto id = ++last_request_index_;
    unsubscription_handlers_[id] = { handler, subscription };
    data = it->second.second;
    subscription_lock_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    if (!send_request(command, id, data, true))
    {
        handle_immediate(command, id, error::network_unreachable);
        return false;
    }

    return true;
}

// Must be called with subscription_lock_ locked (called from
// unsubscription_handler).
bool obelisk_client::terminate_unsubscriber(uint32_t subscription)
{
    // Critical Section.
    ///////////////////////////////////////////////////////////////////////////
    subscription_lock_.lock_upgrade();
    auto it = subscription_handlers_.find(subscription);
    if (it == subscription_handlers_.end())
    {
        subscription_lock_.unlock_upgrade();
        return false;
    }

    subscription_lock_.unlock_upgrade_and_lock();
    subscription_handlers_.erase(it);
    subscription_lock_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    return true;
}


} // namespace client
} // namespace libbitcoin
