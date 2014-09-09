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
#include <client/zeromq_socket.hpp>

#include <client/define.hpp>

namespace libbitcoin {
namespace client {

BC_API zeromq_socket::~zeromq_socket()
{
    if (socket_)
        zmq_close(socket_);
}

BC_API zeromq_socket::zeromq_socket(void* context, int type)
  : socket_(nullptr)
{
    socket_ = zmq_socket(context, type);
    if (socket_)
    {
        // Do not wait for unsent messages when shutting down:
        int linger = 0;
        zmq_setsockopt(socket_, ZMQ_LINGER, &linger, sizeof(linger));
    }
}

BC_API bool zeromq_socket::connect(const std::string& address)
{
    return socket_ && 0 <= zmq_connect(socket_, address.c_str());
}

BC_API bool zeromq_socket::bind(const std::string& address)
{
    return socket_ && 0 <= zmq_bind(socket_, address.c_str());
}

BC_API zmq_pollitem_t zeromq_socket::pollitem()
{
    BITCOIN_ASSERT(socket_);

    return zmq_pollitem_t
    {
        socket_, 0, ZMQ_POLLIN, 0
    };
}

BC_API bool zeromq_socket::forward(message_stream& dest)
{
    BITCOIN_ASSERT(socket_);

    while (pending())
    {
        zmq_msg_t msg;
        zmq_msg_init(&msg);

        if (zmq_msg_recv(&msg, socket_, ZMQ_DONTWAIT) < 0)
            return false;

        const char* raw = reinterpret_cast<const char*>(zmq_msg_data(&msg));
        libbitcoin::data_chunk data(raw, raw + zmq_msg_size(&msg));
        dest.message(data, zmq_msg_more(&msg) != 0);
        zmq_msg_close(&msg);
    }
    return true;
}

BC_API bool zeromq_socket::forward(zeromq_socket& dest)
{
    BITCOIN_ASSERT(socket_);

    while (pending())
    {
        zmq_msg_t msg;
        zmq_msg_init(&msg);

        if (zmq_msg_recv(&msg, socket_, ZMQ_DONTWAIT) < 0)
            return false;

        int flags = 0;
        if (zmq_msg_more(&msg))
            flags = ZMQ_SNDMORE;
        if (zmq_msg_send(&msg, dest.socket_, flags) < 0)
        {
            zmq_msg_close(&msg);
            return false;
        }

    }
    return true;
}

BC_API void zeromq_socket::message(const data_chunk& data, bool more)
{
    BITCOIN_ASSERT(socket_);

    int flags = 0;
    if (more)
        flags = ZMQ_SNDMORE;
    // The message won't go through if this fails,
    // but we are prepared for that possibility anyhow:
    zmq_send(socket_, data.data(), data.size(), flags);
}

bool zeromq_socket::pending()
{
    int events = 0;
    size_t size = sizeof(events);
    if (zmq_getsockopt(socket_, ZMQ_EVENTS, &events, &size) < 0)
        return false;
    return events & ZMQ_POLLIN;
}

} // namespace client
} // namespace libbitcoin
