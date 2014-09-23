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
#ifndef BITCOIN_CLIENT_CONNECTION_HPP
#define BITCOIN_CLIENT_CONNECTION_HPP

#include <memory>
#include <bitcoin/client.hpp>
#include <czmq++/czmq.hpp>

/**
 * A dynamically-allocated structure holding the resources needed for a
 * connection to a bitcoin server.
 */
class connection
{
public:
    connection(czmqpp::socket& socket);

    bc::client::socket_message_stream stream;
    bc::client::obelisk_codec codec;
};

#endif
