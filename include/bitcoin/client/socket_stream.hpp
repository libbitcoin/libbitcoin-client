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

#ifndef LIBBITCOIN_CLIENT_SOCKET_STREAM_HPP
#define LIBBITCOIN_CLIENT_SOCKET_STREAM_HPP

#include <memory>
#include <czmq++/czmqpp.hpp>
#include <bitcoin/client/define.hpp>
#include <bitcoin/client/message_stream.hpp>
//#include <bitcoin/client/response_stream.hpp>
//#include <bitcoin/client/request_stream.hpp>

namespace libbitcoin {
namespace client {

class BCC_API socket_stream
  : public message_stream //, public request_stream
{
public:

    socket_stream(czmqpp::socket& socket);

    virtual ~socket_stream();

    virtual void write(const data_stack& data);

//    virtual void write(
//        const std::shared_ptr<bc::protocol::request>& request);

    virtual bool signal_response(
        std::shared_ptr<message_stream> stream);

//    virtual bool signal_response(
//        std::shared_ptr<response_stream> stream);

    czmqpp::socket& get_socket();

private:

    czmqpp::socket socket_;
};

}
}

#endif
