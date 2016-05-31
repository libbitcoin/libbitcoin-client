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
#include "client.hpp"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <bitcoin/client.hpp>
#include "connection.hpp"
#include "read_line.hpp"

using namespace bc::chain;
using namespace bc::client;
using namespace bc::protocol;
using namespace bc::wallet;

client::client()
  : done_(false),
    pending_request_(false),
    terminal_(context_)
{
}

void client::cmd_exit()
{
    done_ = true;
}

void client::cmd_help()
{
    std::cout << "commands:" << std::endl;
    std::cout << "  exit              - leave the program" << std::endl;
    std::cout << "  help              - this menu" << std::endl;
    std::cout << "  connect <server>  - connect to a server" << std::endl;
    std::cout << "  disconnect        - disconnect from the server" << std::endl;
    std::cout << "  height            - get last block height" << std::endl;
    std::cout << "  history <address> - get an address' history" << std::endl;
}

void client::cmd_connect(std::stringstream& args)
{
    std::string server;
    if (!read_string(args, server, "error: no server given"))
        return;

    std::cout << "connecting to " << server << std::endl;

    zmq::socket socket(context_, zmq::socket::role::dealer);

    if (!socket.connect(server))
        std::cout << "error: failed to connect" << std::endl;
    else
        connection_ = std::make_shared<connection>(socket, 6000);
}

void client::cmd_disconnect(std::stringstream&)
{
    if (!check_connection())
        return;

    connection_.reset();
}

void client::cmd_height(std::stringstream&)
{
    if (!check_connection())
        return;

    auto handler = [this](size_t height)
    {
        std::cout << "height: " << height << std::endl;
        request_done();
    };

    pending_request_ = true;
    connection_->proxy.blockchain_fetch_last_height(error_handler(), handler);
}

void client::cmd_history(std::stringstream& args)
{
    if (!check_connection())
        return;

    payment_address address;
    if (!read_address(args, address))
        return;

    auto handler = [this](const history::list& history)
    {
        for (const auto& row: history)
            std::cout << row.value << std::endl;

        request_done();
    };

    pending_request_ = true;
    connection_->proxy.address_fetch_history(error_handler(),
        handler, address);
}

int client::run()
{
    std::cout << "type \"help\" for instructions" << std::endl;
    terminal_.show_prompt();
    const auto terminal_socket_id = terminal_.socket().id();

    while (!done_)
    {
        uint32_t delay = -1;
        zmq::poller poller;
        poller.add(terminal_.socket());

        if (connection_)
        {
            poller.add(connection_->stream.socket());
            delay = connection_->proxy.refresh();
        }

        const auto ids = poller.wait(delay);

        if (poller.terminated())
            break;

        if (poller.expired())
            continue;

        if (ids.contains(terminal_socket_id))
        {
            command();
            continue;
        }

        if (ids.contains(connection_->stream.socket().id()) && connection_)
            connection_->stream.read(connection_->proxy);
        else
            std::cout << "connect before calling" << std::endl;
    }

    return 0;
}

void client::command()
{
    std::stringstream reader(terminal_.get_line());
    std::string command;
    reader >> command;

    if      (command == "exit")         cmd_exit();
    else if (command == "help")         cmd_help();
    else if (command == "connect")      cmd_connect(reader);
    else if (command == "disconnect")   cmd_disconnect(reader);
    else if (command == "height")       cmd_height(reader);
    else if (command == "history")      cmd_history(reader);
    else
        std::cout << "unknown command " << command << std::endl;

    // Display another prompt, if needed:
    if (!done_ && !pending_request_)
        terminal_.show_prompt();
}

proxy::error_handler client::error_handler()
{
    return [this](const std::error_code& ec)
    {
        std::cout << "error: " << ec.message() << std::endl;
        request_done();
    };
}

void client::request_done()
{
    pending_request_ = false;
    terminal_.show_prompt();
}

bool client::check_connection()
{
    if (!connection_)
    {
        std::cout << "error: no connection" << std::endl;
        return false;
    }

    return true;
}

bool client::read_string(std::stringstream& args, std::string& out,
    const std::string& error_message)
{
    args >> out;
    if (!out.size())
    {
        std::cout << error_message << std::endl;
        return false;
    }

    return true;
}

bool client::read_address(std::stringstream& args, payment_address& out)
{
    std::string address;
    if (!read_string(args, address, "error: no address given"))
        return false;

    payment_address payment(address);
    if (!payment)
    {
        std::cout << "error: invalid address " << address << std::endl;
        return false;
    }

    out = payment;
    return true;
}
