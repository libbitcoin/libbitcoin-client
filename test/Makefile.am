AUTOMAKE_OPTIONS = subdir-objects

noinst_PROGRAMS = libbitcoin_client_test

AM_CPPFLAGS = \
    -I$(srcdir)/../include \
    ${bitcoin_CFLAGS} \
    ${sodium_CFLAGS} \
    ${zmq_CFLAGS} \
    ${czmq_CFLAGS} \
    ${czmqpp_CFLAGS} \
     -DBOOST_TEST_DYN_LINK

    
libbitcoin_client_test_SOURCES = \
    client.cpp \
    main.cpp

libbitcoin_client_test_LDADD = \
    -L../src -lbitcoin_client \
    -lboost_unit_test_framework \
    ${bitcoin_LIBS} \
    ${sodium_LIBS} \
    ${zmq_LIBS} \
    ${czmq_LIBS} \
    ${czmqpp_LIBS}

