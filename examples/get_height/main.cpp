/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
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
#include <iostream>
#include <memory>
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

    if (!socket.connect({ argv[1] }))
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

    // Make the request.
    socket_stream stream(socket);
    proxy proxy(stream, unknown_handler, 2000, 0);
    proxy.blockchain_fetch_last_height(error_handler, completion_handler);

    // Wait for the response.
    zmq::poller poller;
    poller.add(socket);
    const auto socket_id = socket.id();
    auto delay = proxy.refresh();

    while (!proxy.empty() && !poller.terminated() && !poller.expired() &&
        poller.wait(delay) == socket_id)
    {
        stream.read(proxy);

        // Update the time remaining.
        delay = proxy.refresh();
    }

    return 0;
}
