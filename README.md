# libae，redis的网络库

## 文件说明

- `ae*.[ch]`源自redis 4.0.1发行源码，只字未动。
- `config.h`ae依赖，简单地选择使用epoll作为后端。
- `zmalloc.h`ae依赖的动态内存分配和释放函数，直接使用C库的。
- `rot13d.c`demo程序，模仿[libevent book ROT13服务器](http://www.wangafu.net/~nickm/libevent-book/01_intro.html)。

## 接口文档

- [Redis Event Library – Redis](https://redis.io/topics/internals-rediseventlib)