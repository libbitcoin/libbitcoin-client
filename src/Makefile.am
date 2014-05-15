AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = libbitcoin-client.la
AM_CPPFLAGS = -I$(srcdir)/../include $(libbitcoin_CFLAGS)
libbitcoin_client_la_SOURCES = \
    client.cpp

libbitcoin_client_la_LIBADD = $(libbitcoin_LIBS)

