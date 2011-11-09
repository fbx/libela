/*
  Libela, an event-loop abstraction library.

  This file is part of FOILS, the Freebox Open Interface Libraries.
  This file is distributed under a 2-clause BSD license, see
  LICENSE.TXT for details.

  Copyright (c) 2011, Freebox SAS
  See AUTHORS for details
 */


#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ela/ela.h>
#include <ela/backend.h>
#include <ela/cf.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <CoreFoundation/CFRunLoop.h>
#include <CoreFoundation/CFFileDescriptor.h>

struct cf_mainloop
{
    struct ela_el base;
    CFRunLoopRef runloop;
    int auto_allocated;
};

struct ela_event_source
{
    int ref;
    uint32_t flags;

    int fd;
    CFFileDescriptorRef fd_cf;
    CFRunLoopSourceRef fd_source;

    struct timeval tv;
    CFRunLoopTimerRef timeout_source;

    ela_handler_func *handler;
    void *priv;
};

static void source_ref(struct ela_event_source *src)
{
    src->ref += 1;
}

static int source_release(struct ela_event_source *src)
{
    src->ref -= 1;

    int nref = src->ref;

    if ( nref == 0 )
        free(src);

    return nref;
}

static
void _fd_add(CFRunLoopRef rl, struct ela_event_source *src)
{
    if ( ! src->fd_source )
        return;

    if ( !CFRunLoopContainsSource(rl, src->fd_source, kCFRunLoopCommonModes) )
        CFRunLoopAddSource(rl, src->fd_source, kCFRunLoopCommonModes);

    int on = 0;

    if ( src->flags & ELA_EVENT_WRITABLE )
        on |= kCFFileDescriptorWriteCallBack;

    if ( src->flags & ELA_EVENT_READABLE )
        on |= kCFFileDescriptorReadCallBack;

    CFFileDescriptorDisableCallBacks(
        src->fd_cf,
        on ^ (kCFFileDescriptorWriteCallBack|kCFFileDescriptorWriteCallBack));

    CFFileDescriptorEnableCallBacks(
        src->fd_cf,
        on);
}

static
void _fd_remove(CFRunLoopRef rl, struct ela_event_source *src)
{
    if ( ! src->fd_source )
        return;

    CFRunLoopRemoveSource(rl, src->fd_source, kCFRunLoopCommonModes);
}

static
void _timeout_set(CFRunLoopRef rl, struct ela_event_source *src)
{
    CFAbsoluteTime now = CFAbsoluteTimeGetCurrent();
    CFTimeInterval interval
        = ((double)src->tv.tv_sec + (double)src->tv.tv_usec * 1e-6);

    if ( CFRunLoopContainsTimer(rl, src->timeout_source,
                                kCFRunLoopCommonModes) )
        CFRunLoopRemoveTimer(rl, src->timeout_source, kCFRunLoopCommonModes);

    CFRunLoopTimerSetNextFireDate(src->timeout_source, now+interval);

    if ( !CFRunLoopContainsTimer(rl, src->timeout_source,
                                 kCFRunLoopCommonModes) )
        CFRunLoopAddTimer(rl, src->timeout_source, kCFRunLoopCommonModes);
}

static
void elacf_drop_fd(struct ela_event_source *src)
{
    CFRelease(src->fd_cf);
    src->fd_cf = NULL;
}

static
void elacf_drop_fd_source(struct ela_event_source *src)
{
    CFRelease(src->fd_source);
    src->fd_source = NULL;
}

static
void elacf_drop_timer_source(struct ela_event_source *src)
{
    src->timeout_source = NULL;
    src->flags &= ~ELA_EVENT_TIMEOUT;
}

static
void fd_cf_callback(
    CFFileDescriptorRef f,
    CFOptionFlags reason,
    void *info)
{
    struct ela_event_source *src = info;
    int ela_flags = 0;

    if ( reason & kCFFileDescriptorReadCallBack )
        ela_flags |= ELA_EVENT_READABLE;

    if ( reason & kCFFileDescriptorWriteCallBack )
        ela_flags |= ELA_EVENT_WRITABLE;

    source_ref(src);

    src->handler(src, src->fd, ela_flags, src->priv);

    if ( source_release(src) ) {
        if ( src->flags & ELA_EVENT_ONCE )
            _fd_remove(CFRunLoopGetCurrent(), src);
        else
            _fd_add(CFRunLoopGetCurrent(), src);

        if ( src->flags & ELA_EVENT_TIMEOUT )
            _timeout_set(CFRunLoopGetCurrent(), src);
    }
}

static
void cf_timeout_callback(CFRunLoopTimerRef timer, void *info)
{
    struct ela_event_source *src = info;

    if ( src->flags & ELA_EVENT_TIMEOUT )
        _timeout_set(CFRunLoopGetCurrent(), src);

    src->handler(src, src->fd, ELA_EVENT_TIMEOUT, src->priv);
}

static
ela_error_t _ela_cf_set_fd(
    struct ela_el *ctx_,
    struct ela_event_source *src,
    int fd,
    uint32_t ela_flags)
{
    struct cf_mainloop *ctx = (struct cf_mainloop *)ctx_;

    (void)ctx;

    CFFileDescriptorContext context = {
        .info = src,
    };
    src->fd_cf = CFFileDescriptorCreate(
        kCFAllocatorDefault, fd, false,
        fd_cf_callback, &context);

    src->fd_source = CFFileDescriptorCreateRunLoopSource(
        kCFAllocatorDefault, src->fd_cf, 0);
    if ( !src->fd_source )
        return ENOMEM;

    const uint32_t fd_flags
        = (ELA_EVENT_ONCE|ELA_EVENT_READABLE|ELA_EVENT_WRITABLE);
    src->flags = (src->flags & ~fd_flags) | (ela_flags & fd_flags);

    return 0;
}

static
ela_error_t _ela_cf_set_timeout(
    struct ela_el *ctx_,
    struct ela_event_source *src,
    const struct timeval *tv,
    uint32_t ela_flags)
{
    struct cf_mainloop *ctx = (struct cf_mainloop *)ctx_;

    const uint32_t timeout_flags = (ELA_EVENT_ONCE|ELA_EVENT_TIMEOUT);

    (void)ctx;

    if ( tv != NULL ) {
        memcpy(&src->tv, tv, sizeof(*tv));
        ela_flags |= ELA_EVENT_TIMEOUT;
        src->flags
            = (src->flags & ~timeout_flags) | (ela_flags & timeout_flags);
    } else {
        src->flags &= ~ELA_EVENT_TIMEOUT;
    }

    return 0;
}

static
ela_error_t _ela_cf_remove(
    struct ela_el *ctx_,
    struct ela_event_source *src)
{
    struct cf_mainloop *ctx = (struct cf_mainloop *)ctx_;

    _fd_remove(ctx->runloop, src);

//    _timeout_remove(ctx->runloop, src);
    if ( CFRunLoopContainsTimer(ctx->runloop, src->timeout_source,
                                kCFRunLoopCommonModes) )
        CFRunLoopRemoveTimer(
            ctx->runloop, src->timeout_source,
            kCFRunLoopCommonModes);

    return 0;
}

static
ela_error_t _ela_cf_add(
    struct ela_el *ctx_,
    struct ela_event_source *src)
{
    struct cf_mainloop *ctx = (struct cf_mainloop *)ctx_;

    _fd_add(ctx->runloop, src);

    if ( src->flags & ELA_EVENT_TIMEOUT )
        _timeout_set(ctx->runloop, src);

    return 0;
}

static
void _ela_cf_close(struct ela_el *ctx_)
{
    struct cf_mainloop *ctx = (struct cf_mainloop *)ctx_;
    free(ctx);
}

static
void _ela_cf_run(struct ela_el *ctx_)
{
    struct cf_mainloop *ctx = (struct cf_mainloop *)ctx_;
    (void)ctx;
    CFRunLoopRun();
}

static
void _ela_cf_exit(struct ela_el *ctx_)
{
    struct cf_mainloop *ctx = (struct cf_mainloop *)ctx_;
    CFRunLoopStop(ctx->runloop);
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

    (void)ctx;

    if ( src == NULL )
        return ENOMEM;

    memset(src, 0, sizeof(*src));

    src->priv = priv;
    src->handler = func;
    src->flags = 0;

    CFRunLoopTimerContext context = {
        .info = src,
    };

    CFAbsoluteTime now = CFAbsoluteTimeGetCurrent();
    CFTimeInterval year = 365 * 1440 * 60;

    src->timeout_source = CFRunLoopTimerCreate(
        kCFAllocatorDefault, now + 10 * year, year,
        0, 0, cf_timeout_callback, &context);

    src->ref = 1;

    *source = src;
    return 0;
}

static
void _ela_source_free(
    struct ela_el *ctx_,
    struct ela_event_source *src)
{
    struct cf_mainloop *ctx = (struct cf_mainloop *)ctx_;

    if ( src->fd_source )
        elacf_drop_fd_source(src);

    elacf_drop_timer_source(src);

    if ( CFRunLoopContainsTimer(ctx->runloop, src->timeout_source,
                                kCFRunLoopCommonModes) )
        CFRunLoopRemoveTimer(
            ctx->runloop, src->timeout_source,
            kCFRunLoopCommonModes);
    CFRelease(src->timeout_source);

    if ( src->fd_cf )
        elacf_drop_fd(src);

    source_release(src);
}

static
struct ela_el *_ela_cf_create(void)
{
    struct ela_el *el = ela_cf(CFRunLoopGetCurrent());
    struct cf_mainloop *ctx = (struct cf_mainloop *)el;
    ctx->auto_allocated = 1;
    return el;
}

static const struct ela_el_backend backend =
{
    .source_alloc = _ela_source_alloc,
    .source_free = _ela_source_free,
    .set_fd = _ela_cf_set_fd,
    .set_timeout = _ela_cf_set_timeout,
    .remove = _ela_cf_remove,
    .add = _ela_cf_add,
    .close = _ela_cf_close,
    .run = _ela_cf_run,
    .exit = _ela_cf_exit,
    .name = "CFRunLoop",
    .create = _ela_cf_create,
};

ELA_EXPORT
struct ela_el *ela_cf(CFRunLoopRef runloop)
{
    struct cf_mainloop *ctx = malloc(sizeof(*ctx));
    if ( ctx == NULL )
        return NULL;

    ctx->runloop = runloop;
    ctx->base.backend = &backend;
    ctx->auto_allocated = 0;

    return &ctx->base;
}

__attribute__((constructor))
static void _ela_cf_register(void)
{
    ela_register(&backend);
}

CFRunLoopRef ela_cf_get_runloop(struct ela_el *ela)
{
    struct cf_mainloop *ctx = (struct cf_mainloop *)ela;
    return ctx->runloop;
}
