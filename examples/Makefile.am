AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = client

AM_CPPFLAGS = \
    -I$(srcdir)/../include \
    ${bitcoin_CFLAGS} \
    $(protobuf_CFLAGS) \
    ${bitcoin_protocol_CFLAGS} \
    ${sodium_CFLAGS} \
    ${zmq_CFLAGS} \
    ${czmq_CFLAGS} \
    ${czmqpp_CFLAGS}

    
client_SOURCES = \
    cli.cpp \
    connection.cpp \
    read_line.cpp \
    main.cpp

LDADD = \
    -L../src/ -lbitcoin_client \
    ${bitcoin_LIBS} \
    ${protobuf_LIBS} \
    ${bitcoin_protocol_LIBS} \
    ${sodium_LIBS} \
    ${zmq_LIBS} \
    ${czmq_LIBS} \
    ${czmqpp_LIBS}

