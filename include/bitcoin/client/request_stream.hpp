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

#ifndef LIBBITCOIN_CLIENT_REQUEST_STREAM_HPP
#define LIBBITCOIN_CLIENT_REQUEST_STREAM_HPP

#include <memory>
#include <bitcoin/protocol.hpp>
#include <bitcoin/client/define.hpp>

namespace libbitcoin {
namespace client {

class request_stream
{
public:

    BCC_API virtual ~request_stream() {};

    BCC_API virtual void write(
        const std::shared_ptr<bc::protocol::request>& request) = 0;
};

}
}

#endif
