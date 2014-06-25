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
#ifndef LIBBITCOIN_CLIENT_SOCKET_HPP
#define LIBBITCOIN_CLIENT_SOCKET_HPP

#include <zmq.hpp>
#include <bitcoin/client/message_stream.hpp>

namespace libbitcoin {
namespace client {

/**
 * Represents a connection to the bitcoin server.
 */
class BC_API zmq_socket
  : public message_stream
{
public:
    BC_API zmq_socket(zmq::context_t& context, const std::string& server);

    /**
     * Creates a zeromq pollitem_t structure suitable for passing to the
     * zmq_poll function. When zmq_poll indicates that there is data waiting
     * to be read, simply call the `forward` method to process them.
     */
    BC_API zmq_pollitem_t pollitem();

    /**
     * Processes pending input messages, forwarding them to the given
     * message_stream interface.
     */
    BC_API void forward(message_stream& dest);

    /**
     * Sends an outgoing message through the socket.
     */
    BC_API virtual void message(const data_chunk& data, bool more);

private:
    zmq::socket_t socket_;
};

} // namespace client
} // namespace libbitcoin

#endif


