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
#include <cstdint>
#include <string>
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/client.hpp>

using namespace bc;
using namespace bc::client;
using namespace bc::wallet;

// Captures an outgoing message so the test can examine it.
class stream_fixture
  : public stream
{
public:
    data_stack out;

    virtual int32_t refresh() override
    {
        return 0;
    }

    virtual bool read(stream& ) override
    {
        return false;
    }

    virtual bool write(const data_stack& data) override
    {
        out = data;
        return true;
    }
};

// Shoves data into a std::string object.
static std::string to_string(data_slice data)
{
    // zmq strings are not null terminated by convention.
    return std::string(data.begin(), data.end()) + "\0";
}

static void remove_optional_delimiter(data_stack& stack)
{
    if (!stack.empty() && stack.front().empty())
        stack.erase(stack.begin());
}

// Arbitrary value for test cases.
static const uint32_t test_height = 0x12345678;

// sha256("_satoshi"):
static const char raw_satoshi[] = "002688cc350a5333a87fa622eacec626c3d1c0ebf9f3793de3885fa254d7e393";

// sha256("_satoshi"), but in hash literal format:
static const char hash_satoshi[] = "93e3d754a25f88e33d79f3f9ebc0d1c326c6ceea22a67fa833530a35cc882600";

// The private key for this address is sha256("_satoshi"):
static const char address_satoshi[] = "1PeChFbhxDD9NLbU21DfD55aQBC4ZTR3tE";

#define PROXY_TEST_SETUP \
    static const uint32_t retries = 0; \
    static const uint32_t timeout_ms = 2000; \
    static const auto on_error = [](const code&) {}; \
    static const auto on_unknown = [](const std::string&) {}; \
    stream_fixture capture; \
    proxy proxy(capture, on_unknown, timeout_ms, retries, bc::settings())

// Allow REQ or unadressed DEALER client.
#define HANDLE_ROUTING_FRAMES(stack) \
    remove_optional_delimiter(stack);

BOOST_AUTO_TEST_SUITE(proxy_tests)

BOOST_AUTO_TEST_CASE(proxy__fetch_history3__test)
{
    PROXY_TEST_SETUP;

    const auto on_reply = [](const history::list&) {};
    proxy.blockchain_fetch_history3(on_error, on_reply, payment_address(address_satoshi), test_height);

    HANDLE_ROUTING_FRAMES(capture.out);
    BOOST_REQUIRE_EQUAL(capture.out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "blockchain.fetch_history3");
    BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), "f85beb6356d0813ddb0dbb14230a249fe931a13578563412");
}

BOOST_AUTO_TEST_CASE(proxy__fetch_transaction__test)
{
    PROXY_TEST_SETUP;

    const auto on_reply = [](const chain::transaction&) {};
    proxy.blockchain_fetch_transaction(on_error, on_reply, hash_literal(hash_satoshi));

    HANDLE_ROUTING_FRAMES(capture.out);
    BOOST_REQUIRE_EQUAL(capture.out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "blockchain.fetch_transaction");
    BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), raw_satoshi);
}

BOOST_AUTO_TEST_CASE(proxy__fetch_transaction2__test)
{
    PROXY_TEST_SETUP;

    const auto on_reply = [](const chain::transaction&) {};
    proxy.blockchain_fetch_transaction2(on_error, on_reply, hash_literal(hash_satoshi));

    HANDLE_ROUTING_FRAMES(capture.out);
    BOOST_REQUIRE_EQUAL(capture.out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "blockchain.fetch_transaction2");
    BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), raw_satoshi);
}

BOOST_AUTO_TEST_CASE(proxy__fetch_last__height_test)
{
    PROXY_TEST_SETUP;

    const auto on_reply = [](size_t) {};
    proxy.blockchain_fetch_last_height(on_error, on_reply);

    HANDLE_ROUTING_FRAMES(capture.out);
    BOOST_REQUIRE_EQUAL(capture.out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "blockchain.fetch_last_height");
    BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), "");
}

BOOST_AUTO_TEST_CASE(proxy__fetch_block_header__height_test)
{
    PROXY_TEST_SETUP;

    const auto on_reply = [](const chain::header&) {};
    proxy.blockchain_fetch_block_header(on_error, on_reply, test_height);

    HANDLE_ROUTING_FRAMES(capture.out);
    BOOST_REQUIRE_EQUAL(capture.out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "blockchain.fetch_block_header");
    BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), "78563412");
}

BOOST_AUTO_TEST_CASE(proxy__fetch_block_header__hash_test)
{
    PROXY_TEST_SETUP;

    auto on_reply = [](const chain::header&) {};
    proxy.blockchain_fetch_block_header(on_error, on_reply, hash_literal(hash_satoshi));

    HANDLE_ROUTING_FRAMES(capture.out);
    BOOST_REQUIRE_EQUAL(capture.out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "blockchain.fetch_block_header");
    BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), raw_satoshi);
}

BOOST_AUTO_TEST_CASE(proxy__fetch_transaction__index_test)
{
    PROXY_TEST_SETUP;

    const auto on_reply = [](size_t, size_t) {};
    proxy.blockchain_fetch_transaction_index(on_error, on_reply, hash_literal(hash_satoshi));

    HANDLE_ROUTING_FRAMES(capture.out);
    BOOST_REQUIRE_EQUAL(capture.out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "blockchain.fetch_transaction_index");
    BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), raw_satoshi);
}

BOOST_AUTO_TEST_CASE(proxy__fetch_stealth2__test)
{
    PROXY_TEST_SETUP;

    const auto on_reply = [](const stealth::list&) {};
    const std::vector<uint8_t> raw_prefix{ 0xff, 0xff, 0x00, 0x00 };
    const binary prefix(16, raw_prefix);
    proxy.blockchain_fetch_stealth2(on_error, on_reply, prefix, test_height);

    HANDLE_ROUTING_FRAMES(capture.out);
    BOOST_REQUIRE_EQUAL(capture.out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "blockchain.fetch_stealth2");
    BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), "10ffff78563412");
}

BOOST_AUTO_TEST_CASE(proxy__pool_fetch_transaction__test)
{
    PROXY_TEST_SETUP;

    const auto on_reply = [](const chain::transaction&) {};
    proxy.transaction_pool_fetch_transaction(on_error, on_reply, hash_literal(hash_satoshi));

    HANDLE_ROUTING_FRAMES(capture.out);
    BOOST_REQUIRE_EQUAL(capture.out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "transaction_pool.fetch_transaction");
    BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), raw_satoshi);
}

BOOST_AUTO_TEST_CASE(proxy__pool_fetch_transaction2__test)
{
    PROXY_TEST_SETUP;

    const auto on_reply = [](const chain::transaction&) {};
    proxy.transaction_pool_fetch_transaction2(on_error, on_reply, hash_literal(hash_satoshi));

    HANDLE_ROUTING_FRAMES(capture.out);
    BOOST_REQUIRE_EQUAL(capture.out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "transaction_pool.fetch_transaction2");
    BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), raw_satoshi);
}

BOOST_AUTO_TEST_CASE(proxy__subscribe__test)
{
    PROXY_TEST_SETUP;

    const auto on_reply = [](const code&) {};
    proxy.subscribe_address(on_error, on_reply, payment_address(address_satoshi));

    HANDLE_ROUTING_FRAMES(capture.out);
    BOOST_REQUIRE_EQUAL(capture.out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "subscribe.address");
    BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), "f85beb6356d0813ddb0dbb14230a249fe931a135");
}

BOOST_AUTO_TEST_SUITE_END()
