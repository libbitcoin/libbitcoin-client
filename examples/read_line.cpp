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
#include "read_line.hpp"

#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include "zmq.hpp"

read_line::~read_line()
{
    socket_.send("", 1);
    thread_->join();
    delete thread_;
}

read_line::read_line(zmq::context_t& context)
  : socket_(context, ZMQ_REQ)
{
    socket_.bind("inproc://terminal");
    // The thread must be constructed after the socket is already bound.
    // The context must be passed by pointer to avoid copying.
    auto functor = std::bind(&read_line::run, this, &context);
    thread_ = new std::thread(functor);
}

void read_line::show_prompt()
{
    std::cout << "> " << std::flush;
    socket_.send("", 0);
}

zmq_pollitem_t read_line::pollitem()
{
    return zmq_pollitem_t
    {
        socket_, 0, ZMQ_POLLIN, 0
    };
}

std::string read_line::get_line()
{
    char line[1000];
    size_t size = socket_.recv(line, sizeof(line), ZMQ_DONTWAIT);
    return std::string(line, size);
}

void read_line::run(zmq::context_t* context)
{
    zmq::socket_t socket(*context, ZMQ_REP);
    socket.connect("inproc://terminal");

    while (true)
    {
        // Wait for a socket request:
        char request[1];
        if (socket.recv(request, sizeof(request)))
            return;

        // Read the input:
        char line[1000];
        std::cin.getline(line, sizeof(line));
        socket.send(line, std::strlen(line));
    }
}
