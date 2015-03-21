###############################################################################
#  Copyright (c) 2014-2015 libbitcoin-client developers (see COPYING).
#
#         GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY
#
###############################################################################

# Automake settings.
#==============================================================================
# Look for macros in the m4 subdirectory.
#------------------------------------------------------------------------------
ACLOCAL_AMFLAGS = -I m4


# Distribution, build and install products.
#==============================================================================
# files => ${pkgconfigdir}
#------------------------------------------------------------------------------
pkgconfig_DATA = \
    libbitcoin-client.pc

# files => ${docdir}
#------------------------------------------------------------------------------
doc_DATA = \
    AUTHORS \
    COPYING \
    ChangeLog \
    INSTALL \
    NEWS \
    README

# src/libbitcoin-client.la => ${libdir}
#------------------------------------------------------------------------------
lib_LTLIBRARIES = src/libbitcoin-client.la
src_libbitcoin_client_la_CPPFLAGS = -I${srcdir}/include ${bitcoin_CPPFLAGS} ${sodium_CPPFLAGS} ${czmq___CPPFLAGS}
src_libbitcoin_client_la_LIBADD = ${bitcoin_LIBS} ${sodium_LIBS} ${czmq___LIBS}
src_libbitcoin_client_la_SOURCES = \
    src/socket_stream.cpp \
    src/obelisk/obelisk_codec.cpp \
    src/obelisk/obelisk_router.cpp

# local: test/libbitcoin_client_test
#------------------------------------------------------------------------------
if WITH_TESTS

TESTS = libbitcoin_client_test_runner.sh

check_PROGRAMS = test/libbitcoin_client_test
test_libbitcoin_client_test_CPPFLAGS = -I${srcdir}/include ${bitcoin_CPPFLAGS} ${sodium_CPPFLAGS} ${czmq___CPPFLAGS}
test_libbitcoin_client_test_LDADD = src/libbitcoin-client.la ${boost_unit_test_framework_LIBS} ${bitcoin_LIBS} ${sodium_LIBS} ${czmq___LIBS}
test_libbitcoin_client_test_SOURCES = \
    test/main.cpp \
    test/obelisk_codec.cpp

endif WITH_TESTS

# local: examples/console/libbitcoin_client
#------------------------------------------------------------------------------
if WITH_EXAMPLES

noinst_PROGRAMS = examples/console/libbitcoin_client
examples_console_libbitcoin_client_CPPFLAGS = -I${srcdir}/include ${bitcoin_CPPFLAGS} ${sodium_CPPFLAGS} ${czmq___CPPFLAGS}
examples_console_libbitcoin_client_LDADD = src/libbitcoin-client.la ${bitcoin_LIBS} ${sodium_LIBS} ${czmq___LIBS}
examples_console_libbitcoin_client_SOURCES = \
    examples/console/cli.cpp \
    examples/console/cli.hpp \
    examples/console/connection.cpp \
    examples/console/connection.hpp \
    examples/console/main.cpp \
    examples/console/read_line.cpp \
    examples/console/read_line.hpp

endif WITH_EXAMPLES

# local: examples/get_height/get_height
#------------------------------------------------------------------------------
if WITH_EXAMPLES

noinst_PROGRAMS += examples/get_height/get_height
examples_get_height_get_height_CPPFLAGS = -I${srcdir}/include ${bitcoin_CPPFLAGS} ${sodium_CPPFLAGS} ${czmq___CPPFLAGS}
examples_get_height_get_height_LDADD = src/libbitcoin-client.la ${bitcoin_LIBS} ${sodium_LIBS} ${czmq___LIBS}
examples_get_height_get_height_SOURCES = \
    examples/get_height/get_height.cpp

endif WITH_EXAMPLES

# files => ${includedir}/bitcoin
#------------------------------------------------------------------------------
include_bitcoindir = ${includedir}/bitcoin
include_bitcoin_HEADERS = \
    include/bitcoin/client.hpp

include_bitcoin_clientdir = ${includedir}/bitcoin/client
include_bitcoin_client_HEADERS = \
    include/bitcoin/client/define.hpp \
    include/bitcoin/client/message_stream.hpp \
    include/bitcoin/client/sleeper.hpp \
    include/bitcoin/client/socket_stream.hpp \
    include/bitcoin/client/version.hpp

include_bitcoin_client_obeliskdir = ${includedir}/bitcoin/client/obelisk
include_bitcoin_client_obelisk_HEADERS = \
    include/bitcoin/client/obelisk/obelisk_codec.hpp \
    include/bitcoin/client/obelisk/obelisk_router.hpp \
    include/bitcoin/client/obelisk/obelisk_types.hpp


# Custom make targets.
#==============================================================================
# make target: examples
#------------------------------------------------------------------------------
target_examples = \
    examples/console/libbitcoin_client \
    examples/get_height/get_height

examples: ${target_examples}

