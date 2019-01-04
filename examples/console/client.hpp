/**
 * Copyright (c) 2011-2019 libbitcoin developers (see AUTHORS)
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
#ifndef BITCOIN_CLIENT_CLIENT_HPP
#define BITCOIN_CLIENT_CLIENT_HPP

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <bitcoin/client.hpp>
#include "read_line.hpp"

/**
 * Command-line interface for talking to the obelisk server.
 */
class client
{
public:
    /**
     * Constructor.
     */
    client();

    /**
     * The main loop for the example application. This loop can be woken up
     * by either events from the network or by input from the terminal.
     */
    int run();

private:

    /**
     * The commands.
     */
    void cmd_exit(std::stringstream& args);
    void cmd_help(std::stringstream& args);
    void cmd_connect(std::stringstream& args);
    void cmd_disconnect(std::stringstream& args);
    void cmd_height(std::stringstream& args);
    void cmd_history(std::stringstream& args);
    void cmd_header(std::stringstream& args);

    /**
     * Reads a command from the terminal thread, and processes it appropriately.
     */
    void command();

    /**
     * Parses a string argument out of the command line, or prints an error
     * message if there is none.
     */
    bool read_string(std::stringstream& args, std::string& out,
        const std::string& error_message);

    /**
     * Reads a bitcoin address from the command-line, or prints an error if
     * the address is missing or invalid.
     */
    bool read_address(std::stringstream& args,
        bc::system::wallet::payment_address& out);

    /**
     * Reads a 64 byte hex encoded hash from the command-line, or
     * prints an error if the hash is missing or invalid.
     */
    bool read_hash(std::stringstream& args, bc::system::hash_digest& out);

    /**
     * Private members.
     */
    bool done_;
    read_line terminal_;
    std::shared_ptr<bc::client::obelisk_client> connection_;
};

#endif
