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
#ifndef BITCOIN_CLIENT_CLI_HPP
#define BITCOIN_CLIENT_CLI_HPP

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <czmq++/czmqpp.hpp>
#include <bitcoin/client.hpp>
#include "connection.hpp"
#include "read_line.hpp"

/**
 * Command-line interface for talking to the obelisk server.
 */
class cli
{
public:
    /**
     * Constructor.
     */
    cli();
    
    /**
     * The main loop for the example application. This loop can be woken up
     * by either events from the network or by input from the terminal.
     */
    int run();

private:
    
    /**
     * The commands.
     */
    void cmd_exit();
    void cmd_help();
    void cmd_connect(std::stringstream& args);
    void cmd_disconnect(std::stringstream& args);
    void cmd_height(std::stringstream& args);
    void cmd_history(std::stringstream& args);
    
    /**
     * Reads a command from the terminal thread, and processes it appropriately.
     */
    void command();

    /**
     * Obtains a simple error-handling functor object.
     */
    bc::client::obelisk_codec::error_handler error_handler();

    /**
     * Call this to display a new prompt once a request has finished.
     */
    void request_done();

    /**
     * Verifies that a connection exists, and prints an error message otherwise.
     */
    bool check_connection();

    /**
     * Parses a string argument out of the command line, or prints an error 
     * message if there is none.
     */
    bool read_string(
        std::stringstream& args,
        std::string& out,
        const std::string& error_message);

    /**
     * Reads a bitcoin address from the command-line, or prints an error if
     * the address is missing or invalid.
     */
    bool read_address(std::stringstream& args,
        bc::wallet::payment_address& out);

    /**
     * Private members.
     */
    bool done_;
    bool pending_request_;
    czmqpp::context context_;
    read_line terminal_;
    std::shared_ptr<connection> connection_;
};

#endif
