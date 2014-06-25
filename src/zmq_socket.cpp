/*
 * Copyright (c) 2011-2014 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <bitcoin/client/zmq_socket.hpp>

namespace libbitcoin {
namespace client {

BC_API zmq_socket::zmq_socket(zmq::context_t& context, const std::string& server)
  : socket_(context, ZMQ_DEALER)
{
    // Do not wait for unsent messages when shutting down:
    int linger = 0;
    socket_.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
    socket_.connect(server.c_str());
}

BC_API zmq_pollitem_t zmq_socket::pollitem()
{
    return zmq_pollitem_t
    {
        socket_, 0, ZMQ_POLLIN, 0
    };
}

BC_API void zmq_socket::forward(message_stream& dest)
{
    while (true)
    {
        int events = 0;
        size_t size = sizeof(events);
        socket_.getsockopt(ZMQ_EVENTS, &events, &size);
        if (!(events & ZMQ_POLLIN))
            break;

        zmq::message_t msg;
        socket_.recv(&msg, ZMQ_DONTWAIT);

        const char* raw = reinterpret_cast<const char*>(msg.data());
        libbitcoin::data_chunk data(raw, raw + msg.size());
        dest.message(data, msg.more());
    }
}

BC_API void zmq_socket::message(const data_chunk& data, bool more)
{
    int flags = 0;
    if (more)
        flags = ZMQ_SNDMORE;
    socket_.send(data.data(), data.size(), flags);
}

} // namespace client
} // namespace libbitcoin
