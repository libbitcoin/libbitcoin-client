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
    // No sign byte in public key.
    hash_digest ephemkey;
    // No version byte in address.
    short_hash address;
    hash_digest transaction_hash;
};

typedef std::vector<stealth_row> stealth_list;

}
}

#endif
