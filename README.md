[![Build Status](https://travis-ci.org/libbitcoin/libbitcoin-client.svg?branch=master)](https://travis-ci.org/libbitcoin/libbitcoin-client)

[![Coverage Status](https://coveralls.io/repos/libbitcoin/libbitcoin-client/badge.svg)](https://coveralls.io/r/libbitcoin/libbitcoin-client)

# Libbitcoin Client

*The Bitcoin Client Protocol Implementation*

Note that you need g++ 4.8 or higher. For this reason Ubuntu 12.04 and older are not supported. Make sure you have installed [libbitcoin](https://github.com/libbitcoin/libbitcoin) beforehand according to its build instructions, as well as libzmq, czmq and czmqpp.

```sh
$ ./autogen.sh
$ ./configure
$ make
$ sudo make install
$ sudo ldconfig
```

libbitcoin-client is now installed in `/usr/local/`.
