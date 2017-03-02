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
#include <bitcoin/client/proxy.hpp>

#include <cstdint>
#include <memory>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/client/dealer.hpp>
#include <bitcoin/client/define.hpp>
#include <bitcoin/client/stream.hpp>

namespace libbitcoin {
namespace client {

using std::placeholders::_1;
using namespace bc::chain;
using namespace bc::wallet;

proxy::proxy(stream& out, unknown_handler on_unknown_command,
    uint32_t timeout_milliseconds, uint8_t resends)
  : dealer(out, on_unknown_command, timeout_milliseconds, resends)
{
}

// Fetchers.
//-----------------------------------------------------------------------------

void proxy::transaction_pool_broadcast(error_handler on_error,
    result_handler on_reply, const chain::transaction& tx)
{
    send_request("transaction_pool.broadcast", tx.to_data(), on_error,
        std::bind(decode_empty,
            _1, on_reply));
}

void proxy::transaction_pool_validate2(error_handler on_error,
    result_handler on_reply, const chain::transaction& tx)
{
    send_request("transaction_pool.validate2", tx.to_data(), on_error,
        std::bind(decode_empty,
            _1, on_reply));
}

void proxy::transaction_pool_fetch_transaction(error_handler on_error,
    transaction_handler on_reply, const hash_digest& tx_hash)
{
    const auto data = build_chunk({ tx_hash });

    send_request("transaction_pool.fetch_transaction", data, on_error,
        std::bind(decode_transaction,
            _1, on_reply));
}

void proxy::blockchain_broadcast(error_handler on_error,
    result_handler on_reply, const chain::block& block)
{
    send_request("blockchain.broadcast", block.to_data(), on_error,
        std::bind(decode_empty,
            _1, on_reply));
}

void proxy::blockchain_validate(error_handler on_error,
    result_handler on_reply, const chain::block& block)
{
    send_request("blockchain.validate", block.to_data(), on_error,
        std::bind(decode_empty,
            _1, on_reply));
}

void proxy::blockchain_fetch_transaction(error_handler on_error,
    transaction_handler on_reply, const hash_digest& tx_hash)
{
    const auto data = build_chunk({ tx_hash });

    send_request("blockchain.fetch_transaction", data, on_error,
        std::bind(decode_transaction,
            _1, on_reply));
}

void proxy::blockchain_fetch_last_height(error_handler on_error,
    height_handler on_reply)
{
    const data_chunk data;

    send_request("blockchain.fetch_last_height", data, on_error,
        std::bind(decode_height,
            _1, on_reply));
}

void proxy::blockchain_fetch_block_header(error_handler on_error,
    block_header_handler on_reply, uint32_t height)
{
    const auto data = build_chunk({ to_little_endian<uint32_t>(height) });

    send_request("blockchain.fetch_block_header", data, on_error,
        std::bind(decode_block_header,
            _1, on_reply));
}

void proxy::blockchain_fetch_block_header(error_handler on_error,
    block_header_handler on_reply, const hash_digest& block_hash)
{
    const auto data = build_chunk({ block_hash });

    send_request("blockchain.fetch_block_header", data, on_error,
        std::bind(decode_block_header,
            _1, on_reply));
}

void proxy::blockchain_fetch_transaction_index(error_handler on_error,
    transaction_index_handler on_reply, const hash_digest& tx_hash)
{
    const auto data = build_chunk({ tx_hash });

    send_request("blockchain.fetch_transaction_index", data, on_error,
        std::bind(decode_transaction_index,
            _1, on_reply));
}

void proxy::blockchain_fetch_stealth(error_handler on_error,
    stealth_handler on_reply, const binary& prefix, uint32_t from_height)
{
    if (prefix.size() > max_uint8)
    {
        on_error(error::bad_stream);
        return;
    }

    const auto data = build_chunk(
    {
        to_array(static_cast<uint8_t>(prefix.size())),
        prefix.blocks(),
        to_little_endian<uint32_t>(from_height)
    });

    send_request("blockchain.fetch_stealth", data, on_error,
        std::bind(decode_stealth,
            _1, on_reply));
}

// blockchain.fetch_history (v1/v2) used hash reversal and is obsoleted in v3.
void proxy::blockchain_fetch_history2(error_handler on_error,
    history_handler on_reply, const payment_address& address,
    uint32_t from_height)
{
    const auto data = build_chunk(
    {
        to_array(address.version()),
        address.hash(),
        to_little_endian<uint32_t>(from_height)
    });

    send_request("blockchain.fetch_history2", data, on_error,
        std::bind(decode_history,
            _1, on_reply));
}

////// The difference between fetch_history and fetch_history2 is hash reversal.
////void proxy::address_fetch_history2(error_handler on_error,
////    history_handler on_reply, const payment_address& address,
////    uint32_t from_height)
////{
////    const auto data = build_chunk(
////    {
////        to_array(address.version()),
////        address.hash(),
////        to_little_endian<uint32_t>(from_height)
////    });
////
////    // address.fetch_history2 is first available in bs 3.0.
////    send_request("address.fetch_history2", data, on_error,
////        std::bind(decode_history,
////            _1, on_reply));
////}

void proxy::address_fetch_unspent_outputs(error_handler on_error,
    points_value_handler on_reply, const payment_address& address,
    uint64_t satoshi, select_outputs::algorithm algorithm)
{
    static constexpr uint32_t from_height = 0;

    const auto data = build_chunk(
    {
        to_array(address.version()),
        address.hash(),
        to_little_endian<uint32_t>(from_height)
    });

    history_handler select_from_history = [on_reply, satoshi, algorithm](
        const history::list& rows)
    {
        points_value unspent;
        unspent.points.reserve(rows.size());

        for (auto& row: rows)
            if (row.spend.is_null())
                unspent.points.emplace_back(row.output, row.value);

        unspent.points.shrink_to_fit();
        points_value selected;
        select_outputs::select(selected, unspent, satoshi, algorithm);
        on_reply(selected);
    };

    send_request("blockchain.fetch_history2", data, on_error,
        std::bind(decode_history,
            _1, std::move(select_from_history)));
}

// Subscribers.
//-----------------------------------------------------------------------------

// address.subscribe is obsolete, but can pass through to address.subscribe2.
// Ths is a simplified overload for a non-private payment address subscription.
void proxy::address_subscribe(error_handler on_error, result_handler on_reply,
    const payment_address& address)
{
    static constexpr auto address_bits = short_hash_size * byte_bits;
    const auto prefix = binary(address_bits, address.hash());
    address_subscribe2(on_error, on_reply, prefix);
}

// Ths overload supports a prefix for either stealth or payment address.
void proxy::address_subscribe2(error_handler on_error, result_handler on_reply,
    const binary& prefix)
{
    // 20 * 8 = 160 bits.
    static constexpr auto address_bits = short_hash_size * byte_bits;

    // 4 * 8 = 32 bits.
    static constexpr auto stealth_bits = sizeof(uint32_t) * byte_bits;
    static_assert(stealth_bits <= address_bits, "unexpected bit length");

    const auto bit_length = prefix.size();

    if (bit_length > address_bits /*&& bit_length > stealth_bits*/)
    {
        on_error(error::bad_stream);
        return;
    }

    // [ prefix_bitsize:1 ]
    // [ prefix_blocks:...]
    const auto data = build_chunk(
    {
        to_array(static_cast<uint8_t>(bit_length)),
        prefix.blocks()
    });

    send_request("address.subscribe2", data, on_error,
        std::bind(decode_empty,
            _1, on_reply));
}

// Response handlers.
//-----------------------------------------------------------------------------

bool proxy::decode_empty(reader& payload, result_handler& handler)
{
    const code ec = payload.read_error_code();
    if (!payload.is_exhausted())
        return false;

    handler(ec);
    return true;
}

bool proxy::decode_transaction(reader& payload, transaction_handler& handler)
{
    transaction tx;
    if (!tx.from_data(payload) || !payload.is_exhausted())
        return false;

    handler(tx);
    return true;
}

bool proxy::decode_height(reader& payload, height_handler& handler)
{
    const auto last_height = payload.read_4_bytes_little_endian();
    if (!payload.is_exhausted())
        return false;

    handler(last_height);
    return true;
}

bool proxy::decode_block_header(reader& payload, block_header_handler& handler)
{
    chain::header header;
    if (!header.from_data(payload) || !payload.is_exhausted())
        return false;

    handler(header);
    return true;
}

bool proxy::decode_transaction_index(reader& payload,
    transaction_index_handler& handler)
{
    const auto block_height = payload.read_4_bytes_little_endian();
    const auto index = payload.read_4_bytes_little_endian();
    if (!payload.is_exhausted())
        return false;

    handler(block_height, index);
    return true;
}

// Address hash reversal is an idiosyncracy of the original Obelisk protocol.
stealth::list proxy::expand(stealth_compact::list& compact)
{
    // The sign byte of the ephmemeral key is fixed (0x02) by convention.
    static const auto sign = to_array(ec_even_sign);

    stealth::list result;

    for (const auto& in: compact)
    {
        stealth out;
        out.ephemeral_public_key = splice(sign, in.ephemeral_public_key_hash);
        out.public_key_hash = in.public_key_hash;
        std::reverse(out.public_key_hash.begin(), out.public_key_hash.end());
        out.transaction_hash = in.transaction_hash;
        result.emplace_back(out);
    }

    compact.clear();
    return result;
}

// Address hash is still reversed here, to be unreversed in expansion.
bool proxy::decode_stealth(reader& payload, stealth_handler& handler)
{
    chain::stealth_compact::list compact;

    while (!payload.is_exhausted())
    {
        chain::stealth_compact row;
        row.ephemeral_public_key_hash = payload.read_hash();
        row.public_key_hash = payload.read_short_hash();
        row.transaction_hash = payload.read_hash();
        compact.emplace_back(row);

        if (!payload)
            return false;
    }

    handler(expand(compact));
    return true;
}

history::list proxy::expand(history_compact::list& compact)
{
    history::list result;

    // Process and remove all outputs.
    for (auto output = compact.begin(); output != compact.end();)
    {
        if (output->kind == point_kind::output)
        {
            history row;
            row.output = output->point;
            row.output_height = output->height;
            row.value = output->value;
            row.spend = input_point{ null_hash, point::null_index };
            row.temporary_checksum = output->point.checksum();
            result.emplace_back(row);
            output = compact.erase(output);
            continue;
        }

        ++output;
    }

    // All outputs have been removed, process the spends.
    for (const auto& spend: compact)
    {
        auto found = false;

        // Update outputs with the corresponding spends.
        for (auto& row: result)
        {
            if (row.temporary_checksum == spend.previous_checksum &&
                row.spend.is_null())
            {
                row.spend = spend.point;
                row.spend_height = spend.height;
                found = true;
                break;
            }
        }

        // This will only happen if the history height cutoff comes between
        // an output and its spend. In this case we return just the spend.
        if (!found)
        {
            history row;
            row.output = output_point{ null_hash, point::null_index };
            row.output_height = max_uint64;
            row.value = max_uint64;
            row.spend = spend.point;
            row.spend_height = spend.height;
            result.emplace_back(row);
        }
    }

    compact.clear();

    // Clear all remaining checksums from unspent rows.
    for (auto& row: result)
        if (row.spend.is_null())
            row.spend_height = max_uint64;

    // TODO: sort by height and index of output, spend or both in order.
    return result;
}

// row.value || row.previous_checksum is a union, we just decode as row.value.
bool proxy::decode_history(reader& payload, history_handler& handler)
{
    chain::history_compact::list compact;

    while (!payload.is_exhausted())
    {
        chain::history_compact row;
        row.kind = static_cast<point_kind>(payload.read_byte());
        const auto success = row.point.from_data(payload);
        row.height = payload.read_4_bytes_little_endian();
        row.value = payload.read_8_bytes_little_endian();
        compact.push_back(row);

        if (!success || !payload)
            return false;
    }

    handler(expand(compact));
    return true;
}

////// This supports address.fetch_history (which is obsolete as of server v3).
////// In this scenario the server sends to the client a payload that matches the
////// output of decode_history(...), with the exception that spends orphaned by
////// the server's minimum history hieght not included in the payload.
////bool proxy::decode_expanded_history(reader& payload, history_handler& handler)
////{
////    chain::history::list expanded;
////
////    while (!payload.is_exhausted())
////    {
////        chain::history row;
////        auto success = row.output.from_data(payload);
////
////        // Storing uint32_t height into uint64_t.
////        row.output_height = payload.read_4_bytes_little_endian();
////        row.value = payload.read_8_bytes_little_endian();
////
////        // If there is no spend then input is null_hash/max_uint32/max_uint32.
////        success = success && row.spend.from_data(payload);
////
////        // Storing uint32_t height into uint64_t.
////        row.spend_height = payload.read_4_bytes_little_endian();
////        expanded.push_back(row);
////
////        if (!success || !payload)
////            return false;
////    }
////
////    handler(expanded);
////    return true;
////}

} // namespace client
} // namespace libbitcoin
