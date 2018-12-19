/**
 * Copyright (c) 2011-2018 libbitcoin developers (see AUTHORS)
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
#include <bitcoin/client.hpp>

using namespace bc::system;
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

    const auto completion_handler = [](const code& ec, size_t height)
    {
        if (ec)
            std::cerr << "Failed retrieving height: " << ec.message() << std::endl;
        else
            std::cout << "Height: " << height << std::endl;
    };

    obelisk_client client;
    client.connect(config::endpoint(argv[1]));

    // Make the request.
    client.blockchain_fetch_last_height(completion_handler);
    client.wait();

    return 0;
}
