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
#include "client.hpp"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <bitcoin/client.hpp>
#include "read_line.hpp"

using namespace bc::chain;
using namespace bc::client;
using namespace bc::protocol;
using namespace bc::wallet;

client::client()
  : done_(false)
{
}

void client::cmd_exit(std::stringstream&)
{
    done_ = true;
}

void client::cmd_help(std::stringstream&)
{
    std::cout << "Commands:" << std::endl;
    std::cout << "  exit              - Leave the program" << std::endl;
    std::cout << "  help              - Display this menu" << std::endl;
    std::cout << "  connect <server>  - Connect to a server" << std::endl;
    std::cout << "  disconnect        - Disconnect from the server" << std::endl;
    std::cout << "  height            - Fetch last block height" << std::endl;
    std::cout << "  header <hash>     - Fetch a block's header" << std::endl;
    std::cout << "  history <address> - Fetch an address' history" << std::endl;
}

void client::cmd_connect(std::stringstream& args)
{
    static constexpr size_t retries = 3;

    std::string server;
    if (!read_string(args, server, "error: no server given"))
        return;

    std::cout << "Connecting to " << server << std::endl;

    connection_ = std::make_shared<bc::client::obelisk_client>(retries);
    if (!connection_ || !connection_->connect(bc::config::endpoint(server)))
        std::cerr << "Failed to connect to " << server << std::endl;
}

void client::cmd_disconnect(std::stringstream&)
{
    connection_.reset();
    std::cout << "Disconnected from server" << std::endl;
}

void client::cmd_height(std::stringstream&)
{
    if (!connection_)
    {
        std::cerr << "Connect to a server first." << std::endl;
        return;
    }

    auto handler = [this](const bc::code& ec, size_t height)
    {
        if (ec)
            std::cerr << "Failed to retrieve height: " << ec.message()
                      << std::endl;
        else
            std::cout << "Height: " << height << std::endl;
    };

    connection_->blockchain_fetch_last_height(handler);
    connection_->wait();
}

void client::cmd_history(std::stringstream& args)
{
    if (!connection_)
    {
        std::cerr << "Connect to a server first." << std::endl;
        return;
    }

    payment_address address;
    if (!read_address(args, address))
        return;

    auto handler = [this](const bc::code& ec, const history::list& history)
    {
        if (ec != bc::error::success)
            std::cerr << "Failed to retrieve history: " << ec.message()
                      << std::endl;
        else
            for (const auto& row: history)
                std::cout << "History value: " << row.value << std::endl;
    };

    connection_->blockchain_fetch_history4(handler, address.hash());
    connection_->wait();
}

void client::cmd_header(std::stringstream& args)
{
    if (!connection_)
    {
        std::cerr << "Connect to a server first." << std::endl;
        return;
    }

    bc::hash_digest hash{};
    if (!read_hash(args, hash))
        return;

    auto handler = [this](const bc::code& ec,
        const bc::chain::header& header)
    {
        if (ec)
        {
            std::cerr << "Failed to retrieve block header: " << ec.message()
                      << std::endl;
            return;
        }

        std::cout << "Header          : " << bc::encode_base16(header.hash())
            << std::endl;
        std::cout << "Bits            : " << header.bits() << std::endl;
        std::cout << "Merkle Tree Hash: "
            << bc::encode_base16(header.merkle()) << std::endl;
        std::cout << "Nonce           : " << header.nonce() << std::endl;
        std::cout << "Previous Hash   : "
            << bc::encode_base16(header.previous_block_hash()) << std::endl;
        std::cout << "Timestamp       : " << header.timestamp() << std::endl;
        std::cout << "Version         : " << header.version() << std::endl;
    };

    connection_->blockchain_fetch_block_header(handler, hash);
    connection_->wait();
}

int client::run()
{
    static const size_t delay_milliseconds = 100;

    std::cout << "Type \"help\" for supported instructions" << std::endl;
    terminal_.show_prompt();

    const auto terminal_socket_id = terminal_.socket().id();
    zmq::poller poller;
    poller.add(terminal_.socket());

    while (!poller.terminated() && !done_)
    {
        if (poller.wait(delay_milliseconds).contains(terminal_socket_id))
            command();
    }

    return 0;
}

void client::command()
{
    typedef std::function<void(std::stringstream&)> handler;
    typedef std::unordered_map<std::string, handler> handler_map;

    static const handler_map handlers =
    {
        { "exit", [this](std::stringstream& args) { cmd_exit(args); } },
        { "help", [this](std::stringstream& args) { cmd_help(args); } },
        { "connect", [this](std::stringstream& args) { cmd_connect(args); } },
        { "disconnect", [this](std::stringstream& args) { cmd_disconnect(args); } },
        { "height", [this](std::stringstream& args) { cmd_height(args); } },
        { "history", [this](std::stringstream& args) { cmd_history(args); } },
        { "header", [this](std::stringstream& args) { cmd_header(args); } }
    };

    std::stringstream reader(terminal_.get_line());
    std::string command;
    reader >> command;

    auto command_handler = handlers.find(command);
    if (command_handler != handlers.end())
        command_handler->second(reader);
    else
        std::cout << "Unknown command " << command << std::endl;

    if (!done_)
        terminal_.show_prompt();
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
        std::cout << "Error: invalid address " << address << std::endl;
        return false;
    }

    out = payment;
    return true;
}

bool client::read_hash(std::stringstream& args, bc::hash_digest& out)
{
    std::string hash_string;
    if (!read_string(args, hash_string, "error: no hash given"))
        return false;

    bc::hash_digest hash{};
    if (!bc::decode_hash(hash, hash_string))
    {
        std::cout << "Error: invalid hash " << hash_string << std::endl;
        return false;
    }

    out = hash;
    return true;
}
