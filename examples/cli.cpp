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
#include "cli.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <czmq++/czmqpp.hpp>
#include <bitcoin/client.hpp>
#include "connection.hpp"
#include "read_line.hpp"

using namespace bc;

cli::cli()
  : done_(false),
    pending_request_(false),
    terminal_(context_)
{
}

cli::~cli()
{
}

void cli::cmd_exit()
{
    done_ = true;
}

void cli::cmd_help()
{
    std::cout << "commands:" << std::endl;
    std::cout << "  exit              - leave the program" << std::endl;
    std::cout << "  help              - this menu" << std::endl;
    std::cout << "  connect <server>  - connect to a server" << std::endl;
    std::cout << "  disconnect        - disconnect from the server" << std::endl;
    std::cout << "  height            - get last block height" << std::endl;
    std::cout << "  history <address> - get an address' history" << std::endl;
}

void cli::cmd_connect(std::stringstream& args)
{
    std::string server;
    if (!read_string(args, server, "error: no server given"))
        return;
    std::cout << "connecting to " << server << std::endl;

    czmqpp::socket obelisk_socket(context_, ZMQ_DEALER);

    if (obelisk_socket.connect(server) >= 0)
    {
        connection_ = std::make_shared<connection>(obelisk_socket);
        connection_->codec->set_timeout(std::chrono::seconds(60));
    }
    else
    {
        std::cout << "error: failed to connect" << std::endl;
    }
}

void cli::cmd_disconnect(std::stringstream& args)
{
    if (!check_connection())
        return;
    connection_.reset();
}

void cli::cmd_height(std::stringstream& args)
{
    if (!check_connection())
        return;

    auto handler = [this](size_t height)
    {
        std::cout << "height: " << height << std::endl;
        request_done();
    };
    pending_request_ = true;
    connection_->codec->fetch_last_height(error_handler(), handler);
}

void cli::cmd_history(std::stringstream& args)
{
    if (!check_connection())
        return;
    payment_address address;
    if (!read_address(args, address))
        return;

    auto handler = [this](const client::history_list& history)
    {
        for (auto row: history)
            std::cout << row.value << std::endl;
        request_done();
    };
    pending_request_ = true;
    connection_->codec->address_fetch_history(error_handler(),
        handler, address);
}

int cli::run()
{
    std::cout << "type \"help\" for instructions" << std::endl;
    terminal_.show_prompt();

    while (!done_)
    {
        long delay = -1;
        czmqpp::poller poller;

        poller.add(terminal_.get_socket());

        if (connection_)
        {
            poller.add(connection_->stream->get_socket());

            auto next_wakeup = connection_->codec->wakeup();
            if (next_wakeup.count())
            {
                delay = static_cast<long>(next_wakeup.count());
            }
        }

        czmqpp::socket which = poller.wait(delay);

        if (poller.terminated())
        {
            break;
        }

        if (!poller.expired())
        {
            if (which == terminal_.get_socket())
            {
                command();
            }

            else if (which == connection_->stream->get_socket())
            {
                connection_->stream->signal_response(connection_->codec);
            }
        }
    }

    return 0;
}

void cli::command()
{
    std::stringstream reader(terminal_.get_line());
    std::string command;
    reader >> command;

    if (command == "exit")              cmd_exit();
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

client::obelisk_codec::error_handler cli::error_handler()
{
    return [this](const std::error_code& ec)
    {
        std::cout << "error: " << ec.message() << std::endl;
        request_done();
    };
}

void cli::request_done()
{
    pending_request_ = false;
    terminal_.show_prompt();
}

bool cli::check_connection()
{
    if (!connection_)
    {
        std::cout << "error: no connection" << std::endl;
        return false;
    }
    return true;
}

bool cli::read_string(std::stringstream& args, std::string& out,
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

bool cli::read_address(std::stringstream& args, payment_address& out)
{
    std::string address;
    if (!read_string(args, address, "error: no address given"))
        return false;
    if (!out.set_encoded(address))
    {
        std::cout << "error: invalid address " << address << std::endl;
        return false;
    }
    return true;
}
