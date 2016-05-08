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
#include <czmq++/czmqpp.hpp>
#include <bitcoin/client.hpp>

using namespace bc;
using namespace bc::client;

/**
 * A minimalist example that connects to a server and fetches height.
 */
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "usage: " << argv[0] << " <server>" << std::endl;
        return 1;
    }

    // Set up the server connection.
    czmqpp::context context;
    czmqpp::socket socket(context, ZMQ_DEALER);

    if (socket.connect(argv[1]) < 0)
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
    obelisk_codec codec(stream, unknown_handler, 2000, 0);
    czmqpp::poller poller(socket);

    // Make the request.
    codec.fetch_last_height(error_handler, completion_handler);

    // Figure out how much time we have left.
    auto remainder_ms = codec.refresh();

    // Wait for the response:
    while (!codec.empty() && !poller.terminated() && !poller.expired() &&
        poller.wait(remainder_ms) == socket)
    {
        stream.read(codec);

        // Update the time remaining.
        remainder_ms = codec.refresh();
    }

    return 0;
}
