/*
  Libela, an event-loop abstraction library.

  This file is part of FOILS, the Freebox Open Interface Libraries.
  This file is distributed under a 2-clause BSD license, see
  LICENSE.TXT for details.

  Copyright (c) 2011, Freebox SAS
  See AUTHORS for details
 */

#ifndef ELA_BACKEND_H
#define ELA_BACKEND_H

/**
   @file
   @module {Backend API}
   @short Libela implementor's API
 */

#include <ela/ela.h>

/** Functions to be implemented by a event loop backend */
struct ela_el_backend
{
    /** Allocate an new event source structure. See @ref ela_source_alloc */
    ela_error_t (*source_alloc)(
        struct ela_el *context,
        ela_handler_func *func,
        void *priv,
        struct ela_event_source **ret);

    /** Release an event source structure. See @ref ela_source_free */
    void (*source_free)(
        struct ela_el *context,
        struct ela_event_source *src);

    /** FD watcher registration. See @ref ela_set_fd */
    ela_error_t (*set_fd)(
        struct ela_el *context,
        struct ela_event_source *src,
        int fd,
        uint32_t flags);

    /** Timeout. See @ref ela_set_timeout */
    ela_error_t (*set_timeout)(
        struct ela_el *context,
        struct ela_event_source *src,
        const struct timeval *tv,
        uint32_t flags);

    /** Watcher registration. See @ref ela_add */
    ela_error_t (*add)(struct ela_el *context,
                       struct ela_event_source *src);

    /** Watcher unregistration. See @ref ela_remove */
    ela_error_t (*remove)(struct ela_el *context,
                          struct ela_event_source *src);

    /** Make the event loop exit. See @ref ela_exit */
    void (*exit)(struct ela_el *context);

    /** Run the event loop. See @ref ela_run */
    void (*run)(struct ela_el *context);

    /** Close the event loop. See @ref ela_close */
    void (*close)(struct ela_el *context);

    /** Backend name for enumeration and selection */
    const char *name;

    /** Standalone constructor */
    struct ela_el *(*create)(void);
};

/**
   @this is an event loop context structure. Implementations may
   decide to inherit this declaration and add other internal fields
   like:

   @code
   struct my_event_loop_adapter
   {
       struct ela_el base;
       struct my_event_loop *loop;
   };
   @end code

   Then allocate a @tt {struct my_event_loop_adapter adapter} and
   @code return &adapter->base; @end code.
 */
struct ela_el
{
    /**
       Pointer to the event loop backend.
     */
    const struct ela_el_backend *backend;
};

/**
   @this registers a backend to the global libela backend list.  This
   provides a new backend to @ref ela_create.

   @param backend The backend to register
 */
ELA_EXPORT
void ela_regsiter(const struct ela_el_backend *backend);

#endif
