/*
  Libela, an event-loop abstraction library.

  This file is part of FOILS, the Freebox Open Interface Libraries.
  This file is distributed under a 2-clause BSD license, see
  LICENSE.TXT for details.

  Copyright (c) 2011, Freebox SAS
  See AUTHORS for details
 */

#ifndef ELA_LIBEVENT_H
#define ELA_LIBEVENT_H

/**
   @file
   @module {Backends}
   @short libevent backend
 */

#include <ela/ela.h>

struct event_base;

/**
   @this creates an adapter to libevent.

   @returns an event loop, or NULL if libevent support is unavailable.
 */
struct ela_el *ela_libevent(struct event_base *event);

#endif
