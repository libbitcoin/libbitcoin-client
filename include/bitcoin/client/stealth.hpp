/**
 * Copyright (c) 2011-2018 libbitcoin developers (see AUTHORS)
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
#ifndef LIBBITCOIN_CLIENT_STEALTH_HPP
#define LIBBITCOIN_CLIENT_STEALTH_HPP

#include <vector>
#include <bitcoin/bitcoin.hpp>

namespace libbitcoin {
namespace client {

/// This structure is used between client and API callers in v2/v3.
/// The normal stealth row includes the sign byte of the ephemeral public key.
struct BCC_API stealth
{
    typedef std::vector<stealth> list;

    // Constructor provided for in-place construction.
    stealth(const ec_compressed& ephemeral_public_key,
        const short_hash& public_key_hash, const hash_digest& transaction_hash)
      : ephemeral_public_key(ephemeral_public_key),
        public_key_hash(public_key_hash),
        transaction_hash(transaction_hash)
    {
    }

    ec_compressed ephemeral_public_key;
    short_hash public_key_hash;
    hash_digest transaction_hash;
};

} // namespace client
} // namespace libbitcoin

#endif
