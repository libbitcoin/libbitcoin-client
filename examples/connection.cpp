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
static void on_update(const payment_address& address, size_t height,
    const hash_digest& blk_hash, const transaction_type&)
{
    std::cout << "update:" << address.encoded() << std::endl;
}

connection::connection(czmqpp::socket& socket)
    : stream(socket), codec(stream, on_update, on_unknown)
{
}
