/*
  Libela, an event-loop abstraction library.

  This file is part of FOILS, the Freebox Open Interface Libraries.
  This file is distributed under a 2-clause BSD license, see
  LICENSE.TXT for details.

  Copyright (c) 2011, Freebox SAS
  See AUTHORS for details
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ela/ela.h>
#include <ela/backend.h>
#include <ela/libevent.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <event.h>

#if defined(__GNUC__)
#define INITIALIZER(f) \
    static void f(void) __attribute__((constructor)); \
    static void f(void)
#endif

struct libevent_mainloop
{
    struct ela_el base;
    struct event_base *event;
    int auto_allocated;
};

struct ela_event_source
{
    ela_handler_func *handler;
    struct event event;
    struct timeval timeout;
    void *priv;
    uint32_t flags;
};

static ela_error_t _real_add(struct ela_event_source *src)
{
    const struct timeval *tv = &src->timeout;

    if ( ! (src->flags & ELA_EVENT_TIMEOUT) )
        tv = NULL;

    event_del(&src->event);
    int ev_err = event_add(&src->event, tv);
    if ( ev_err )
        return ECANCELED;

    return 0;
}

static
void _ela_event_cb(int fd, short ev_flags, void *priv)
{
    struct ela_event_source *src = priv;
    int ela_flags = 0;

    if ( ev_flags & EV_READ ) ela_flags |= ELA_EVENT_READABLE;
    if ( ev_flags & EV_WRITE ) ela_flags |= ELA_EVENT_WRITABLE;
    if ( ev_flags & EV_TIMEOUT ) ela_flags |= ELA_EVENT_TIMEOUT;

    if ( (src->flags & ELA_EVENT_TIMEOUT) && !(src->flags & ELA_EVENT_ONCE) )
        _real_add(src);

    src->handler(src, fd, ela_flags, src->priv);
}

static
ela_error_t _ela_event_set_fd(
    struct ela_el *ctx_,
    struct ela_event_source *src,
    int fd,
    uint32_t ela_flags)
{
    struct libevent_mainloop *ctx = (struct libevent_mainloop *)ctx_;
    ela_error_t err = 0;
    uint32_t fd_flags
        = (ELA_EVENT_ONCE|ELA_EVENT_READABLE|ELA_EVENT_WRITABLE);

    (void)ctx;

    int ev_flags = EV_PERSIST;

    if ( ela_flags & ELA_EVENT_ONCE ) ev_flags &= ~EV_PERSIST;
    if ( ela_flags & ELA_EVENT_READABLE ) ev_flags |= EV_READ;
    if ( ela_flags & ELA_EVENT_WRITABLE ) ev_flags |= EV_WRITE;

    src->flags = (src->flags & ~fd_flags) | (ela_flags & fd_flags);

    event_set(&src->event, fd, ev_flags, _ela_event_cb, src);

    return err;
}

static
ela_error_t _ela_event_set_timeout(
    struct ela_el *ctx_,
    struct ela_event_source *src,
    const struct timeval *tv,
    uint32_t ela_flags)
{
    struct libevent_mainloop *ctx = (struct libevent_mainloop *)ctx_;

    const uint32_t timeout_flags = (ELA_EVENT_ONCE|ELA_EVENT_TIMEOUT);

    (void)ctx;

    if ( tv != NULL ) {
        memcpy(&src->timeout, tv, sizeof(*tv));
        ela_flags |= ELA_EVENT_TIMEOUT;
        src->flags
            = (src->flags & ~timeout_flags) | (ela_flags & timeout_flags);
    } else {
        src->flags &= ~ELA_EVENT_TIMEOUT;
    }

    return 0;
}

static
ela_error_t _ela_event_remove(
    struct ela_el *ctx_,
    struct ela_event_source *src)
{
    struct libevent_mainloop *ctx = (struct libevent_mainloop *)ctx_;
    (void)ctx;

    event_del(&src->event);

    return 0;
}

static
ela_error_t _ela_event_add(
    struct ela_el *ctx_,
    struct ela_event_source *src)
{
    struct libevent_mainloop *ctx = (struct libevent_mainloop *)ctx_;
    (void)ctx;

    event_base_set(ctx->event, &src->event);
    return _real_add(src);
}

static
void _ela_event_close(struct ela_el *ctx_)
{
    struct libevent_mainloop *ctx = (struct libevent_mainloop *)ctx_;
    if ( ctx->auto_allocated )
        event_base_free(ctx->event);
    free(ctx);
}

static
void _ela_event_run(struct ela_el *ctx_)
{
    struct libevent_mainloop *ctx = (struct libevent_mainloop *)ctx_;
    event_base_dispatch(ctx->event);
}

static
void _ela_event_exit(struct ela_el *ctx_)
{
    struct libevent_mainloop *ctx = (struct libevent_mainloop *)ctx_;
    event_base_loopbreak(ctx->event);
}

static
ela_error_t _ela_source_alloc(
    struct ela_el *ctx_,
    ela_handler_func *func,
    void *priv,
    struct ela_event_source **source)
{
    struct libevent_mainloop *ctx = (struct libevent_mainloop *)ctx_;
    struct ela_event_source *src = malloc(sizeof(*src));

    if ( src == NULL )
        return ENOMEM;

    src->priv = priv;
    src->handler = func;
    src->flags = 0;
    event_set(&src->event, -1, EV_PERSIST, _ela_event_cb, src);
    event_base_set(ctx->event, &src->event);

    *source = src;
    return 0;
}

static
void _ela_source_free(
    struct ela_el *ctx_,
    struct ela_event_source *src)
{
    _ela_event_remove(ctx_, src);
    free(src);
}

static
struct ela_el *_ela_event_create(void)
{
    struct event_base *event = event_base_new();
    struct ela_el *el = ela_libevent(event);
    struct libevent_mainloop *ctx = (struct libevent_mainloop *)el;
    ctx->auto_allocated = 1;
    return el;
}

static const struct ela_el_backend event_backend =
{
    .source_alloc = _ela_source_alloc,
    .source_free = _ela_source_free,
    .set_fd = _ela_event_set_fd,
    .set_timeout = _ela_event_set_timeout,
    .remove = _ela_event_remove,
    .add = _ela_event_add,
    .close = _ela_event_close,
    .run = _ela_event_run,
    .exit = _ela_event_exit,
    .name = "libevent",
    .create = _ela_event_create,
};

ELA_EXPORT
struct ela_el *ela_libevent(struct event_base *event)
{
    struct libevent_mainloop *m = malloc(sizeof(*m));
    if ( m == NULL )
        return NULL;

    m->event = event;
    m->base.backend = &event_backend;
    m->auto_allocated = 0;
    return &m->base;
}

#if defined(_MSC_VER)
BOOLEAN WINAPI
DllMain(IN HINSTANCE hDllHandle, IN DWORD nReason, IN LPVOID Reserved)
{
    switch (nReason) {
    case DLL_PROCESS_ATTACH:
        //DisableThreadLibraryCalls(hDllHandle);
        ela_register(&event_backend);
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}
#else
INITIALIZER(_ela_event_register)
{
    ela_register(&event_backend);
}
#endif
