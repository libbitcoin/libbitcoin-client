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
#include "connection.hpp"

#include <iostream>
#include <string>
#include <czmq++/czmqpp.hpp>
#include <bitcoin/client.hpp>

using namespace bc;

/**
 * Unknown message callback handler.
 */
static void on_unknown(const std::string& command)
{
    std::cout << "unknown message:" << command << std::endl;
}

/**
 * Update message callback handler.
 */
static void on_update(const bc::wallet::payment_address& address, size_t,
    const hash_digest&, const chain::transaction&)
{
    std::cout << "update:" << address.encoded() << std::endl;
}

connection::connection(czmqpp::socket& socket)
{
    stream = std::make_shared<bc::client::socket_stream>(socket);

    std::shared_ptr<bc::client::message_stream> base_stream
        = std::static_pointer_cast<bc::client::message_stream>(stream);

    codec = std::make_shared<bc::client::obelisk_codec>(
        base_stream, on_update, on_unknown);
}

/*
std::shared_ptr<bc::client::socket_stream> connection::get_stream()
{
    return stream_;
}

std::shared_ptr<bc::client::obelisk_codec> connection::get_codec()
{
    return codec_;
}
*/
