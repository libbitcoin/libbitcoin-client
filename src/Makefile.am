AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = libbitcoin-client.la
AM_CPPFLAGS = -I$(srcdir)/../include $(libbitcoin_CFLAGS)
libbitcoin_client_la_SOURCES = \
    obelisk_codec.cpp \
    zmq_socket.cpp

libbitcoin_client_la_LIBADD = $(libbitcoin_LIBS)

