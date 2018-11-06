/**
 * Copyright (c) 2011-2018 libbitcoin developers (see AUTHORS)
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
#include <cstdint>
#include <string>
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <bitcoin/client.hpp>

using namespace bc;
using namespace bc::client;
using namespace bc::wallet;

static const std::string mainnet_test_url = "tcp://mainnet.libbitcoin.net:9091";

// Arbitrary values for test cases (from mainnet block 100).
static const uint32_t test_height = 100;
static const std::string test_address = "13A1W4jLPP75pzvn2qJ5KyyqG3qPSpb9jM";
static const char test_hash[] = "2d05f0c9c3e1c226e63b5fac240137687544cf631cd616fd34fd188fc9020866";

#define CLIENT_TEST_SETUP \
    static const uint32_t retries = 0; \
    obelisk_client client(retries); \
    client.connect(config::endpoint(mainnet_test_url))

BOOST_AUTO_TEST_SUITE(client_tests)

/* BOOST_AUTO_TEST_CASE(client__fetch_history4__test) */
/* { */
/*     CLIENT_TEST_SETUP; */

/*     const uint32_t from_height = 0; */
/*     const auto address = payment_address(test_address); */
/*     const std::string encoded_expected = "f85beb6356d0813ddb0dbb14230a249fe931a13578563412"; */
/*     std::string encoded_received; */

/*     const auto on_done = [&encoded_received](const history::list& rows) */
/*     { */
/*     }; */

/*     client.blockchain_fetch_history4(on_error, on_done, address, from_height); */
/*     client.wait(); */

/*     BOOST_REQUIRE_EQUAL(encoded_received, encoded_expected); */
/* } */

BOOST_AUTO_TEST_CASE(client__fetch_transaction__test)
{
    CLIENT_TEST_SETUP;

    const std::string expected_hash = "660802c98f18fd34fd16d61c63cf447568370124ac5f3be626c2e1c3c9f0052d";

    std::string received_hash;
    const auto on_done = [&received_hash](const code& ec, const chain::transaction& tx)
    {
        if (ec == error::success)
            received_hash = encode_base16(tx.hash());
    };

    client.blockchain_fetch_transaction(on_done, hash_literal(test_hash));
    client.wait();

    BOOST_REQUIRE_EQUAL(received_hash, expected_hash);
}

BOOST_AUTO_TEST_CASE(client__fetch_transaction2__test)
{
    CLIENT_TEST_SETUP;

    const std::string expected_hash = "660802c98f18fd34fd16d61c63cf447568370124ac5f3be626c2e1c3c9f0052d";

    std::string received_hash;
    const auto on_done = [&received_hash](const code& ec, const chain::transaction& tx)
    {
        if (ec == error::success)
            received_hash = encode_base16(tx.hash());
    };

    client.blockchain_fetch_transaction2(on_done, hash_literal(test_hash));
    client.wait();

    BOOST_REQUIRE_EQUAL(received_hash, expected_hash);
}

/* BOOST_AUTO_TEST_CASE(client__fetch_unspent_outputs__test) */
/* { */
/*     CLIENT_TEST_SETUP; */

/*     const auto satoshis = 10000; */
/*     const uint64_t expected_value = 100; // FIXME */
/*     uint64_t received_value = 0; */

/*     const auto on_done = [&received_value](const chain::points_value& value) */
/*     { */
/*         received_value = value.value(); */
/*     }; */

/*     client.blockchain_fetch_unspent_outputs(on_error, on_done, { test_address }, */
/*         satoshis, wallet::select_outputs::algorithm::individual); */
/*     client.wait(); */

/*     BOOST_REQUIRE_EQUAL(received_value, expected_value); */
/* } */

BOOST_AUTO_TEST_CASE(client__fetch_last_height__test)
{
    CLIENT_TEST_SETUP;

    size_t received_height = 0;
    const auto on_done = [&received_height](const code& ec, size_t height)
    {
        if (ec == error::success)
            received_height = height;
    };

    client.blockchain_fetch_last_height(on_done);
    client.wait();

    BOOST_REQUIRE_EQUAL(received_height > 0, true);
}

BOOST_AUTO_TEST_CASE(client__fetch_last_height__multi_handler_test)
{
    CLIENT_TEST_SETUP;

    size_t received_height1 = 0;
    size_t received_height2 = 0;
    size_t received_height3 = 0;

    const auto handler1 = [&received_height1](const code& ec, size_t height)
    {
        if (ec == error::success)
            received_height1 = height;
    };

    const auto handler2 = [&received_height2](const code& ec, size_t height)
    {
        if (ec == error::success)
            received_height2 = height;
    };

    const auto handler3 = [&received_height3](const code& ec, size_t height)
    {
        if (ec == error::success)
            received_height3 = height;
    };

    client.blockchain_fetch_last_height(handler1);
    client.blockchain_fetch_last_height(handler2);
    client.blockchain_fetch_last_height(handler3);
    client.wait();

    BOOST_REQUIRE_EQUAL(received_height1 > 0, true);
    BOOST_REQUIRE_EQUAL(received_height2 > 0, true);
    BOOST_REQUIRE_EQUAL(received_height3 > 0, true);
}

BOOST_AUTO_TEST_CASE(client__fetch_block_header__height_test)
{
    CLIENT_TEST_SETUP;

    std::string expected_hash = "9a22db7fd25e719abf9e8ccf869fbbc1e22fa71822a37efae054c17b00000000";
    std::string received_hash;
    const auto on_done = [&received_hash](const code& ec, const chain::header& header)
    {
        if (ec == error::success)
            received_hash = encode_base16(header.hash());
    };

    client.blockchain_fetch_block_header(on_done, test_height);
    client.wait();

    BOOST_REQUIRE_EQUAL(received_hash, expected_hash);
}

BOOST_AUTO_TEST_CASE(client__fetch_block_header__hash_test)
{
    CLIENT_TEST_SETUP;

    const std::string expected_hash = "9a22db7fd25e719abf9e8ccf869fbbc1e22fa71822a37efae054c17b00000000";
    const std::string expected_previous_hash = "95194b8567fe2e8bbda931afd01a7acd399b9325cb54683e64129bcd00000000";

    std::string received_hash;
    std::string received_previous_hash;

    const auto on_done = [&received_hash, &received_previous_hash](const code& ec, const chain::header& header)
    {
        if (ec != error::success)
            return;

        received_hash = encode_base16(header.hash());
        received_previous_hash = encode_base16(header.previous_block_hash());
    };

    client.blockchain_fetch_block_header(on_done, test_height);
    client.wait();

    BOOST_REQUIRE_EQUAL(received_hash, expected_hash);
    BOOST_REQUIRE_EQUAL(received_previous_hash, expected_previous_hash);
}

BOOST_AUTO_TEST_CASE(client__fetch_transaction__index_test)
{
    CLIENT_TEST_SETUP;

    const size_t expected_block = 100;
    const size_t expected_index = 0;

    size_t received_block = 0;
    size_t received_index = 0;

    const auto on_done = [&received_block, &received_index](const code& ec, size_t block, size_t index)
    {
        if (ec != error::success)
            return;

        received_block = block;
        received_index = index;
    };

    client.blockchain_fetch_transaction_index(on_done, hash_literal(test_hash));
    client.wait();

    BOOST_REQUIRE_EQUAL(received_block, expected_block);
    BOOST_REQUIRE_EQUAL(received_index, expected_index);
}

/* BOOST_AUTO_TEST_CASE(client__fetch_stealth2__test) */
/* { */
/*     CLIENT_TEST_SETUP; */

/*     const auto on_reply = [](const stealth::list&) {}; */
/*     const std::vector<uint8_t> raw_prefix{ 0xff, 0xff, 0x00, 0x00 }; */
/*     const binary prefix(16, raw_prefix); */
/*     client.blockchain_fetch_stealth2(on_error, on_reply, prefix, test_height); */

/*     HANDLE_ROUTING_FRAMES(capture.out); */
/*     BOOST_REQUIRE_EQUAL(capture.out.size(), 3u); */
/*     BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "blockchain.fetch_stealth2"); */
/*     BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), "10ffff78563412"); */
/* } */

BOOST_AUTO_TEST_CASE(client__stealth_public__test)
{
    CLIENT_TEST_SETUP;

    const std::string expected_hash = "660802c98f18fd34fd16d61c63cf447568370124ac5f3be626c2e1c3c9f0052d";

    std::string received_hash;
    const auto on_done = [&received_hash](const code& ec, const chain::transaction& tx)
    {
        if (ec == error::success)
            received_hash = encode_base16(tx.hash());
    };

    client.transaction_pool_fetch_transaction(on_done, hash_literal(test_hash));
    client.wait();

    BOOST_REQUIRE_EQUAL(received_hash, expected_hash);
}

BOOST_AUTO_TEST_CASE(client__pool_fetch_transaction__test)
{
    CLIENT_TEST_SETUP;

    const std::string expected_hash = "660802c98f18fd34fd16d61c63cf447568370124ac5f3be626c2e1c3c9f0052d";

    std::string received_hash;
    const auto on_done = [&received_hash](const code& ec, const chain::transaction& tx)
    {
        if (ec == error::success)
            received_hash = encode_base16(tx.hash());
    };

    client.transaction_pool_fetch_transaction(on_done, hash_literal(test_hash));
    client.wait();

    BOOST_REQUIRE_EQUAL(received_hash, expected_hash);
}

BOOST_AUTO_TEST_CASE(client__pool_fetch_transaction2__test)
{
    CLIENT_TEST_SETUP;

    const std::string expected_hash = "660802c98f18fd34fd16d61c63cf447568370124ac5f3be626c2e1c3c9f0052d";

    std::string received_hash;
    const auto on_done = [&received_hash](const code& ec, const chain::transaction& tx)
    {
        if (ec == error::success)
            received_hash = encode_base16(tx.hash());
    };

    client.transaction_pool_fetch_transaction2(on_done, hash_literal(test_hash));
    client.wait();

    BOOST_REQUIRE_EQUAL(received_hash, expected_hash);
}

/* BOOST_AUTO_TEST_CASE(client__subscribe__test) */
/* { */
/*     CLIENT_TEST_SETUP; */

/*     const auto on_reply = [](const code&) {}; */
/*     client.subscribe_address(on_error, on_reply, payment_address(address_satoshi)); */

/*     HANDLE_ROUTING_FRAMES(capture.out); */
/*     BOOST_REQUIRE_EQUAL(capture.out.size(), 3u); */
/*     BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "subscribe.address"); */
/*     BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), "f85beb6356d0813ddb0dbb14230a249fe931a135"); */
/* } */

BOOST_AUTO_TEST_SUITE_END()
