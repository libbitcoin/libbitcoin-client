[![Build Status](https://travis-ci.org/libbitcoin/libbitcoin-client.svg?branch=master)](https://travis-ci.org/libbitcoin/libbitcoin-client)

[![Coverage Status](https://coveralls.io/repos/libbitcoin/libbitcoin-client/badge.svg)](https://coveralls.io/r/libbitcoin/libbitcoin-client)

# Libbitcoin Client

*Bitcoin Client Query Library*

Make sure you have installed [libbitcoin-protocol](https://github.com/libbitcoin/libbitcoin-protocol) beforehand according to its build instructions.

```sh
$ ./autogen.sh
$ ./configure
$ make
$ sudo make install
$ sudo ldconfig
```

If you installed [libbitcoin](https://github.com/libbitcoin/libbitcoin) to `/usr/local` (the default) and linking fails, retry by setting `PKG_CONFIG_PATH` beforehand:
```
$ export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
$ ./configure && make
```

libbitcoin-client is now installed in `/usr/local/`.
