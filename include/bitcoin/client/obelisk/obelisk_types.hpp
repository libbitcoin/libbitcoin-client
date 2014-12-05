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
#ifndef LIBBITCOIN_CLIENT_OBELISK_OBELISK_TYPES_HPP
#define LIBBITCOIN_CLIENT_OBELISK_OBELISK_TYPES_HPP

#include <bitcoin/client/define.hpp>

namespace libbitcoin {
namespace client {

typedef uint32_t stealth_bitfield;

struct BCC_API stealth_prefix
{
    uint8_t number_bits;
    stealth_bitfield bitfield;
};

//typedef bc::stealth_prefix address_prefix;

struct BCC_API history_row
{
    output_point output;
    size_t output_height;
    uint64_t value;
    input_point spend;
    size_t spend_height;
};

typedef std::vector<history_row> history_list;

struct BCC_API stealth_row
{
    data_chunk ephemkey;
    payment_address address;
    hash_digest transaction_hash;
};

typedef std::vector<stealth_row> stealth_list;

}
}

#endif
