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

#include <bitcoin/client/message_stream.hpp>

namespace libbitcoin {
namespace client {

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

private:
    // Outgoing message stream:
    message_stream& out_;
};

} // namespace client
} // namespace libbitcoin

#endif

