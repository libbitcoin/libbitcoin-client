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
#ifndef LIBBITCOIN_CLIENT_SOCKET_MESSAGE_STREAM_HPP
#define LIBBITCOIN_CLIENT_SOCKET_MESSAGE_STREAM_HPP

#include <czmq++/czmq.hpp>
#include <bitcoin/client/define.hpp>
#include <bitcoin/client/message_stream.hpp>
#include <bitcoin/client/zmq_pollable.hpp>

namespace libbitcoin {
namespace client {

class BCC_API socket_message_stream
: public message_stream, public zmq_pollable
{
public:

    socket_message_stream();

    socket_message_stream(czmqpp::socket& socket);

    socket_message_stream(const socket_message_stream&) = delete;

    ~socket_message_stream();

    virtual void write(const data_stack& data);

    virtual void add(czmqpp::poller& poller);

    virtual bool matches(czmqpp::poller& poller, czmqpp::socket& which);

    bool forward(message_stream& stream);

    bool forward(czmqpp::socket& socket);

private:

    czmqpp::socket socket_;
};

}
}

#endif

