/**
 * in chapter 0 of libevent-book.
 *
 * see http://www.wangafu.net/~nickm/libevent-book/TOC.html
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>     // for sockaddr_in{}
#include <fcntl.h>

#include "ae.h"

#define MAX_LINE 16384

struct fd_state
{
    int fd;
    char buffer[MAX_LINE];

    // 命名以此buffer作为宾语
    size_t write_to;      // where of the buffer to write next time，用于往buffer写
    size_t read_from;     // buffer[read_from] ... buffer[read_upto - 1] is the data to be sent out.用于从buffer读
    size_t read_upto;
};

int
set_nonblocking(int fd)
{
    int flags;
    if ((flags = fcntl(fd, F_GETFL, NULL)) < 0) {
        return -1;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return -1;
    }

    return 0;
}

char
rot13_char(char c)
{
    /**
     * we don't want to use isalpha here;
     * setting the locale would change which characters are considered alphabetical.
     */
    if((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M')) {
        return c + 13;
    }
    else if((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z')) {
        return c - 13;
    }
    else {
        return c;
    }
}

struct fd_state *
alloc_fd_state(int fd)
{
    struct fd_state *state;

    state = malloc(sizeof(struct fd_state));
    if(!state) {
        return NULL;
    }

    state->fd = fd;
    state->write_to = 0;
    state->read_from = 0;
    state->read_upto = 0;

    return state;
}

void
free_fd_state(struct fd_state *state)
{
    free(state);
}

void
connWriteProc(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask)
{
    struct fd_state *state;

    if (!(mask & AE_WRITABLE)) {
        fprintf(stderr,"not writable\n");
    }

    state = (struct fd_state *)clientData;
    while(state->read_from < state->read_upto) {
        ssize_t result = send(fd, state->buffer + state->read_from, state->read_upto - state->read_from, 0);
        if(result < 0) {
            if(errno == EAGAIN)  {    // XXX use evutil macro
                return;
            }
            free_fd_state(state);
            return;
        }
        assert(result != 0);

        state->read_from += result;
    }

    if(state->read_from == state->write_to) {
        state->read_from = 0;
        state->read_upto = 0;
        state->write_to = 0;
    }

    aeDeleteFileEvent(eventLoop, fd, AE_WRITABLE); // 写事件由读事件引发的，有数据待发时才要注册此事件
}

void
connReadProc(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask)
{
    struct fd_state *state;
    char buf[1024];
    int i;
    ssize_t result;

    state = (struct fd_state *)clientData;
    while(1) {
        result = recv(fd, buf, sizeof(buf), 0);
        if(result <= 0) {
            break;
        }

        for(i = 0; i < result; ++i) {
            if(state->write_to < sizeof(state->buffer)) {
                state->buffer[state->write_to++] = rot13_char(buf[i]);
            }

            if(buf[i] == '\n') {
                state->read_upto = state->write_to;
                aeCreateFileEvent(eventLoop, fd, AE_WRITABLE, connWriteProc, state); // 把数据及时送回去
            }
        }
    }

    if(result == 0) {
        if(state->read_from == state->read_upto) // 不能立即释放，得等没有待写的数据时再释放
        {
            free_fd_state(state);
            close(fd);
        }
    } else if(result < 0) {
        if(errno == EAGAIN)     // XXX use evutil macro
        {
            return; // 这一次的读事件处理完了
        }
        perror("recv");
        free_fd_state(state);
        close(fd);
    }
}

void
listenerProc(struct aeEventLoop *eventLoop, int listenFd, void *clientData, int mask)
{
    struct sockaddr_storage ss;
    socklen_t slen;
    int fd;

    fprintf(stderr,"blocked in accept()\n");
    sleep(5);

    slen = sizeof(ss);
    fd = accept(listenFd, (struct sockaddr *)&ss, &slen);
    if(fd < 0)  {   // XXX EAGAIN?
        perror("accept");
    } else if(fd > FD_SETSIZE) {
        close(fd);
    } else {
        struct fd_state *state;
        set_nonblocking(fd);
        state = alloc_fd_state(fd);
        assert(state);

        int rc = aeCreateFileEvent(eventLoop, fd, AE_READABLE, connReadProc, state);
        fprintf(stderr,"accept fd=%d rc=%d\n", fd, rc);
    }
}

int
timerProc(struct aeEventLoop *eventLoop, long long id, void *clientData)
{
    printf("timer %lld timeout: %d\n", id, (int)time(NULL));
    return 1000;
}

void
run(void)
{
    int listener;
    struct sockaddr_in sin;

    aeEventLoop *loop;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(12345);

    listener = socket(AF_INET, SOCK_STREAM, 0);

    // make it nonblocking
    set_nonblocking(listener);

    int one = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    if(bind(listener, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("bind");
        return;
    }

    if(listen(listener, 16) < 0) {
        perror("listen");
        return;
    }

    loop = aeCreateEventLoop(64);
    assert(loop);

    (void)aeCreateTimeEvent(loop, 1000, timerProc, NULL, NULL);
    aeCreateFileEvent(loop, listener, AE_READABLE, listenerProc, NULL);

    aeMain(loop);

    aeDeleteEventLoop(loop);
}

int
main(void)
{
    run();
    return 0;
}
