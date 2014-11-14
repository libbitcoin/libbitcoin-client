#
# documentation
#
doc_DATA = README INSTALL COPYING

#
# pkg-config
#
pkgconfig_DATA = libbitcoin-client.pc

# look for macros in the m4 subdirectory
ACLOCAL_AMFLAGS = -I m4

#
# headers
#
bitcoin_includedir = ${includedir}/bitcoin
bitcoin_include_HEADERS = \
    include/bitcoin/client.hpp

bitcoin_client_includedir = ${includedir}/bitcoin/client
bitcoin_client_include_HEADERS = \
    include/bitcoin/client/define.hpp \
    include/bitcoin/client/message_stream.hpp \
    include/bitcoin/client/obelisk_codec.hpp \
    include/bitcoin/client/obelisk_v1_types.hpp \
    include/bitcoin/client/random_number_generator.hpp \
    include/bitcoin/client/response_stream.hpp \
    include/bitcoin/client/request_stream.hpp \
    include/bitcoin/client/server_codec.hpp \
    include/bitcoin/client/server_codec_impl.hpp \
    include/bitcoin/client/sleeper.hpp \
    include/bitcoin/client/socket_stream.hpp \
    include/bitcoin/client/uniform_uint32_generator.hpp

#
# libraries
#
lib_LTLIBRARIES = src/libbitcoin-client.la

src_libbitcoin_client_la_CPPFLAGS = \
    -I${srcdir}/include \
    ${sodium_CFLAGS} \
    ${bitcoin_protocol_CFLAGS}

src_libbitcoin_client_la_SOURCES = \
    src/obelisk_codec.cpp \
    src/server_codec_impl.cpp \
    src/socket_stream.cpp \
    src/uniform_uint32_generator.cpp

src_libbitcoin_client_la_LIBADD = \
    ${sodium_LIBS} \
    ${bitcoin_protocol_LIBS}

#
# examples
#
EXTRA_PROGRAMS = examples/libbitcoin_client

examples_libbitcoin_client_CPPFLAGS = \
    -I${srcdir}/include \
    ${sodium_CFLAGS} \
    ${bitcoin_protocol_CFLAGS}

examples_libbitcoin_client_SOURCES = \
    examples/cli.cpp \
    examples/connection.cpp \
    examples/read_line.cpp \
    examples/main.cpp

examples_libbitcoin_client_LDADD = \
    src/libbitcoin-client.la \
    ${sodium_LIBS} \
    ${bitcoin_protocol_LIBS}

examples: examples/libbitcoin_client

#
# tests
#
if WITH_TESTS

TESTS = test/libbitcoin_client_test

check_PROGRAMS = test/libbitcoin_client_test

test_libbitcoin_client_test_CPPFLAGS = \
    -I${srcdir}/include \
    ${sodium_CFLAGS} \
    ${bitcoin_protocol_CFLAGS}
    
if SET_BOOST_TEST_DYN_LINK
test_libbitcoin_client_test_CPPFLAGS += \
    -DBOOST_TEST_DYN_LINK
endif

test_libbitcoin_client_test_SOURCES = \
    test/client.cpp \
    test/main.cpp

test_libbitcoin_client_test_LDADD = \
    src/libbitcoin-client.la \
    -lboost_unit_test_framework \
    ${sodium_LIBS} \
    ${bitcoin_protocol_LIBS}
    
endif
