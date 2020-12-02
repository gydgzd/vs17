/*
 * HttpServerLibevent.cpp
 *
 *  Created on: Jul 2, 2020
 *      Author: gyd
 */

#include "HttpServerLibevent.h"

HttpServerLibevent::HttpServerLibevent() {
    // TODO Auto-generated constructor stub

}

HttpServerLibevent::~HttpServerLibevent() {
    // TODO Auto-generated destructor stub
}



char rot13_char(char c)
{
    /* We don't want to use isalpha here; setting the locale would change
     * which characters are considered alphabetical. */
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c - 13;
    else
        return c;
}

struct fd_state {
    char buffer[MAX_LINE];
    size_t buffer_used;

    size_t n_written;
    size_t write_upto;

    struct event *read_event;
    struct event *write_event;
};

struct fd_state * alloc_fd_state(struct event_base *base, evutil_socket_t fd)
{
    struct fd_state *state = (struct fd_state *)malloc(sizeof(struct fd_state));
    if (!state)
        return NULL;
    state->read_event = event_new(base, fd, EV_READ|EV_PERSIST, do_read, state);
    if (!state->read_event) {
        free(state);
        return NULL;
    }
    state->write_event = event_new(base, fd, EV_WRITE|EV_PERSIST, do_write, state);

    if (!state->write_event) {
        event_free(state->read_event);
        free(state);
        return NULL;
    }

    state->buffer_used = state->n_written = state->write_upto = 0;

 //   assert(state->write_event);
    return state;
}
int called = 0;
static void signal_cb(evutil_socket_t fd, short event, void *arg)
{
    struct event *signal = (struct event *)arg;
    printf("signal_cb: got signal %d\n", event_get_signal(signal));
    if (called >= 2)
        event_del(signal);
    called++;
}

void free_fd_state(struct fd_state *state)
{
    event_free(state->read_event);
    event_free(state->write_event);
    free(state);
}

void do_read(evutil_socket_t fd, short events, void *arg)
{
    struct fd_state *state = (struct fd_state *)arg;
    char buf[1024];
    int i;
    ssize_t result;
    while (1) {
    //    assert(state->write_event);
        result = recv(fd, buf, sizeof(buf), 0);
        if (result <= 0)
            break;

        for (i=0; i < result; ++i)  {
            if (state->buffer_used < sizeof(state->buffer))
                state->buffer[state->buffer_used++] = rot13_char(buf[i]);
            if (buf[i] == '\n') {
            //    assert(state->write_event);
                event_add(state->write_event, NULL);
                state->write_upto = state->buffer_used;
            }
        }
    }

    if (result == 0) {
        free_fd_state(state);
    } else if (result < 0) {
        if (errno == EAGAIN) // XXXX use evutil macro
            return;
        perror("recv");
        free_fd_state(state);
    }
}

void do_write(evutil_socket_t fd, short events, void *arg)
{
    struct fd_state *state = (struct fd_state *)arg;

    while (state->n_written < state->write_upto) {
        ssize_t result = send(fd, state->buffer + state->n_written,
                              state->write_upto - state->n_written, 0);
        if (result < 0) {
            if (errno == EAGAIN) // XXX use evutil macro
                return;
            free_fd_state(state);
            return;
        }
     //   assert(result != 0);

        state->n_written += result;
    }

    if (state->n_written == state->buffer_used)
        state->n_written = state->write_upto = state->buffer_used = 1;

    event_del(state->write_event);
}

void do_accept(evutil_socket_t listener, short event, void *arg)
{
    struct event_base *base = (struct event_base *)arg;
    struct sockaddr_storage ss;
    int slen = sizeof(ss);
    int fd = accept(listener, (struct sockaddr*)&ss, &slen);
    if (fd < 0) { // XXXX eagain??
        perror("accept");
    } else if (fd > FD_SETSIZE) {
        EVUTIL_CLOSESOCKET(fd); // XXX replace all closes with EVUTIL_CLOSESOCKET */
    } else {
        struct fd_state *state;
        evutil_make_socket_nonblocking(fd);
        state = alloc_fd_state(base, fd);
    //    assert(state); /*XXX err*/
    //    assert(state->write_event);
        event_add(state->read_event, NULL);
    }
}

void run(void)
{
    evutil_socket_t listener;
    struct sockaddr_in sin;
    struct event_base *base;
    struct event *listener_event;

    base = event_base_new();
    if (!base)
        return; /*XXXerr*/
    std::cout << event_base_get_method(base);      // win32

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(40713);

    listener = socket(AF_INET, SOCK_STREAM, 0);
    evutil_make_socket_nonblocking(listener);
    evutil_make_listen_socket_reuseable(listener); //设置端口重用
#ifndef WIN32
    {
        int one = 1;
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    }
#endif

    if (bind(listener, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("bind");
        return;
    }

    if (listen(listener, 16)<0) {
        perror("listen");
        return;
    }

    listener_event = event_new(base, listener, EV_READ|EV_PERSIST, do_accept, (void*)base);
    /*XXX check it */
    event_add(listener_event, NULL);
    event_base_dispatch(base);     // 阻塞
    event_base_free(base);
}

int HttpServerLibevent::testLibevent()
{
    setvbuf(stdout, NULL, _IONBF, 0);

//    run();
    // test signal
    struct event *signal_int;
    struct event_base* base;
#ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    (void)WSAStartup(wVersionRequested, &wsaData);
#endif

    /* Initalize the event library */
    base = event_base_new();

    /* Initalize one event */
    signal_int = evsignal_new(base, SIGINT, signal_cb, event_self_cbarg());

    event_add(signal_int, NULL);

    event_base_dispatch(base);
    event_free(signal_int);
    event_base_free(base);
    //



    return 0;
}
