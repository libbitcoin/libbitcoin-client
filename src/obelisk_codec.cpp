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
#include <bitcoin/client/obelisk_codec.hpp>

namespace libbitcoin {
namespace client {

BC_API obelisk_codec::obelisk_codec(message_stream& out)
  : last_request_id_(0), out_(out)
{
}

BC_API void obelisk_codec::message(const data_chunk& data, bool more)
{
    (void)data;
    (void)more;
}

BC_API void obelisk_codec::fetch_history(error_handler&& on_error,
    fetch_history_handler&& on_reply,
    const payment_address& address, size_t from_height)
{
    data_chunk data;
    data.resize(1 + short_hash_size + 4);
    auto serial = make_serializer(data.begin());
    serial.write_byte(address.version());
    serial.write_short_hash(address.hash());
    serial.write_4_bytes(from_height);
    BITCOIN_ASSERT(serial.iterator() == data.end());

    send_request("blockchain.fetch_history", data, std::move(on_error));
    (void)on_reply;
}

BC_API void obelisk_codec::fetch_transaction(error_handler&& on_error,
    fetch_transaction_handler&& on_reply,
    const hash_digest& tx_hash)
{
    data_chunk data;
    data.resize(hash_size);
    auto serial = make_serializer(data.begin());
    serial.write_hash(tx_hash);
    BITCOIN_ASSERT(serial.iterator() == data.end());

    send_request("blockchain.fetch_transaction", data, std::move(on_error));
    (void)on_reply;
}

BC_API void obelisk_codec::fetch_last_height(error_handler&& on_error,
    fetch_last_height_handler&& on_reply)
{
    send_request("blockchain.fetch_last_height", data_chunk(),
        std::move(on_error));
    (void)on_reply;
}

BC_API void obelisk_codec::fetch_block_header(error_handler&& on_error,
    fetch_block_header_handler&& on_reply,
    size_t height)
{
    data_chunk data = to_data_chunk(to_little_endian<uint32_t>(height));

    send_request("blockchain.fetch_block_header", data, std::move(on_error));
    (void)on_reply;
}

BC_API void obelisk_codec::fetch_block_header(error_handler&& on_error,
    fetch_block_header_handler&& on_reply,
    const hash_digest& blk_hash)
{
    data_chunk data(hash_size);
    auto serial = make_serializer(data.begin());
    serial.write_hash(blk_hash);
    BITCOIN_ASSERT(serial.iterator() == data.end());

    send_request("blockchain.fetch_block_header", data, std::move(on_error));
    (void)on_reply;
}

BC_API void obelisk_codec::fetch_transaction_index(error_handler&& on_error,
    fetch_transaction_index_handler&& on_reply,
    const hash_digest& tx_hash)
{
    data_chunk data(hash_size);
    auto serial = make_serializer(data.begin());
    serial.write_hash(tx_hash);
    BITCOIN_ASSERT(serial.iterator() == data.end());

    send_request("blockchain.fetch_transaction_index", data,
        std::move(on_error));
    (void)on_reply;
}

BC_API void obelisk_codec::fetch_stealth(error_handler&& on_error,
    fetch_stealth_handler&& on_reply,
    const stealth_prefix& prefix, size_t from_height)
{
    data_chunk data(1 + prefix.num_blocks() + 4);
    auto serial = make_serializer(data.begin());
    // number_bits
    serial.write_byte(prefix.size());
    // Serialize bitfield to raw bytes and serialize
    data_chunk bitfield(prefix.num_blocks());
    boost::to_block_range(prefix, bitfield.begin());
    serial.write_data(bitfield);
    // from_height
    serial.write_4_bytes(from_height);
    BITCOIN_ASSERT(serial.iterator() == data.end());

    send_request("blockchain.fetch_stealth", data, std::move(on_error));
    (void)on_reply;
}

BC_API void obelisk_codec::validate(error_handler&& on_error,
    validate_handler&& on_reply,
    const transaction_type& tx)
{
    data_chunk data(satoshi_raw_size(tx));
    auto it = satoshi_save(tx, data.begin());
    BITCOIN_ASSERT(it == data.end());

    send_request("transaction_pool.validate", data, std::move(on_error));
    (void)on_reply;
}

BC_API void obelisk_codec::fetch_unconfirmed_transaction(
    error_handler&& on_error,
    fetch_transaction_handler&& on_reply,
    const hash_digest& tx_hash)
{
    data_chunk data;
    data.resize(hash_size);
    auto serial = make_serializer(data.begin());
    serial.write_hash(tx_hash);
    BITCOIN_ASSERT(serial.iterator() == data.end());

    send_request("transaction_pool.fetch_transaction", data,
        std::move(on_error));
    (void)on_reply;
}

BC_API void obelisk_codec::broadcast_transaction(error_handler&& on_error,
    empty_handler&& on_reply,
    const transaction_type& tx)
{
    data_chunk data(satoshi_raw_size(tx));
    auto it = satoshi_save(tx, data.begin());
    BITCOIN_ASSERT(it == data.end());

    send_request("protocol.broadcast_transaction", data, std::move(on_error));
    (void)on_reply;
}

BC_API void obelisk_codec::address_fetch_history(error_handler&& on_error,
    fetch_history_handler&& on_reply,
    const payment_address& address, size_t from_height)
{
    data_chunk data;
    data.resize(1 + short_hash_size + 4);
    auto serial = make_serializer(data.begin());
    serial.write_byte(address.version());
    serial.write_short_hash(address.hash());
    serial.write_4_bytes(from_height);
    BITCOIN_ASSERT(serial.iterator() == data.end());

    send_request("address.fetch_history", data, std::move(on_error));
    (void)on_reply;
}

BC_API void obelisk_codec::subscribe(error_handler&& on_error,
    empty_handler&& on_reply,
    const address_prefix& prefix)
{
    BITCOIN_ASSERT(prefix.size() <= 255);
    data_chunk data(1 + prefix.num_blocks());
    data[0] = prefix.size();
    boost::to_block_range(prefix, data.begin() + 1);

    send_request("address.subscribe", data, std::move(on_error));
    (void)on_reply;
}

void obelisk_codec::send_request(const std::string& command,
    const data_chunk& payload, error_handler&& on_error)
{
    uint32_t id = ++last_request_id_;
    pending_request& request = pending_requests_[id];
    request.message = obelisk_message{command, id, payload};
    request.on_error = std::move(on_error);
    //request.on_reply = on_reply;
    request.retries = 0;
    //request.last_action = get_time()
    send(request.message);
}

void obelisk_codec::send(const obelisk_message& message)
{
    out_.message(to_data_chunk(message.command), true);
    out_.message(to_data_chunk(to_little_endian(message.id)), true);
    out_.message(message.payload, false);
}

} // namespace client
} // namespace libbitcoin
