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
#ifndef BITCOIN_CLIENT_READ_LINE_HPP
#define BITCOIN_CLIENT_READ_LINE_HPP

#include <string>
#include <thread>
#include <czmq++/czmqpp.hpp>
#include <bitcoin/client.hpp>

/*
 * Reads lines from the terminal in a separate thread.
 *
 * A networking thread cannot use the C++ standard library to read from the
 * terminal. Once the thread calls std::cin.getline or similar, it becomes
 * stuck until the user types something, so the thread cannot handle network
 * events at the same time. Therefore, the network stuff and the terminal
 * stuff need to run in separate threads.
 *
 * The simplest solution is to create a thread that simply reads from the
 * terminal and transmits the results over a zeromq inproc socket. The main
 * thread sends an empty REQ message when it wants to read from the terminal,
 * and the reader thread sends back a REP message with whatever the user
 * typed. If the main thread sends a non-empty REQ message, the thread quits.
 *
 * To use this class, first call `show_prompt`. This call will display a
 * command prompt and begin reading input in the background. Then, use
 * `pollitem` with `zmq_poll` to determine when the line is available. Once
 * the line is available, use `get_line` to retrieve it.
 *
 * If you attempt to destroy this class while reading a line, the destructor
 * will block until the user finishes their entry.
 */
class read_line : public libbitcoin::client::zmq_pollable
{
public:
    ~read_line();
    read_line(czmqpp::context& context);

    /**
     * Displays a command prompt and begins reading a line in the background.
     */
    void show_prompt();

    /**
     * Retrieves the line requested by `show_prompt`. This method will
     * return a blank string if no line is available yet.
     */
    std::string get_line();

    virtual void add(czmqpp::poller& poller);

    virtual bool matches(czmqpp::poller& poller, czmqpp::socket& which);

private:
    void run(czmqpp::context* context);

    czmqpp::socket socket_;
    std::thread* thread_;
};

#endif
