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
#ifndef LIBBITCOIN_CLIENT_HPP
#define LIBBITCOIN_CLIENT_HPP

/**
 * @mainpage libbitcoin-client API dox
 *
 * @section intro_sec Introduction
 *
 * This library will contain the logic and communications code needed to talk
 * to a libbitcoin-server.
 */

// Convenience header that includes everything
// Not to be used internally. For API users.
#include <client/define.hpp>
#include <client/message_stream.hpp>
#include <client/obelisk_codec.hpp>
#include <client/sleeper.hpp>
#include <client/zeromq_socket.hpp>

// cppzmq (zmq.hpp) is not presently usable with versions of libzmq since 
// 2014.01.28. Expecting a patch: https://github.com/zeromq/cppzmq/issues/40
// Until then this will fail with recent libzmq versions.
// #include <client/zmq.hpp>

#endif
