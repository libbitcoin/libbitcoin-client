#
# documentation
#
doc_DATA = README INSTALL COPYING

#
# pkg-config
#
pkgconfig_DATA = libbitcoin-client.pc

# ?
ACLOCAL_AMFLAGS = -I m4

#
# headers
#
bitcoin_includedir = $(includedir)/bitcoin
bitcoin_include_HEADERS = \
    include/bitcoin/client.hpp

bitcoin_client_includedir = $(includedir)/bitcoin/client
bitcoin_client_include_HEADERS = \
    include/bitcoin/client/define.hpp \
    include/bitcoin/client/message_stream.hpp \
    include/bitcoin/client/obelisk_codec.hpp \
    include/bitcoin/client/random_number_generator.hpp \
    include/bitcoin/client/response_stream.hpp \
    include/bitcoin/client/request_stream.hpp \
    include/bitcoin/client/server_codec.hpp \
    include/bitcoin/client/server_codec_impl.hpp \
    include/bitcoin/client/sleeper.hpp \
    include/bitcoin/client/socket_stream.hpp \
    include/bitcoin/client/uniform_uint32_generator.hpp

#
# binaries
#
lib_LTLIBRARIES = libbitcoin-client.la

libbitcoin_client_la_CPPFLAGS = \
    -I$(srcdir)/include \
    ${sodium_CFLAGS} \
    ${bitcoin_protocol_CFLAGS}

libbitcoin_client_la_SOURCES = \
    src/obelisk_codec.cpp \
    src/server_codec_impl.cpp \
    src/socket_stream.cpp \
    src/uniform_uint32_generator.cpp

libbitcoin_client_la_LIBADD = \
    ${sodium_LIBS} \
    ${bitcoin_protocol_LIBS}

#
# test
#
TESTS = libbitcoin-client-test

check_PROGRAMS = libbitcoin-client-test

libbitcoin_client_test_CPPFLAGS = \
    -I$(srcdir)/include \
    ${sodium_CFLAGS} \
    ${bitcoin_protocol_CFLAGS} \
     -DBOOST_TEST_DYN_LINK

libbitcoin_client_test_SOURCES = \
    test/client.cpp \
    test/main.cpp

libbitcoin_client_test_LDADD = \
    libbitcoin-client.la \
    -lboost_unit_test_framework \
    ${sodium_LIBS} \
    ${bitcoin_protocol_LIBS}

#
# example(s)
#
EXTRA_PROGRAMS = client

client_CPPFLAGS = \
    -I$(srcdir)/include \
    ${sodium_CFLAGS} \
    ${bitcoin_protocol_CFLAGS}

client_SOURCES = \
    examples/cli.cpp \
    examples/connection.cpp \
    examples/read_line.cpp \
    examples/main.cpp

client_LDADD = \
    libbitcoin-client.la \
    ${sodium_LIBS} \
    ${bitcoin_protocol_LIBS}

