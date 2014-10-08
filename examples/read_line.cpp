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
#include <string>
#include <thread>
#include <czmq++/czmqpp.hpp>
#include <bitcoin/client.hpp>

uint32_t signal_halt = 0;
uint32_t signal_continue = 1;

read_line::~read_line()
{
    czmqpp::message message;
    message.append(bc::to_data_chunk(bc::to_little_endian(signal_halt)));
    message.send(socket_);
    thread_->join();
    delete thread_;
}

read_line::read_line(czmqpp::context& context)
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
    czmqpp::message message;
    message.append(bc::to_data_chunk(bc::to_little_endian(signal_continue)));
    message.send(socket_);
}

std::string read_line::get_line()
{
    std::string data;
    czmqpp::message message;
    czmqpp::poller poller(socket_);

    czmqpp::socket which = poller.wait(1);

    if (!poller.expired() && !poller.terminated() && (which == socket_))
    {
        if (message.receive(socket_))
        {
            data = std::string(message.parts()[0].begin(), message.parts()[0].end());
        }
    }

    return data;
}

void read_line::run(czmqpp::context* context)
{
    czmqpp::socket socket(*context, ZMQ_REP);
    socket.connect("inproc://terminal");

    while (true)
    {
        czmqpp::message request;

        // Wait for a socket request:
        if (request.receive(socket))
        {
            czmqpp::data_stack stack = request.parts();

            uint32_t signal = bc::from_little_endian<uint32_t>((*(stack.begin())).begin());

            if (signal == signal_halt)
            {
                break;
            }

            // Read input:
            char line[1000];
            std::cin.getline(line, sizeof(line));

            std::string response_text(line);

            czmqpp::message response;
            response.append(czmqpp::data_chunk(response_text.begin(), response_text.end()));
            response.send(socket);
        }
        else
        {
            break;
        }
    }
}

czmqpp::socket& read_line::get_socket()
{
    return socket_;
}

