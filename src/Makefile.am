AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = libbitcoin_client.la

AM_CPPFLAGS = \
    -I$(srcdir)/../include \
    ${bitcoin_protocol_CFLAGS} \
    ${bitcoin_CFLAGS} \
    ${sodium_CFLAGS} \
    ${zmq_CFLAGS} \
    ${czmq_CFLAGS} \
    ${czmqpp_CFLAGS}

    
libbitcoin_client_la_SOURCES = \
    server_codec_impl.cpp \
    socket_stream.cpp \
    uniform_uint32_generator.cpp

libbitcoin_client_la_LIBADD = \
    ${bitcoin_protocol_LIBS} \
    ${bitcoin_LIBS} \
    ${sodium_LIBS} \
    ${zmq_LIBS} \
    ${czmq_LIBS} \
    ${czmqpp_LIBS}

