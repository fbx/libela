/*
  Libela, an event-loop abstraction library.

  This file is part of FOILS, the Freebox Open Interface Libraries.
  This file is distributed under a 2-clause BSD license, see
  LICENSE.TXT for details.

  Copyright (c) 2011, Freebox SAS
  See AUTHORS for details
 */

#ifndef ELA_H_
/** @internal */
#define ELA_H_

/**
   @file
   @module {User API}
   @short Libela user API
 */

#include <stdint.h>

#if !defined(_MSC_VER)
# include <sys/time.h>
#endif

/* GCC visibility */
#if defined(__GNUC__) && __GNUC__ >= 4 /** mkdoc:skip */
/** @internal */
# define ELA_EXPORT __attribute__ ((visibility("default")))
#elif defined(_MSC_VER) /** mkdoc:skip */
# define ELA_EXPORT __declspec(dllexport)
#else
/** @internal */
# define ELA_EXPORT
#endif

/**
   An event source handle
 */
struct ela_event_source;

/**
   @this is a callback function type on FD readiness or timeout.

   @param source Event source
   @param fd Relevant file descriptor, if any
   @param mask Bitmask of events available
   @param data Callback private data
 */
typedef void ela_handler_func(struct ela_event_source *source, int fd,
                              uint32_t mask, void *data);

/**
   @mgroup {Source source type control}
   Read available action
 */
#define ELA_EVENT_READABLE 1
/**
   @mgroup {Source source type control}
   Write available action
 */
#define ELA_EVENT_WRITABLE 2
/**
   @mgroup {Source source type control}
   Timeout action
 */
#define ELA_EVENT_TIMEOUT 4
/**
   @mgroup {Source source type control}
   Dont auto reinsert
 */
#define ELA_EVENT_ONCE 8

struct ela_el;

/**
   @this is the ela error code. Values are taken from the standard @tt
   errno codes. See @tt {man 7 errno}.  @em {No error} is guaranteed to be
   @tt 0.
 */
typedef int ela_error_t;

/**
   @this allocates a new event source structure for future usage for
   any type of event watching.

   @mgroup {Event source allocation}

   @param ctx The event loop context
   @param func Callback to call on event ready state
   @param priv Callback's private data
   @param ret (out) Event source handle

   @returns 0 if all went right, or an error
 */
ELA_EXPORT
ela_error_t ela_source_alloc(
    struct ela_el *context,
    ela_handler_func *func,
    void *priv,
    struct ela_event_source **ret);

/**
   @this frees an event source structure allocated with @ref
   ela_source_alloc.

   @mgroup {Event source allocation}

   @param ctx The event loop context
   @param src Event source handle to free
 */
ELA_EXPORT
void ela_source_free(
    struct ela_el *context,
    struct ela_event_source *src);

/**
   @this sets a file descriptor for watching in the event loop.

   @mgroup {Event source setup}

   @param ctx The event loop context
   @param src Event source handle, for unregistration
   @param fd File descriptor to watch for
   @param flags Bitmask of events to watch for The only relevant flags
          are @ref #ELA_EVENT_ONCE, @ref #ELA_EVENT_READABLE and
          @ref #ELA_EVENT_WRITABLE.
   @returns Whether things went all right

   The action stays watched on the FD until unregistration. No
   implicit unregistration occurs.
 */
ELA_EXPORT
ela_error_t ela_set_fd(
    struct ela_el *ctx,
    struct ela_event_source *src,
    int fd,
    uint32_t flags);

/**
   @this sets a timeout on which event is called if there is no event
   on the associated fd. If no fd is associated, only the timeout may
   get fired.

   @mgroup {Event source setup}

   @param ctx The event loop context
   @param src Event source handle, for unregistration
   @param tv Timeout expiration value, relative
   @param flags Bitmask of events to watch for.
          The only relevant flag is @ref #ELA_EVENT_ONCE.
   @returns Whether things went all right

   The action is fired only once, and gets unregistered afterwards.
 */
ELA_EXPORT
ela_error_t ela_set_timeout(
    struct ela_el *ctx,
    struct ela_event_source *src,
    const struct timeval *tv,
    uint32_t flags);

/**
   @this registers a watch and/or timeout in the event loop.

   @mgroup {Event source handling}

   @param ctx The event loop considered
   @param source Source to unregister
   @returns 0 or ENOENT
 */
ELA_EXPORT
ela_error_t ela_add(struct ela_el *ctx,
                    struct ela_event_source *source);

/**
   @this unregisters a watch, action, timeout, etc. in the event loop.

   @mgroup {Event source handling}

   @param ctx The event loop considered
   @param source Source to unregister
   @returns 0 or ENOENT
 */
ELA_EXPORT
ela_error_t ela_remove(struct ela_el *ctx,
                       struct ela_event_source *source);

/**
   @this runs the event loop.

   @mgroup {Event loop handling}

   The event loop processes events until:
   @list
     @item the event loop is broken through @ref ela_exit,
     @item the underlying event loop is broken,
     @item no event sources are left in the loop
   @end list

   @param ctx The event loop to run
 */
ELA_EXPORT
void ela_run(struct ela_el *ctx);

/**
   @this exits the event loop.

   @mgroup {Event loop handling}

   You may exit the event loop even if it was not explicitely started
   with @ref ela_run.

   For instance, if you run a GTK application, this will make the GLib
   mainloop exit even if you called @tt {gtk_main()} rather than @tt
   {ela_run()}.

   @param ctx The event loop to exit
 */
ELA_EXPORT
void ela_exit(struct ela_el *ctx);

/**
   @this frees the event loop.

   @mgroup {Event loop handling}

   @param ctx The event loop to close

   Note the context given to the constructor is left alive. Caller is
   responsible for it.
 */
ELA_EXPORT
void ela_close(struct ela_el *ctx);

/**
   @this creates an event loop using the first available registered
   backend.

   @mgroup {Event loop handling}

   This makes no preference in the backend.

   @param preferred Preferred backend name
   @returns a valid ela context, or NULL.
 */
ELA_EXPORT
struct ela_el *ela_create(const char *preferred);

#endif
