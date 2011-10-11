/*
  Libela, an event-loop abstraction library.

  This file is part of FOILS, the Freebox Open Interface Libraries.
  This file is distributed under a 2-clause BSD license, see
  LICENSE.TXT for details.

  Copyright (c) 2011, Freebox SAS
  See AUTHORS for details
 */

/**
   @file
   @module {Backends}
   @short CoreFoundation backend
 */


#ifndef ELA_CF_H
#define ELA_CF_H

#include <ela/ela.h>
#include <CoreFoundation/CFRunLoop.h>

/**
   @this creates an adapter to core foundation runloop.

   @returns an event loop, or NULL if core foundation support is
   unavailable.
 */
struct ela_el *ela_cf(CFRunLoopRef event);

/**
   @this retrieves the runloop under the ELA one. Returns NULL if the
   ela is not an actual CFRunLoop.

   @param ela Abstract event loop
   @returns a CFRunLoop reference, if applicable
 */
CFRunLoopRef ela_cf_get_runloop(struct ela_el *ela);

#endif
