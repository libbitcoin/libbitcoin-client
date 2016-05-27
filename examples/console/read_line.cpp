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
#include "read_line.hpp"

#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <bitcoin/client.hpp>

using namespace bc;
using namespace bc::protocol;

uint32_t signal_halt = 0;
uint32_t signal_continue = 1;

read_line::~read_line()
{
    zmq::message message;
    message.enqueue_little_endian(signal_halt);
    message.send(socket_);
    thread_->join();
}

read_line::read_line(zmq::context& context)
  : socket_(context, zmq::socket::role::requester)
{
    socket_.bind({ "inproc://terminal" });

    // The thread must be constructed after the socket is already bound.
    thread_ = std::make_shared<std::thread>(
        std::bind(&read_line::run,
            this, std::ref(context)));
}

void read_line::show_prompt()
{
    std::cout << "> " << std::flush;
    zmq::message message;
    message.enqueue_little_endian(signal_continue);
    message.send(socket_);
}

std::string read_line::get_line()
{
    zmq::message message;
    zmq::poller poller;
    poller.add(socket_);
    const auto id = poller.wait(1);

    if (id == socket_.id() && !poller.expired() && !poller.terminated())
        if (message.receive(socket_))
            return message.dequeue_text();

    return{};
}

void read_line::run(zmq::context& context)
{
    zmq::socket socket(context, zmq::socket::role::replier);
    socket.connect({ "inproc://terminal" });

    while (true)
    {
        uint32_t signal;
        zmq::message message;

        if (!message.receive(socket) || !message.dequeue(signal) ||
            signal == signal_halt)
        {
            break;
        }

        // Read input:
        char line[1000];
        std::cin.getline(line, sizeof(line));
        std::string text(line);

        zmq::message response;
        response.enqueue(text);
        response.send(socket);
    }
}

zmq::socket& read_line::socket()
{
    return socket_;
}
