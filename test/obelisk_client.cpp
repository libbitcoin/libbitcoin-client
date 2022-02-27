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
#include <cstdint>
#include <string>
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <bitcoin/client.hpp>
#include <bitcoin/protocol.hpp>

using namespace bc::client;
using namespace bc::protocol;
using namespace bc::system;
using namespace bc::system::wallet;

// FIXME: This points to a temporary v4 testnet instance.
static const std::string testnet_url = "tcp://testnet2.libbitcoin.net:29091";

// Arbitrary values for test cases (from testnet block 800,001).
static const uint32_t test_height = 800001;
static const std::string test_address = "2NGDnSYWMPY1mZCre69wWWgqV1T2wryAXNV";
static const char test_key[] = "2ef44127d8b0e66eb991f79a8da10e901fc07a82d69a9cfc1ea6e53ae1c66465";
static const char test_utxo_key[] = "4dda1bb623465ef9c36390975126c1cbff2f5693cc6ad6d3de34c240c092e2e5";
static const char test_tx_hash[] = "6b0b5509edd6f14c85245f4192097632a7f785d1b2edba0566a2014a29277d73";
static const char test_block_hash[] = "00000000002889eccd1262e2b7fe893b9839574d9db57755a1c717f88dae73d5";

#define CLIENT_TEST_SETUP \
    static const uint32_t retries = 0; \
    obelisk_client client(retries); \
    client.connect(config::endpoint(testnet_url))

BOOST_AUTO_TEST_SUITE(stub)

BOOST_AUTO_TEST_CASE(client__dummy_test__ok)
{
    BOOST_REQUIRE_EQUAL(true, true);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(network)

BOOST_AUTO_TEST_CASE(client__fetch_history4__test)
{
    CLIENT_TEST_SETUP;

    const uint64_t expected_height = 923346;
    const std::string expected_hash = "c331a7e31978f1b7ba4a60c6ebfce6eb713ab1542ddf2fd67bbf0824f9d1a353";
    uint64_t received_height = 0;
    std::string received_hash;

    const auto on_done = [&received_hash, &received_height](const code& ec, const history::list& rows)
    {
        if (ec == error::success)
        {
            const auto& row = rows.front();
            received_hash = encode_hash(row.output.hash());
            received_height = row.output_height;
        }
    };

    client.blockchain_fetch_history4(on_done, hash_literal(test_utxo_key), 0);
    client.wait();

    BOOST_REQUIRE_EQUAL(received_hash, expected_hash);
    BOOST_REQUIRE_EQUAL(received_height, expected_height);
}

BOOST_AUTO_TEST_CASE(client__subscribe_key__test_ok_and_timeout)
{
    CLIENT_TEST_SETUP;

    size_t times_called = 0;

    // This should be called exactly twice.  Once for subscription
    // success, and then again for subscription timeout.
    auto on_done = [&times_called](const code& ec, uint16_t, size_t,
        const hash_digest&)
    {
        if (++times_called == 1)
            BOOST_REQUIRE_EQUAL(ec, error::success);
        else
            BOOST_REQUIRE_EQUAL(ec, error::channel_timeout);
    };

    const auto id = client.subscribe_key(on_done, hash_literal(test_key));
    client.monitor(0);

    BOOST_REQUIRE_EQUAL(id, 1);
}

BOOST_AUTO_TEST_CASE(client__unsubscribe_key__test_ok)
{
    CLIENT_TEST_SETUP;

    bool subscribed = false;
    uint32_t id = obelisk_client::null_subscription;

    auto subscribe_handler = [&client, &subscribed, &id]()
    {
        static const auto ten_minutes_in_milliseconds = 600 * 1000;

        auto on_done = [&subscribed](const code& ec, uint16_t, size_t,
            const hash_digest&)
        {
            if (ec == error::success)
                subscribed = true;
        };

        id = client.subscribe_key(on_done, hash_literal(test_key));
        BOOST_REQUIRE_EQUAL(id, 1);
        client.monitor(ten_minutes_in_milliseconds);
    };

    // Waits until subscribe handler is subscribed and then unsubscribes.
    auto unsubscribe_handler = [&client, &subscribed, &id]()
    {
        zmq::poller poller;
        while(!subscribed)
            poller.wait(100);

        bool unsubscribe_complete = false;

        auto on_done = [&unsubscribe_complete](const code& ec)
        {
            BOOST_REQUIRE_EQUAL(ec, error::success);
            unsubscribe_complete = true;
        };

        BOOST_REQUIRE(id != obelisk_client::null_subscription);
        BOOST_REQUIRE(client.unsubscribe_key(on_done, id) == true);

        poller.wait(2000);
        BOOST_REQUIRE(unsubscribe_complete == true);
    };

    std::thread unsubscriber(unsubscribe_handler);
    std::thread subscriber(subscribe_handler);

    subscriber.join();
    unsubscriber.join();
}

BOOST_AUTO_TEST_CASE(client__fetch_transaction__test)
{
    CLIENT_TEST_SETUP;

    const std::string expected_hash = std::string(test_tx_hash);

    std::string received_hash;
    const auto on_done = [&received_hash](const code& ec, const chain::transaction& tx)
    {
        BOOST_REQUIRE_EQUAL(ec, error::success);
        if (ec == error::success)
            received_hash = encode_hash(tx.hash());
    };

    client.blockchain_fetch_transaction(on_done, hash_literal(test_tx_hash));
    client.wait();

    BOOST_REQUIRE_EQUAL(received_hash, expected_hash);
}

BOOST_AUTO_TEST_CASE(client__fetch_transaction2__test)
{
    CLIENT_TEST_SETUP;

    const std::string expected_hash = std::string(test_tx_hash);

    std::string received_hash;
    const auto on_done = [&received_hash](const code& ec, const chain::transaction& tx)
    {
        BOOST_REQUIRE_EQUAL(ec, error::success);
        if (ec == error::success)
            received_hash = encode_hash(tx.hash());
    };

    client.blockchain_fetch_transaction2(on_done, hash_literal(test_tx_hash));
    client.wait();

    BOOST_REQUIRE_EQUAL(received_hash, expected_hash);
}

BOOST_AUTO_TEST_CASE(client__fetch_unspent_outputs__test)
{
    CLIENT_TEST_SETUP;

    const auto satoshis = 100000;
    const std::string expected_hash = "c331a7e31978f1b7ba4a60c6ebfce6eb713ab1542ddf2fd67bbf0824f9d1a353";
    std::string received_hash;

    const auto on_done = [&received_hash](const code& ec, const chain::points_value& value)
    {
        if (ec == error::success)
            received_hash = encode_hash(value.points.front().hash());
    };

    client.blockchain_fetch_unspent_outputs(on_done, hash_literal(test_utxo_key),
        satoshis, chain::points_value::selection::individual);
    client.wait();

    BOOST_REQUIRE_EQUAL(received_hash, expected_hash);
}

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

    const std::string expected_hash = std::string(test_block_hash);

    std::string received_hash;
    const auto on_done = [&received_hash](const code& ec, const chain::header& header)
    {
        if (ec == error::success)
            received_hash = encode_hash(header.hash());
    };

    client.blockchain_fetch_block_header(on_done, test_height);
    client.wait();

    BOOST_REQUIRE_EQUAL(received_hash, expected_hash);
}

BOOST_AUTO_TEST_CASE(client__fetch_block_header__hash_test)
{
    CLIENT_TEST_SETUP;

    const std::string expected_hash = std::string(test_block_hash);
    const std::string expected_previous_hash = "0000000000209b091d6519187be7c2ee205293f25f9f503f90027e25abf8b503";

    std::string received_hash;
    std::string received_previous_hash;

    const auto on_done = [&received_hash, &received_previous_hash](const code& ec, const chain::header& header)
    {
        if (ec != error::success)
            return;

        received_hash = encode_hash(header.hash());
        received_previous_hash = encode_hash(header.previous_block_hash());
    };

    client.blockchain_fetch_block_header(on_done, test_height);
    client.wait();

    BOOST_REQUIRE_EQUAL(received_hash, expected_hash);
    BOOST_REQUIRE_EQUAL(received_previous_hash, expected_previous_hash);
}

BOOST_AUTO_TEST_CASE(client__fetch_transaction__index_test)
{
    CLIENT_TEST_SETUP;

    const size_t expected_block = test_height;
    const size_t expected_index = 1;

    size_t received_block = 9999;
    size_t received_index = 9999;

    const auto on_done = [&received_block, &received_index](const code& ec,
        size_t block, size_t index)
    {
        if (ec != error::success)
            return;

        received_block = block;
        received_index = index;
    };

    client.blockchain_fetch_transaction_index(on_done, hash_literal(test_tx_hash));
    client.wait();

    BOOST_REQUIRE_EQUAL(received_block, expected_block);
    BOOST_REQUIRE_EQUAL(received_index, expected_index);
}

BOOST_AUTO_TEST_CASE(client__pool_fetch_transaction__test)
{
    CLIENT_TEST_SETUP;

    const std::string expected_hash = std::string(test_tx_hash);

    std::string received_hash;
    const auto on_done = [&received_hash](const code& ec, const chain::transaction& tx)
    {
        if (ec == error::success)
            received_hash = encode_hash(tx.hash());
    };

    client.transaction_pool_fetch_transaction(on_done, hash_literal(test_tx_hash));
    client.wait();

    BOOST_REQUIRE_EQUAL(received_hash, expected_hash);
}

BOOST_AUTO_TEST_CASE(client__pool_fetch_transaction2__test)
{
    CLIENT_TEST_SETUP;

    const std::string expected_hash = std::string(test_tx_hash);

    std::string received_hash;
    const auto on_done = [&received_hash](const code& ec, const chain::transaction& tx)
    {
        if (ec == error::success)
            received_hash = encode_hash(tx.hash());
    };

    client.transaction_pool_fetch_transaction2(on_done, hash_literal(test_tx_hash));
    client.wait();

    BOOST_REQUIRE_EQUAL(received_hash, expected_hash);
}

BOOST_AUTO_TEST_SUITE_END()
