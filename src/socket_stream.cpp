/*
 * Copyright (c) 2011-2014 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin_client.
 *
 * libbitcoin_client is free software: you can redistribute it and/or
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

#include <bitcoin/client/socket_stream.hpp>

#include <bitcoin/protocol.hpp>

namespace libbitcoin {
namespace client {

socket_stream::socket_stream(std::shared_ptr<czmqpp::socket> socket)
  : socket_(socket)
{
}

socket_stream::~socket_stream()
{
}

void socket_stream::write(const std::shared_ptr<bc::protocol::request>& request)
{
    if (socket_ && request)
    {
        bc::protocol::request_message message;

        message.set_request(request);

        message.send(socket_);
    }
}

bool socket_stream::signal_response(std::shared_ptr<response_stream> stream)
{
    bool signaled = false;

    if (socket_ && stream)
    {
        bc::protocol::response_message message;

        if (message.receive(socket_))
        {
            std::shared_ptr<bc::protocol::response> response = message.get_response();

            if (response)
            {
                stream->write(response);
                signaled = true;
            }
        }
    }

    return signaled;
}

}
}
