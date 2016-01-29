/*
  Libela, an event-loop abstraction library.

  This file is part of FOILS, the Freebox Open Interface Libraries.
  This file is distributed under a 2-clause BSD license, see
  LICENSE.TXT for details.

  Copyright (c) 2011, Freebox SAS
  See AUTHORS for details
 */


#include <ela/ela.h>
#include <ela/backend.h>
#include <stdio.h>
#include <string.h>

#if 1
# define DBG(a, ...) printf(a, __VA_ARGS__)
#else
# define DBG(a, ...) do{}while(0)
#endif

ela_error_t ela_set_fd(
    struct ela_el *ctx,
    struct ela_event_source *src,
    int fd,
    uint32_t flags)
{
    ela_error_t err = ctx->backend->set_fd(ctx, src, fd, flags);
    if ( err ) {
        DBG("%s(%p, %p) : %d\n", __FUNCTION__, ctx, src, err);
    }
    return err;
}

ela_error_t ela_set_timeout(
    struct ela_el *ctx,
    struct ela_event_source *src,
    const struct timeval *tv,
    uint32_t flags)
{
    ela_error_t err = ctx->backend->set_timeout(ctx, src, tv, flags);
    if ( err ) {
        DBG("%s(%p, %p) : %d\n", __FUNCTION__, ctx, src, err);
    }
    return err;
}

ela_error_t ela_add(struct ela_el *ctx,
                    struct ela_event_source *src)
{
    ela_error_t err = ctx->backend->add(ctx, src);
    if ( err ) {
        DBG("%s(%p, %p) : %d\n", __FUNCTION__, ctx, src, err);
    }
    return err;
}

ela_error_t ela_remove(struct ela_el *ctx,
                       struct ela_event_source *src)
{
    ela_error_t err = ctx->backend->remove(ctx, src);
    if ( err ) {
        DBG("%s(%p, %p) : %d\n", __FUNCTION__, ctx, src, err);
    }
    return err;
}

void ela_run(struct ela_el *ctx)
{
    return ctx->backend->run(ctx);
}

void ela_exit(struct ela_el *ctx)
{
    return ctx->backend->exit(ctx);
}

void ela_close(struct ela_el *ctx)
{
    return ctx->backend->close(ctx);
}

ela_error_t ela_source_alloc(
    struct ela_el *ctx,
    ela_handler_func *func,
    void *priv,
    struct ela_event_source **ret)
{
    ela_error_t err = ctx->backend->source_alloc(ctx, func, priv, ret);
    if ( err ) {
        DBG("%s(%p) : %d\n", __FUNCTION__, ctx, err);
    }
    return err;
}

void ela_source_free(
    struct ela_el *ctx,
    struct ela_event_source *src)
{
    return ctx->backend->source_free(ctx, src);
}

#define REGISTRY_SIZE 8

static const struct ela_el_backend *registry[REGISTRY_SIZE] = {0};

void ela_register(const struct ela_el_backend *backend)
{
    size_t i;

    for ( i=0; i<REGISTRY_SIZE; ++i ) {
        if ( registry[i] != NULL )
            continue;
        if ( registry[i] == backend )
            return;
        registry[i] = backend;
        return;
    }
}

struct ela_el *ela_create(const char *name)
{
    size_t i;

    if ( name )
        for ( i=0; i<REGISTRY_SIZE; ++i ) {
            if ( registry[i] == NULL )
                continue;
            if ( strcmp(registry[i]->name, name) )
                continue;
            return registry[i]->create();
        }

    for ( i=0; i<REGISTRY_SIZE; ++i ) {
        if ( registry[i] == NULL )
            continue;
        return registry[i]->create();
    }

    return NULL;
}
