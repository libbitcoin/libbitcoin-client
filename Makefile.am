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
    include/bitcoin/client/sleeper.hpp \
    include/bitcoin/client/socket_stream.hpp \
    include/bitcoin/client/version.hpp
    #include/bitcoin/client/random_number_generator.hpp
    #include/bitcoin/client/request_stream.hpp
    #include/bitcoin/client/response_stream.hpp
    #include/bitcoin/client/server_codec.hpp
    #include/bitcoin/client/server_codec_impl.hpp
    #include/bitcoin/client/uniform_uint32_generator.hpp

bitcoin_client_obelisk_includedir = ${includedir}/bitcoin/client/obelisk
bitcoin_client_obelisk_include_HEADERS = \
    include/bitcoin/client/obelisk/obelisk_codec.hpp \
    include/bitcoin/client/obelisk/obelisk_router.hpp \
    include/bitcoin/client/obelisk/obelisk_types.hpp

#
# libraries
#
lib_LTLIBRARIES = src/libbitcoin-client.la

src_libbitcoin_client_la_CPPFLAGS = \
    -I${srcdir}/include \
    ${bitcoin_CFLAGS} \
    ${sodium_CFLAGS} \
    ${czmq___CFLAGS}

src_libbitcoin_client_la_SOURCES = \
    src/obelisk/obelisk_codec.cpp \
    src/obelisk/obelisk_router.cpp \
    src/socket_stream.cpp
    #src/server_codec_impl.cpp
    #src/uniform_uint32_generator.cpp

src_libbitcoin_client_la_LIBADD = \
    ${bitcoin_LIBS} \
    ${sodium_LIBS} \
    ${czmq___LIBS}

#
# examples
#
EXTRA_PROGRAMS = examples/libbitcoin_client examples/get_height

examples_libbitcoin_client_CPPFLAGS = \
    -I${srcdir}/include \
    ${bitcoin_CFLAGS} \
    ${sodium_CFLAGS} \
    ${czmq___CFLAGS}

examples_libbitcoin_client_SOURCES = \
    examples/cli.cpp \
    examples/connection.cpp \
    examples/read_line.cpp \
    examples/main.cpp

examples_libbitcoin_client_LDADD = \
    src/libbitcoin-client.la \
    ${bitcoin_LIBS} \
    ${sodium_LIBS} \
    ${czmq___LIBS}

examples_get_height_SOURCES = \
    examples/get_height.cpp

examples_get_height_CPPFLAGS = \
    -I${srcdir}/include \
    ${bitcoin_CFLAGS} \
    ${sodium_CFLAGS} \
    ${czmq___CFLAGS}

examples_get_height_LDADD = \
    src/libbitcoin-client.la \
    ${bitcoin_LIBS} \
    ${sodium_LIBS} \
    ${czmq___LIBS}

examples: examples/libbitcoin_client examples/get_height

#
# tests
#
if WITH_TESTS

TESTS = test/libbitcoin_client_test

check_PROGRAMS = test/libbitcoin_client_test

test_libbitcoin_client_test_CPPFLAGS = \
    -I${srcdir}/include \
    ${bitcoin_CFLAGS} \
    ${sodium_CFLAGS} \
    ${czmq___CFLAGS}

test_libbitcoin_client_test_SOURCES = \
    test/obelisk_codec.cpp \
    test/main.cpp

test_libbitcoin_client_test_LDADD = \
    src/libbitcoin-client.la \
    -lboost_unit_test_framework \
    ${bitcoin_LIBS} \
    ${sodium_LIBS} \
    ${czmq___LIBS}

endif
