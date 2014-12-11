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
#ifndef LIBBITCOIN_CLIENT_CLIENT_HPP
#define LIBBITCOIN_CLIENT_CLIENT_HPP

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
#include <bitcoin/protocol.hpp>
#include <bitcoin/client/define.hpp>
#include <bitcoin/client/message_stream.hpp>
#include <bitcoin/client/obelisk/obelisk_codec.hpp>
#include <bitcoin/client/obelisk/obelisk_router.hpp>
#include <bitcoin/client/obelisk/obelisk_types.hpp>
#include <bitcoin/client/random_number_generator.hpp>
#include <bitcoin/client/request_stream.hpp>
#include <bitcoin/client/response_stream.hpp>
#include <bitcoin/client/server_codec.hpp>
#include <bitcoin/client/server_codec_impl.hpp>
#include <bitcoin/client/sleeper.hpp>
#include <bitcoin/client/socket_stream.hpp>
#include <bitcoin/client/uniform_uint32_generator.hpp>

#endif
