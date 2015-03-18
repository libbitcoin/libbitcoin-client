###############################################################################
#  Copyright (c) 2014-2015 libbitcoin-client developers (see COPYING).
#
#         GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY
#
###############################################################################

# Standard declarations.
#==============================================================================
# look for macros in the m4 subdirectory
ACLOCAL_AMFLAGS = -I m4


# Automake products.
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
    src/server_codec_impl.cpp \
    src/socket_stream.cpp \
    src/uniform_uint32_generator.cpp \
    src/obelisk/obelisk_codec.cpp \
    src/obelisk/obelisk_router.cpp

# local: test/libbitcoin_client_test
#------------------------------------------------------------------------------
if WITH_TESTS

# Execute 'test/libbitcoin_client_test' as a test.
TESTS = test/libbitcoin_client_test

check_PROGRAMS = test/libbitcoin_client_test
test_libbitcoin_client_test_CPPFLAGS = -I${srcdir}/include ${bitcoin_CPPFLAGS}
test_libbitcoin_client_test_LDADD = src/libbitcoin-client.la ${bitcoin_LIBS} ${boost_unit_test_framework_LIBS}
test_libbitcoin_client_test_SOURCES = \
    test/main.cpp \
    test/obelisk_codec.cpp

endif WITH_TESTS

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

