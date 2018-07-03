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


# Distribute, make and install products.
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
src_libbitcoin_client_la_CPPFLAGS = -I${srcdir}/include ${bitcoin_BUILD_CPPFLAGS} ${bitcoin_protocol_BUILD_CPPFLAGS}
src_libbitcoin_client_la_LIBADD = ${bitcoin_LIBS} ${bitcoin_protocol_LIBS}
src_libbitcoin_client_la_SOURCES = \
    src/dealer.cpp \
    src/obelisk_client.cpp \
    src/proxy.cpp \
    src/socket_stream.cpp

# local: test/libbitcoin-client-test
#------------------------------------------------------------------------------
if WITH_TESTS

TESTS = libbitcoin-client-test_runner.sh

check_PROGRAMS = test/libbitcoin-client-test
test_libbitcoin_client_test_CPPFLAGS = -I${srcdir}/include ${bitcoin_BUILD_CPPFLAGS} ${bitcoin_protocol_BUILD_CPPFLAGS}
test_libbitcoin_client_test_LDADD = src/libbitcoin-client.la ${boost_unit_test_framework_LIBS} ${bitcoin_LIBS} ${bitcoin_protocol_LIBS}
test_libbitcoin_client_test_SOURCES = \
    test/main.cpp \
    test/proxy.cpp

endif WITH_TESTS

# local: examples/console/console
#------------------------------------------------------------------------------
if WITH_EXAMPLES

noinst_PROGRAMS = examples/console/console
examples_console_console_CPPFLAGS = -I${srcdir}/include ${bitcoin_BUILD_CPPFLAGS} ${bitcoin_protocol_BUILD_CPPFLAGS}
examples_console_console_LDADD = src/libbitcoin-client.la ${bitcoin_LIBS} ${bitcoin_protocol_LIBS}
examples_console_console_SOURCES = \
    examples/console/client.cpp \
    examples/console/client.hpp \
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
examples_get_height_get_height_CPPFLAGS = -I${srcdir}/include ${bitcoin_BUILD_CPPFLAGS} ${bitcoin_protocol_BUILD_CPPFLAGS}
examples_get_height_get_height_LDADD = src/libbitcoin-client.la ${bitcoin_LIBS} ${bitcoin_protocol_LIBS}
examples_get_height_get_height_SOURCES = \
    examples/get_height/main.cpp

endif WITH_EXAMPLES

# files => ${includedir}/bitcoin
#------------------------------------------------------------------------------
include_bitcoindir = ${includedir}/bitcoin
include_bitcoin_HEADERS = \
    include/bitcoin/client.hpp

include_bitcoin_clientdir = ${includedir}/bitcoin/client
include_bitcoin_client_HEADERS = \
    include/bitcoin/client/dealer.hpp \
    include/bitcoin/client/define.hpp \
    include/bitcoin/client/history.hpp \
    include/bitcoin/client/obelisk_client.hpp \
    include/bitcoin/client/proxy.hpp \
    include/bitcoin/client/socket_stream.hpp \
    include/bitcoin/client/stealth.hpp \
    include/bitcoin/client/stream.hpp \
    include/bitcoin/client/version.hpp


# Custom make targets.
#==============================================================================
# make target: examples
#------------------------------------------------------------------------------
target_examples = \
    examples/console/console \
    examples/get_height/get_height

examples: ${target_examples}

