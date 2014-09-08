AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = libbitcoin-client.la

AM_CPPFLAGS = \
    -I$(srcdir)/../include \
    ${bitcoin_CFLAGS} \
    ${sodium_CFLAGS}
    
libbitcoin_client_la_SOURCES = \
    obelisk_codec.cpp \
    zeromq_socket.cpp

libbitcoin_client_la_LIBADD = \
    ${bitcoin_LIBS} \
    ${sodium_LIBS}
