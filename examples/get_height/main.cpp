/**
 * Copyright (c) 2011-2017 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <iostream>
#include <memory>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/client.hpp>

using namespace bc;
using namespace bc::client;
using namespace bc::protocol;

/**
 * A minimal example that connects to a server and fetches height.
 */
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "usage: " << argv[0] << " <server>" << std::endl;
        return 1;
    }

    // Set up the server connection.
    zmq::context context;
    zmq::socket socket(context, zmq::socket::role::dealer);

    if (socket.connect({ argv[1] }) != error::success)
    {
        std::cerr << "Cannot connect to " << argv[1] << std::endl;
        return 1;
    }

    const auto unknown_handler = [](const std::string& command)
    {
        std::cout << "unknown command: " << command << std::endl;
    };

    const auto error_handler = [](const code& code)
    {
        std::cout << "error: " << code.message() << std::endl;
    };

    const auto completion_handler = [](size_t height)
    {
        std::cout << "height: " << height << std::endl;
    };

    socket_stream stream(socket);

    // Wait 2 seconds for the connection, with no failure retries.
    proxy proxy(stream, unknown_handler, 2000, 0, bc::settings());

    // Make the request.
    proxy.blockchain_fetch_last_height(error_handler, completion_handler);

    zmq::poller poller;
    poller.add(socket);

    // Wait 1 second for the response.
    if (poller.wait(1000).contains(socket.id()))
        stream.read(proxy);

    return 0;
}
