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
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <bitcoin/client.hpp>

using namespace bc;
using namespace bc::client;

/**
 * Captures an outgoing message so the test can examine it.
 */
class message_capture
  : public message_stream
{
public:
    data_stack out;

    virtual void write(const data_stack& data) override
    {
        out = data;
    }
};

/**
 * Shoves data into a std::string object.
 */
std::string to_string(data_slice data)
{
    return std::string(data.begin(), data.end());
}

// sha256("Satoshi"):
static const char rawSatoshi[] =
    "002688cc350a5333a87fa622eacec626c3d1c0ebf9f3793de3885fa254d7e393";

// sha256("Satoshi"), but in hash literal format:
static const char hashSatoshi[] =
    "93e3d754a25f88e33d79f3f9ebc0d1c326c6ceea22a67fa833530a35cc882600";

// The private key for this address is sha256("Satoshi"):
static const char addressSatoshi[] = "1PeChFbhxDD9NLbU21DfD55aQBC4ZTR3tE";

BOOST_AUTO_TEST_SUITE(obelisk_codec_tests)

BOOST_AUTO_TEST_CASE(obelisk_codec_fetch_history_test)
{
    std::shared_ptr<message_capture> capture(new message_capture());
    obelisk_codec codec(std::static_pointer_cast<message_stream>(capture));

    auto on_error = [](const std::error_code& code) {};
    auto on_reply = [](const history_list&) {};
    codec.fetch_history(on_error, on_reply,
        payment_address(addressSatoshi), 0x12345678);

    BOOST_REQUIRE_EQUAL(capture->out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture->out[0]),
        "blockchain.fetch_history");
    BOOST_REQUIRE_EQUAL(encode_hex(capture->out[2]),
        "0035a131e99f240a2314bb0ddb3d81d05663eb5bf878563412");
}

BOOST_AUTO_TEST_CASE(obelisk_codec_fetch_transaction_test)
{
    std::shared_ptr<message_capture> capture(new message_capture());
    obelisk_codec codec(std::static_pointer_cast<message_stream>(capture));

    auto on_error = [](const std::error_code& code) {};
    auto on_reply = [](const transaction_type&) {};
    codec.fetch_transaction(on_error, on_reply,
        hash_literal(hashSatoshi));

    BOOST_REQUIRE_EQUAL(capture->out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture->out[0]),
        "blockchain.fetch_transaction");
    BOOST_REQUIRE_EQUAL(encode_hex(capture->out[2]),
        rawSatoshi);
}

BOOST_AUTO_TEST_CASE(obelisk_codec_fetch_last_height_test)
{
    std::shared_ptr<message_capture> capture(new message_capture());
    obelisk_codec codec(std::static_pointer_cast<message_stream>(capture));

    auto on_error = [](const std::error_code& code) {};
    auto on_reply = [](size_t) {};
    codec.fetch_last_height(on_error, on_reply);

    BOOST_REQUIRE_EQUAL(capture->out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture->out[0]),
        "blockchain.fetch_last_height");
    BOOST_REQUIRE_EQUAL(encode_hex(capture->out[2]),
        "");
}

BOOST_AUTO_TEST_CASE(obelisk_codec_fetch_block_header_height_test)
{
    std::shared_ptr<message_capture> capture(new message_capture());
    obelisk_codec codec(std::static_pointer_cast<message_stream>(capture));

    auto on_error = [](const std::error_code& code) {};
    auto on_reply = [](const block_header_type&) {};
    codec.fetch_block_header(on_error, on_reply,
        0x12345678);

    BOOST_REQUIRE_EQUAL(capture->out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture->out[0]),
        "blockchain.fetch_block_header");
    BOOST_REQUIRE_EQUAL(encode_hex(capture->out[2]),
        "78563412");
}

BOOST_AUTO_TEST_CASE(obelisk_codec_fetch_block_header_hash_test)
{
    std::shared_ptr<message_capture> capture(new message_capture());
    obelisk_codec codec(std::static_pointer_cast<message_stream>(capture));

    auto on_error = [](const std::error_code& code) {};
    auto on_reply = [](const block_header_type&) {};
    codec.fetch_block_header(on_error, on_reply,
        hash_literal(hashSatoshi));

    BOOST_REQUIRE_EQUAL(capture->out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture->out[0]),
        "blockchain.fetch_block_header");
    BOOST_REQUIRE_EQUAL(encode_hex(capture->out[2]),
        rawSatoshi);
}

BOOST_AUTO_TEST_CASE(obelisk_codec_fetch_transaction_index_test)
{
    std::shared_ptr<message_capture> capture(new message_capture());
    obelisk_codec codec(std::static_pointer_cast<message_stream>(capture));

    auto on_error = [](const std::error_code& code) {};
    auto on_reply = [](size_t block_height, size_t index) {};
    codec.fetch_transaction_index(on_error, on_reply,
        hash_literal(hashSatoshi));

    BOOST_REQUIRE_EQUAL(capture->out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture->out[0]),
        "blockchain.fetch_transaction_index");
    BOOST_REQUIRE_EQUAL(encode_hex(capture->out[2]),
        rawSatoshi);
}

BOOST_AUTO_TEST_CASE(obelisk_codec_fetch_stealth_test)
{
    std::shared_ptr<message_capture> capture(new message_capture());
    obelisk_codec codec(std::static_pointer_cast<message_stream>(capture));

    auto on_error = [](const std::error_code& code) {};
    auto on_reply = [](const stealth_list&) {};
    stealth_prefix prefix{16, 0x0000ffff};
    codec.fetch_stealth(on_error, on_reply, prefix,
        0x12345678);

    BOOST_REQUIRE_EQUAL(capture->out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture->out[0]),
        "blockchain.fetch_stealth");
    BOOST_REQUIRE_EQUAL(encode_hex(capture->out[2]),
        "10ffff000078563412");
}

BOOST_AUTO_TEST_CASE(obelisk_codec_fetch_unconfirmed_transaction_test)
{
    std::shared_ptr<message_capture> capture(new message_capture());
    obelisk_codec codec(std::static_pointer_cast<message_stream>(capture));

    auto on_error = [](const std::error_code& code) {};
    auto on_reply = [](const transaction_type&) {};
    codec.fetch_unconfirmed_transaction(on_error, on_reply,
        hash_literal(hashSatoshi));

    BOOST_REQUIRE_EQUAL(capture->out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture->out[0]),
        "transaction_pool.fetch_transaction");
    BOOST_REQUIRE_EQUAL(encode_hex(capture->out[2]),
        rawSatoshi);
}

BOOST_AUTO_TEST_CASE(obelisk_codec_address_fetch_history_test)
{
    std::shared_ptr<message_capture> capture(new message_capture());
    obelisk_codec codec(std::static_pointer_cast<message_stream>(capture));

    auto on_error = [](const std::error_code& code) {};
    auto on_reply = [](const history_list&) {};
    codec.address_fetch_history(on_error, on_reply,
        payment_address(addressSatoshi), 0x12345678);

    BOOST_REQUIRE_EQUAL(capture->out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture->out[0]),
        "address.fetch_history");
    BOOST_REQUIRE_EQUAL(encode_hex(capture->out[2]),
        "0035a131e99f240a2314bb0ddb3d81d05663eb5bf878563412");
}

BOOST_AUTO_TEST_CASE(obelisk_codec_subscribe_test)
{
    std::shared_ptr<message_capture> capture(new message_capture());
    obelisk_codec codec(std::static_pointer_cast<message_stream>(capture));

    auto on_error = [](const std::error_code& code) {};
    auto on_reply = []() {};
    codec.subscribe(on_error, on_reply,
        payment_address(addressSatoshi));

    BOOST_REQUIRE_EQUAL(capture->out.size(), 3u);
    BOOST_REQUIRE_EQUAL(to_string(capture->out[0]),
        "address.subscribe");
    BOOST_REQUIRE_EQUAL(encode_hex(capture->out[2]),
        "0035a131e99f240a2314bb0ddb3d81d05663eb5bf8");
}

BOOST_AUTO_TEST_SUITE_END()
