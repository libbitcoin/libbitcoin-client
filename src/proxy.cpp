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
#include <utility>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/client/dealer.hpp>
#include <bitcoin/client/define.hpp>
#include <bitcoin/client/history.hpp>
#include <bitcoin/client/stealth.hpp>
#include <bitcoin/client/stream.hpp>

namespace libbitcoin {
namespace client {

using namespace std::placeholders;
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

void proxy::blockchain_fetch_stealth2(error_handler on_error,
    stealth_handler on_reply, const binary& prefix, uint32_t from_height)
{
    const auto bits = prefix.size();

    if (bits < stealth_address::min_filter_bits || 
        bits > stealth_address::max_filter_bits)
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

    send_request("blockchain.fetch_stealth2", data, on_error,
        std::bind(decode_stealth,
            _1, on_reply));
}

// blockchain.fetch_history3 (v3.1) does not accept a version byte.
// blockchain.fetch_history2 (v3.0) ignored version and is obsoleted in v3.1.
// blockchain.fetch_history (v1/v2) used hash reversal and is obsoleted in v3.
void proxy::blockchain_fetch_history3(error_handler on_error,
    history_handler on_reply, const payment_address& address,
    uint32_t from_height)
{
    const auto data = build_chunk(
    {
        address.hash(),
        to_little_endian<uint32_t>(from_height)
    });

    send_request("blockchain.fetch_history3", data, on_error,
        std::bind(decode_history,
            _1, on_reply));
}

void proxy::blockchain_fetch_unspent_outputs(error_handler on_error,
    points_value_handler on_reply, const payment_address& address,
    uint64_t satoshi, select_outputs::algorithm algorithm)
{
    static constexpr uint32_t from_height = 0;

    const auto data = build_chunk(
    {
        address.hash(),
        to_little_endian<uint32_t>(from_height)
    });

    history_handler select_from_history = [on_reply, satoshi, algorithm](
        const history::list& rows)
    {
        points_value unspent;
        unspent.points.reserve(rows.size());

        for (const auto& row: rows)
            if (row.spend.is_null())
                unspent.points.emplace_back(row.output, row.value);

        unspent.points.shrink_to_fit();
        points_value selected;
        select_outputs::select(selected, unspent, satoshi, algorithm);
        on_reply(selected);
    };

    send_request("blockchain.fetch_history3", data, on_error,
        std::bind(decode_history,
            _1, std::move(select_from_history)));
}

// Subscribers.
//-----------------------------------------------------------------------------

// address.subscribe is obsolete, but can pass through to address.subscribe2.
// Ths is a simplified overload for a non-private payment address subscription.
void proxy::subscribe_address(error_handler on_error, result_handler on_reply,
    const short_hash& address_hash)
{
    // [ address_hash:20 ]
    const auto data = build_chunk({ address_hash });

    send_request("subscribe.address", data, on_error,
        std::bind(decode_empty,
            _1, on_reply));
}

// Ths overload supports a prefix for either stealth or payment address.
void proxy::subscribe_stealth(error_handler on_error, result_handler on_reply,
    const binary& stealth_prefix)
{
    const auto bits = stealth_prefix.size();

    if (bits < stealth_address::min_filter_bits ||
        bits > stealth_address::max_filter_bits)
    {
        on_error(error::bad_stream);
        return;
    }

    // [ prefix_bitsize:1 ]
    // [ prefix_blocks:...]
    const auto data = build_chunk(
    {
        to_array(static_cast<uint8_t>(bits)),
        stealth_prefix.blocks()
    });

    send_request("subscribe.stealth", data, on_error,
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

stealth::list proxy::expand(stealth_record::list& records)
{
    // TODO: just use stealth_record::list here.
    stealth::list result;
    result.reserve(records.size());

    for (const auto& in: records)
    {
        result.push_back(stealth
        {
            in.ephemeral_public_key(),
            std::move(in.public_key_hash()),
            std::move(in.transaction_hash())
        });
    }

    return result;
}

bool proxy::decode_stealth(reader& payload, stealth_handler& handler)
{
    stealth_record stealth;
    stealth_record::list records;

    while (!payload.is_exhausted())
    {
        if (stealth.from_data(payload, true))
            records.push_back(stealth);
        else
            return false;
    }

    handler(expand(records));
    return true;
}

history::list proxy::expand(payment_record::list& records)
{
    // TODO: can we use use history_record::list here?
    history::list result;
    result.reserve(records.size());

    std::vector<uint8_t> isDeleted(records.size(), 0);
    
    std::unordered_multimap<uint64_t, size_t> resultCache;
    
    // Process and remove all outputs.
    for (auto record = records.begin(); record != records.end();)
    {
        if (record->is_output())
        {
            result.push_back(history
            {
                std::move(record->point()),
                record->height(),
                record->data(),
                input_point{ null_hash, point::null_index },
                { record->point().checksum() }
            });

            resultCache.insert(std::make_pair(result.back().temporary_checksum, result.size() - 1));

            isDeleted[std::distance(records.begin(), record)] = 1;
        }

        // Skip the input.
        ++record;
    }

    // TODO: reduce to output set with distinct checksums, as a fault signal.
    ////std::sort(result.begin(), result.end());
    ////result.erase(std::unique(result.begin(), result.end()), result.end());

    // All outputs have been removed, process the spends.
    size_t i = 0;
    for (auto& record: records)
    {
        size_t prevI = i;
        i++;
        if (isDeleted[prevI] == 1) {
            continue;
        }
        
        auto found = false;

        // Update outputs with the corresponding spends.
        // This relies on the lucky avoidance of checksum hash collisions :<.
        // Ordering is insufficient since the server may write concurrently.
        const auto foundInCache = resultCache.equal_range(record.data());
        for (auto it = foundInCache.first; it != foundInCache.second; it++) 
        {
            auto &history = result.at(it->second);
            // The temporary_checksum is a union with spend_height, so we must
            // guard against reading temporary_checksum unless spend is null.
            if (history.spend.is_null())
            {
                // Move the spend to the row of its correlated output.
                history.spend = std::move(record.point());
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
            result.push_back(history
            {
                output_point{ null_hash, point::null_index },
                max_size_t,
                max_uint64,
                std::move(record.point()),
                { record.height() }
            });
        }
    }

    result.shrink_to_fit();
    
    // Clear all remaining checksums from unspent rows.
    for (auto& history: result)
        if (history.spend.is_null())
            history.spend_height = max_uint64;
        
    // TODO: sort by height and index of output, spend or both in order.
    return result;
}

// row.value || row.previous_checksum is a union, we just decode as row.value.
bool proxy::decode_history(reader& payload, history_handler& handler)
{
    payment_record payment;
    payment_record::list records;

    while (!payload.is_exhausted())
    {
        if (payment.from_data(payload, true))
            records.push_back(payment);
        else
            return false;
    }

    handler(expand(records));
    return true;
}

} // namespace client
} // namespace libbitcoin
