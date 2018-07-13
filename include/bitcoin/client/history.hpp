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
#ifndef LIBBITCOIN_CLIENT_HISTORY_HPP
#define LIBBITCOIN_CLIENT_HISTORY_HPP

#include <cstddef>
#include <cstdint>
#include <bitcoin/bitcoin.hpp>

namespace libbitcoin {
namespace client {

/// This structure is used between client and API callers in v3.
/// This structure models the client-server protocol in v1/v2.
struct BCC_API history
{
    typedef std::vector<history> list;

    /// If there is no output this is null_hash:max.
    chain::output_point output;
    uint64_t output_height;

    /// The satoshi value of the output.
    uint64_t value;

    /// If there is no spend this is null_hash:max.
    chain::input_point spend;

    union
    {
        /// The height of the spend or max if no spend.
        uint64_t spend_height;

        /// During expansion this value temporarily doubles as a checksum.
        uint64_t temporary_checksum;
    };
};

} // namespace client
} // namespace libbitcoin

#endif
