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

#include <bitcoin/client/socket_message_stream.hpp>

namespace libbitcoin {
namespace client {

socket_message_stream::socket_message_stream()
{
}

socket_message_stream::socket_message_stream(czmqpp::socket& socket)
: socket_(socket.self())
{
}

socket_message_stream::~socket_message_stream()
{
}

void socket_message_stream::write(const data_stack& data)
{
    czmqpp::message message;

    for (auto chunk: data)
    {
        message.append(chunk);
    }

    message.send(socket_);
}

void socket_message_stream::add(czmqpp::poller& poller)
{
    poller.add(socket_);
}

bool socket_message_stream::matches(czmqpp::poller& poller, czmqpp::socket& which)
{
    return !poller.expired() && !poller.terminated() && (which == socket_);
}

bool socket_message_stream::forward(message_stream& stream)
{
    bool success = false;

    czmqpp::message message;

    if (message.receive(socket_))
    {
        stream.write(message.parts());
        success = true;
    }

    return success;
}

bool socket_message_stream::forward(czmqpp::socket& socket)
{
    bool success = false;

    czmqpp::message message;

    if (message.receive(socket_))
    {
        message.send(socket);
        success = true;
    }

    return success;
}

}
}
