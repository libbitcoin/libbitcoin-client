/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
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
#include <cstdint>
#include <string>
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <bitcoin/client.hpp>

using namespace bc;
using namespace bc::client;

// Captures an outgoing message so the test can examine it.
class message_capture
  : public message_stream
{
public:
    data_stack out;

    virtual bool read(message_stream& stream) override
    {
        // Not implemented.
        return false;
    }

    virtual void write(const data_stack& data) override
    {
        out = data;
    }
};

// Shoves data into a std::string object.
std::string to_string(data_slice data)
{
    return std::string(data.begin(), data.end());
}

// Arbitrary value for test cases.
static const uint32_t test_height = 0x12345678;

// sha256("_satoshi"):
static const char raw_satoshi[] = "002688cc350a5333a87fa622eacec626c3d1c0ebf9f3793de3885fa254d7e393";

// sha256("_satoshi"), but in hash literal format:
static const char hash_satoshi[] = "93e3d754a25f88e33d79f3f9ebc0d1c326c6ceea22a67fa833530a35cc882600";

// The private key for this address is sha256("_satoshi"):
static const char address_satoshi[] = "1PeChFbhxDD9NLbU21DfD55aQBC4ZTR3tE";

#define OBELISK_CODEC_TEST_SETUP \
    static const uint32_t retries = 0; \
    static const uint32_t timeout_ms = 2000; \
    static const auto on_error = [](const code&) {}; \
    static const auto on_unknown = [](const std::string&) {}; \
    message_capture capture; \
    obelisk_codec codec(capture, on_unknown, timeout_ms, retries)

BOOST_AUTO_TEST_SUITE(obelisk_codec_tests)

BOOST_AUTO_TEST_CASE(obelisk_codec__fetch_history__test)
{
    OBELISK_CODEC_TEST_SETUP;

    const auto on_reply = [](const history_list&) {};
    codec.fetch_history(on_error, on_reply,
        wallet::payment_address(address_satoshi), test_height);

    BOOST_REQUIRE_EQUAL(capture.out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "blockchain.fetch_history");
    BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), "0035a131e99f240a2314bb0ddb3d81d05663eb5bf878563412");
}

BOOST_AUTO_TEST_CASE(obelisk_codec__fetch_transaction__test)
{
    OBELISK_CODEC_TEST_SETUP;

    const auto on_reply = [](const chain::transaction&) {};
    codec.fetch_transaction(on_error, on_reply, hash_literal(hash_satoshi));

    BOOST_REQUIRE_EQUAL(capture.out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "blockchain.fetch_transaction");
    BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), raw_satoshi);
}

BOOST_AUTO_TEST_CASE(obelisk_codec__fetch_last__height_test)
{
    OBELISK_CODEC_TEST_SETUP;

    const auto on_reply = [](size_t) {};
    codec.fetch_last_height(on_error, on_reply);

    BOOST_REQUIRE_EQUAL(capture.out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "blockchain.fetch_last_height");
    BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), "");
}

BOOST_AUTO_TEST_CASE(obelisk_codec__fetch_block_header__height_test)
{
    OBELISK_CODEC_TEST_SETUP;

    const auto on_reply = [](const chain::header&) {};
    codec.fetch_block_header(on_error, on_reply, test_height);

    BOOST_REQUIRE_EQUAL(capture.out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "blockchain.fetch_block_header");
    BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), "78563412");
}

BOOST_AUTO_TEST_CASE(obelisk_codec__fetch_block_header__hash_test)
{
    OBELISK_CODEC_TEST_SETUP;

    auto on_reply = [](const chain::header&) {};
    codec.fetch_block_header(on_error, on_reply, hash_literal(hash_satoshi));

    BOOST_REQUIRE_EQUAL(capture.out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "blockchain.fetch_block_header");
    BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), raw_satoshi);
}

BOOST_AUTO_TEST_CASE(obelisk_codec__fetch_transaction__index_test)
{
    OBELISK_CODEC_TEST_SETUP;

    const auto on_reply = [](size_t, size_t) {};
    codec.fetch_transaction_index(on_error, on_reply, hash_literal(hash_satoshi));

    BOOST_REQUIRE_EQUAL(capture.out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "blockchain.fetch_transaction_index");
    BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), raw_satoshi);
}

BOOST_AUTO_TEST_CASE(obelisk_codec__fetch_stealth__test)
{
    OBELISK_CODEC_TEST_SETUP;

    const auto on_reply = [](const stealth_list&) {};
    const std::vector<uint8_t> raw_prefix{ 0xff, 0xff, 0x00, 0x00 };
    const binary prefix(16, raw_prefix);
    codec.fetch_stealth(on_error, on_reply, prefix, test_height);

    BOOST_REQUIRE_EQUAL(capture.out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "blockchain.fetch_stealth");
    BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), "10ffff78563412");
}

BOOST_AUTO_TEST_CASE(obelisk_codec__fetch_unconfirmed_transaction__test)
{
    OBELISK_CODEC_TEST_SETUP;

    const auto on_reply = [](const chain::transaction&) {};
    codec.fetch_unconfirmed_transaction(on_error, on_reply, hash_literal(hash_satoshi));

    BOOST_REQUIRE_EQUAL(capture.out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "transaction_pool.fetch_transaction");
    BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), raw_satoshi);
}

BOOST_AUTO_TEST_CASE(obelisk_codec__address_fetch_history__test)
{
    OBELISK_CODEC_TEST_SETUP;

    const auto on_reply = [](const history_list&) {};
    codec.address_fetch_history(on_error, on_reply, wallet::payment_address(address_satoshi), test_height);

    BOOST_REQUIRE_EQUAL(capture.out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "address.fetch_history");
    BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), "0035a131e99f240a2314bb0ddb3d81d05663eb5bf878563412");
}

BOOST_AUTO_TEST_CASE(obelisk_codec__subscribe__test)
{
    OBELISK_CODEC_TEST_SETUP;

    const auto on_reply = []() {};
    codec.subscribe(on_error, on_reply, wallet::payment_address(address_satoshi));

    BOOST_REQUIRE_EQUAL(capture.out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture.out[0]), "address.subscribe");
    BOOST_REQUIRE_EQUAL(encode_base16(capture.out[2]), "00a0f85beb6356d0813ddb0dbb14230a249fe931a135");
}

BOOST_AUTO_TEST_SUITE_END()
