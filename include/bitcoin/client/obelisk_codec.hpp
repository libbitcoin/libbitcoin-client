/*
 * Copyright (c) 2011-2014 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_CLIENT_OBELISK_CODEC_HPP
#define LIBBITCOIN_CLIENT_OBELISK_CODEC_HPP

#include <functional>
#include <map>
#include <bitcoin/client/message_stream.hpp>

namespace libbitcoin {
namespace client {

typedef stealth_prefix address_prefix;

/**
 * Decodes and encodes messages in the obelisk protocol.
 * This class is a pure codec; it does not talk directly to zeromq.
 */
class BC_API obelisk_codec
  : public message_stream
{
public:
    /**
     * Constructor.
     * @param out a stream to receive outgoing messages created by the codec.
     */
    BC_API obelisk_codec(message_stream& out);

    /**
     * Pass in a message for decoding.
     */
    BC_API void message(const data_chunk& data, bool more);

    // Message reply handlers:
    typedef std::function<void (const std::error_code&)>
        error_handler;
    typedef std::function<void (const blockchain::history_list&)>
        fetch_history_handler;
    typedef std::function<void (const transaction_type&)>
        fetch_transaction_handler;
    typedef std::function<void (size_t)>
        fetch_last_height_handler;
    typedef std::function<void (const block_header_type&)>
        fetch_block_header_handler;
    typedef std::function<void (size_t block_height, size_t index)>
        fetch_transaction_index_handler;
    typedef std::function<void (const blockchain::stealth_list&)>
        fetch_stealth_handler;
    typedef std::function<void (const index_list& unconfirmed)>
        validate_handler;
    typedef std::function<void ()> empty_handler;

    // Loose message handlers:
    typedef std::function<void (const payment_address& address,
        size_t height, const hash_digest& blk_hash, const transaction_type&)>
        update_handler;

    // Outgoing messages:
    BC_API void fetch_history(error_handler&& on_error,
        fetch_history_handler&& on_reply,
        const payment_address& address, size_t from_height=0);
    BC_API void fetch_transaction(error_handler&& on_error,
        fetch_transaction_handler&& on_reply,
        const hash_digest& tx_hash);
    BC_API void fetch_last_height(error_handler&& on_error,
        fetch_last_height_handler&& on_reply);
    BC_API void fetch_block_header(error_handler&& on_error,
        fetch_block_header_handler&& on_reply,
        size_t height);
    BC_API void fetch_block_header(error_handler&& on_error,
        fetch_block_header_handler&& on_reply,
        const hash_digest& blk_hash);
    BC_API void fetch_transaction_index(error_handler&& on_error,
        fetch_transaction_index_handler&& on_reply,
        const hash_digest& tx_hash);
    BC_API void fetch_stealth(error_handler&& on_error,
        fetch_stealth_handler&& on_reply,
        const stealth_prefix& prefix, size_t from_height=0);
    BC_API void validate(error_handler&& on_error,
        validate_handler&& on_reply,
        const transaction_type& tx);
    BC_API void fetch_unconfirmed_transaction(error_handler&& on_error,
        fetch_transaction_handler&& on_reply,
        const hash_digest& tx_hash);
    BC_API void broadcast_transaction(error_handler&& on_error,
        empty_handler&& on_reply,
        const transaction_type& tx);
    BC_API void address_fetch_history(error_handler&& on_error,
        fetch_history_handler&& on_reply,
        const payment_address& address, size_t from_height=0);
    BC_API void subscribe(error_handler&& on_error,
        empty_handler&& on_reply,
        const address_prefix& prefix);

private:
    /**
     * Sends an outgoing request, and adds the handlers to the pending
     * request table.
     */
    void send_request(const std::string& command,
        const data_chunk& payload, error_handler&& on_error);

    struct obelisk_message
    {
        std::string command;
        uint32_t id;
        data_chunk payload;
    };
    void send(const obelisk_message& message);

    // Request management:
    uint32_t last_request_id_;
    struct pending_request
    {
        obelisk_message message;
        error_handler on_error;
        //??? on_reply;
        unsigned retries;
        //time_t last_action;
    };
    std::map<uint32_t, pending_request> pending_requests_;

    // Outgoing message stream:
    message_stream& out_;
};

} // namespace client
} // namespace libbitcoin

#endif

