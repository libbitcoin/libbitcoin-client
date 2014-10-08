AUTOMAKE_OPTIONS = subdir-objects

TESTS = libbitcoin-client-test

check_PROGRAMS = libbitcoin-client-test

libbitcoin-client-test_CPPFLAGS = \
    -I$(srcdir)/../include \
    ${bitcoin_CFLAGS} \
    ${sodium_CFLAGS} \
    ${zmq_CFLAGS} \
    ${czmq_CFLAGS} \
    ${czmqpp_CFLAGS} \
     -DBOOST_TEST_DYN_LINK

    
libbitcoin-client-test_SOURCES = \
    client.cpp \
    main.cpp

libbitcoin-client-test_LDADD = \
    -L../src -lbitcoin-client \
    -lboost_unit_test_framework \
    ${bitcoin_LIBS} \
    ${sodium_LIBS} \
    ${zmq_LIBS} \
    ${czmq_LIBS} \
    ${czmqpp_LIBS}

