AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = libbitcoin_client.la

AM_CPPFLAGS = \
    -I$(srcdir)/../include \
    ${sodium_CFLAGS} \
    ${bitcoin_CFLAGS} \
    ${protobuf_CFLAGS} \
    ${bitcoin_protocol_CFLAGS} \
    ${zmq_CFLAGS} \
    ${czmq_CFLAGS} \
    ${czmqpp_CFLAGS}

    
libbitcoin_client_la_SOURCES = \
    obelisk_codec.cpp \
    server_codec_impl.cpp \
    socket_stream.cpp \
    uniform_uint32_generator.cpp

libbitcoin_client_la_LIBADD = \
    ${sodium_LIBS} \
    ${bitcoin_LIBS} \
    ${protobuf_LIBS} \
    ${bitcoin_protocol_LIBS} \
    ${zmq_LIBS} \
    ${czmq_LIBS} \
    ${czmqpp_LIBS}

