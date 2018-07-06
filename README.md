# libae, the simple network library extracted from `Redis`

## Source files

- `ae*.[ch]`, extracted from `Redis` 4.0.1 release without any changes.
- `config.h`, compilation config macro for libae.
- `zmalloc.h`, definitions for memory management functions: `zmalloc`, `zrealloc`, `zcalloc` and `zfree`.
- `rot13d.c`, an example of ROT13 server, modified from [ROT13 server in libevent book](http://www.wangafu.net/~nickm/libevent-book/01_intro.html)。

## Library Documentation

- [Redis Event Library – Redis](https://redis.io/topics/internals-rediseventlib)