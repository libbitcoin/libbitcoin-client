/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
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
#include <bitcoin/client/obelisk/obelisk_codec.hpp>

#include <bitcoin/bitcoin.hpp>
#include <bitcoin/client/define.hpp>

namespace libbitcoin {
namespace client {

using std::placeholders::_1;

/**
 * Reverses data.
 * Due to an unfortunate historical accident,
 * the obelisk wire format puts address hashes in reverse order.
 */
template <typename T>
T reverse(const T& in)
{
    T out;
    std::reverse_copy(in.begin(), in.end(), out.begin());
    return out;
}

BCC_API obelisk_codec::obelisk_codec(
    std::shared_ptr<message_stream> out,
    update_handler on_update,
    unknown_handler on_unknown,
    period_ms timeout,
    uint8_t retries)
  : obelisk_router(out)
{
    set_on_update(std::move(on_update));
    set_on_unknown(std::move(on_unknown));
    set_timeout(timeout);
    set_retries(retries);
}

obelisk_codec::~obelisk_codec()
{
}

BCC_API void obelisk_codec::fetch_history(error_handler on_error,
    fetch_history_handler on_reply,
    const wallet::payment_address& address, uint32_t from_height)
{
    // This reversal on the wire is an idiosyncracy of the Obelisk protocol.
    // We un-reverse here to limit confusion downsteam.

    auto data = build_chunk({
        to_array(address.version()),
        reverse(address.hash()),
        to_little_endian<uint32_t>(from_height)
    });

    send_request("blockchain.fetch_history", data, std::move(on_error),
        std::bind(decode_fetch_history, _1, std::move(on_reply)));
}

BCC_API void obelisk_codec::fetch_transaction(error_handler on_error,
    fetch_transaction_handler on_reply,
    const hash_digest& tx_hash)
{
    auto data = build_chunk({
        tx_hash
    });

    send_request("blockchain.fetch_transaction", data, std::move(on_error),
        std::bind(decode_fetch_transaction, _1, std::move(on_reply)));
}

BCC_API void obelisk_codec::fetch_last_height(error_handler on_error,
    fetch_last_height_handler on_reply)
{
    auto data = data_chunk();

    send_request("blockchain.fetch_last_height", data,
        std::move(on_error),
        std::bind(decode_fetch_last_height, _1, std::move(on_reply)));
}

BCC_API void obelisk_codec::fetch_block_header(error_handler on_error,
    fetch_block_header_handler on_reply,
    uint32_t height)
{
    auto data = build_chunk({
        to_little_endian<uint32_t>(height)
    });

    send_request("blockchain.fetch_block_header", data, std::move(on_error),
        std::bind(decode_fetch_block_header, _1, std::move(on_reply)));
}

BCC_API void obelisk_codec::fetch_block_header(error_handler on_error,
    fetch_block_header_handler on_reply,
    const hash_digest& blk_hash)
{
    auto data = build_chunk({
        blk_hash
    });

    send_request("blockchain.fetch_block_header", data, std::move(on_error),
        std::bind(decode_fetch_block_header, _1, std::move(on_reply)));
}

BCC_API void obelisk_codec::fetch_transaction_index(error_handler on_error,
    fetch_transaction_index_handler on_reply,
    const hash_digest& tx_hash)
{
    auto data = build_chunk({
        tx_hash
    });

    send_request("blockchain.fetch_transaction_index", data,
        std::move(on_error),
        std::bind(decode_fetch_transaction_index, _1, std::move(on_reply)));
}

BCC_API void obelisk_codec::fetch_stealth(error_handler on_error,
    fetch_stealth_handler on_reply,
    const binary& prefix, uint32_t from_height)
{
    // should this be a throw or should there be a return type instead?
    if (prefix.size() > max_uint8)
    {
        on_error(std::make_error_code(std::errc::bad_address));
        return;
    }

    auto data = build_chunk({
        to_array(static_cast<uint8_t>(prefix.size())),
        prefix.blocks(),
        to_little_endian<uint32_t>(from_height)
    });

    send_request("blockchain.fetch_stealth", data, std::move(on_error),
        std::bind(decode_fetch_stealth, _1, std::move(on_reply)));
}

BCC_API void obelisk_codec::validate(error_handler on_error,
    validate_handler on_reply,
    const chain::transaction& tx)
{
    send_request("transaction_pool.validate", tx.to_data(),
        std::move(on_error),
        std::bind(decode_validate, _1, std::move(on_reply)));
}

BCC_API void obelisk_codec::fetch_unconfirmed_transaction(
    error_handler on_error,
    fetch_transaction_handler on_reply,
    const hash_digest& tx_hash)
{
    auto data = build_chunk({
        tx_hash
    });

    send_request("transaction_pool.fetch_transaction", data,
        std::move(on_error),
        std::bind(decode_fetch_transaction, _1, std::move(on_reply)));
}

BCC_API void obelisk_codec::broadcast_transaction(error_handler on_error,
    empty_handler on_reply,
    const chain::transaction& tx)
{
    send_request("protocol.broadcast_transaction", tx.to_data(),
        std::move(on_error),
        std::bind(decode_empty, _1, std::move(on_reply)));
}

BCC_API void obelisk_codec::address_fetch_history(error_handler on_error,
    fetch_history_handler on_reply,
    const wallet::payment_address& address, uint32_t from_height)
{
    // This reversal on the wire is an idiosyncracy of the Obelisk protocol.
    // We un-reverse here to limit confusion downsteam.

    auto data = build_chunk({
        to_array(address.version()),
        reverse(address.hash()),
        to_little_endian<uint32_t>(from_height)
    });

    send_request("address.fetch_history", data, std::move(on_error),
        std::bind(decode_fetch_history, _1, std::move(on_reply)));
}

BCC_API void obelisk_codec::subscribe(error_handler on_error,
    empty_handler on_reply,
    const wallet::payment_address& address)
{
    binary prefix((short_hash_size * byte_bits), address.hash());

    // [ type:1 ] (0 = address prefix, 1 = stealth prefix)
    // [ prefix_bitsize:1 ]
    // [ prefix_blocks:...  ]
    auto data = build_chunk({
        to_array(static_cast<uint8_t>(subscribe_type::address)),
        to_array(static_cast<uint8_t>(prefix.size())),
        prefix.blocks()
    });

    send_request("address.subscribe", data, std::move(on_error),
        std::bind(decode_empty, _1, std::move(on_reply)));
}

BCC_API void obelisk_codec::subscribe(error_handler on_error,
    empty_handler on_reply, subscribe_type discriminator,
    const binary& prefix)
{
    // should this be a throw or should there be a return type instead?
    if (prefix.size() > max_uint8)
    {
        on_error(std::make_error_code(std::errc::bad_address));
        return;
    }

    auto data = build_chunk({
        to_array(static_cast<uint8_t>(discriminator)),
        to_array(static_cast<uint8_t>(prefix.size())),
        prefix.blocks()
    });

    send_request("address.subscribe", data, std::move(on_error),
        std::bind(decode_empty, _1, std::move(on_reply)));
}

// See below for description of updates data format.
//enum class subscribe_type : uint8_t
//{
//    address = 0,
//    stealth = 1
//};
//
//BCC_API void obelisk_codec::subscribe(error_handler on_error,
//    empty_handler on_reply,
//    const address_prefix& prefix)
//{
//    // BUGBUG: assertion is not good enough here.
//    BITCOIN_ASSERT(prefix.size() <= 255);
//
//    // [ type:1 ] (0 = address prefix, 1 = stealth prefix)
//    // [ prefix_bitsize:1 ]
//    // [ prefix_blocks:...  ]
//    auto data = build_chunk({
//        to_array(static_cast<uint8_t>(subscribe_type::address)),
//        to_array(prefix.size()),
//        prefix.blocks()
//    });
//
//    send_request("address.subscribe", data, std::move(on_error),
//        std::bind(decode_empty, _1, std::move(on_reply)));
//}

/**
See also libbitcoin-server repo subscribe_manager::post_updates() and
subscribe_manager::post_stealth_updates().

The address result is:

    Response command = "address.update"

    [ version:1 ]
    [ hash:20 ]
    [ height:4 ]
    [ block_hash:32 ]

    struct BCC_API address_subscribe_result
    {
        payment_address address;
        uint32_t height;
        hash_digest block_hash;
    };

When the subscription type is stealth, then the result is:

    Response command = "address.stealth_update"

    [ 32 bit prefix:4 ]
    [ height:4 ]
    [ block_hash:32 ]
    
    // Currently not used.
    struct BCC_API stealth_subscribe_result
    {
        typedef byte_array<4> stealth_prefix_bytes;
        // Protocol will send back 4 bytes of prefix.
        // See libbitcoin-server repo subscribe_manager::post_stealth_updates()
        stealth_prefix_bytes prefix;
        uint32_t height;
        hash_digest block_hash;
    };

Subscriptions expire after 10 minutes. Therefore messages with the command
"address.renew" should be sent periodically to the server. The format
is the same as for "address.subscribe, and the server will respond
with a 4 byte error code.
*/

bool obelisk_codec::decode_empty(reader& payload,
    empty_handler& handler)
{
    auto success = payload.is_exhausted();

    if (success)
        handler();

    return success;
}

bool obelisk_codec::decode_fetch_history(reader& payload,
    fetch_history_handler& handler)
{
    auto success = true;
    history_list history;

    while (success && payload && !payload.is_exhausted())
    {
        history_row row;
        success = row.output.from_data(payload);
        row.output_height = payload.read_4_bytes_little_endian();
        row.value = payload.read_8_bytes_little_endian();
        success &= row.spend.from_data(payload);
        row.spend_height = payload.read_4_bytes_little_endian();
        history.push_back(row);
    }

    if (success && payload)
        handler(history);

    return success && payload;
}

bool obelisk_codec::decode_fetch_transaction(reader& payload,
    fetch_transaction_handler& handler)
{
    chain::transaction tx;
    auto success = tx.from_data(payload);
    success &= payload.is_exhausted();

    if (success)
        handler(tx);

    return success;
}

bool obelisk_codec::decode_fetch_last_height(reader& payload,
    fetch_last_height_handler& handler)
{
    uint32_t last_height = payload.read_4_bytes_little_endian();
    bool success = payload.is_exhausted();

    if (success)
        handler(last_height);

    return success;
}

bool obelisk_codec::decode_fetch_block_header(reader& payload,
    fetch_block_header_handler& handler)
{
    chain::header header;
    auto success = header.from_data(payload, false);
    success &= payload.is_exhausted();

    if (success)
        handler(header);

    return success;
}

bool obelisk_codec::decode_fetch_transaction_index(reader& payload,
    fetch_transaction_index_handler& handler)
{
    uint32_t block_height = payload.read_4_bytes_little_endian();
    uint32_t index = payload.read_4_bytes_little_endian();
    bool success = payload.is_exhausted();

    if (success)
        handler(block_height, index);

    return success;
}

bool obelisk_codec::decode_fetch_stealth(reader& payload,
    fetch_stealth_handler& handler)
{
    auto success = true;
    stealth_list results;

    while (success && !payload.is_exhausted())
    {
        stealth_row row;

        // The sign byte of the ephmemeral key is fixed (0x02) by convetion.
        // To recover the key concatenate: [0x02, ephemeral_key_hash]. 
        row.ephemeral_public_key = splice(to_array(ephemeral_public_key_sign),
            payload.read_hash());

        // This reversal on the wire is an idiosyncracy of the Obelisk protocol.
        // We un-reverse here to limit confusion downsteam.
        // See libbitcoin-server COMPAT_fetch_history.
        const auto address_hash = payload.read_short_hash();
        row.public_key_hash = reverse(address_hash);

        row.transaction_hash = payload.read_hash();
        results.push_back(row);
        success = payload;
    }

    if (success)
        handler(results);

    return success;
}

bool obelisk_codec::decode_validate(reader& payload,
    validate_handler& handler)
{
    auto success = true;
    index_list unconfirmed;

    while (success && payload.is_exhausted())
    {
        unconfirmed.push_back(payload.read_4_bytes_little_endian());
        success = payload;
    }

    if (success)
        handler(unconfirmed);

    return success;
}

} // namespace client
} // namespace libbitcoin
