/*
  LIBELA_BSD_LICENSE_BEGIN

  This file is part of Libela.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the following
  disclaimer in the documentation and/or other materials provided
  with the distribution.

  LIBELA_BSD_LICENSE_END

  Copyright (c) 2011, Freebox SAS
  See AUTHORS for details
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ela/ela.h>

/* anchor callback */
static
void cb(struct ela_event_source *source, int fd,
        uint32_t mask, void *data)
{
    printf("callback. Flags: %02x\n", (int)mask);

    if ( mask & ELA_EVENT_TIMEOUT )
        printf("Timeout !\n");
}
/* anchor end */

int main(int argc, char **argv)
{
    /* anchor creation */
    const char *backend_name = argc > 1 ? argv[1] : NULL;

    struct ela_el *el = ela_create(backend_name);

    if ( el == NULL ) {
        fprintf(stderr, "No suitable event loop\n");
        return 1;
    }
    /* anchor end */

    /* anchor source_creation */
    struct ela_event_source *source;

    ela_error_t err = ela_source_alloc(el, cb, NULL, &source);
    /* anchor end */

    if ( err )
        goto ela_err;

    /* anchor source_timeout */
    struct timeval tv = {0, 500000};
    ela_set_timeout(el, source, &tv, 0);
    /* anchor end */
    /* anchor source_add */
    ela_add(el, source);
    /* anchor end */

    ela_run(el);

    /* anchor cleanup */
    ela_source_free(el, source);
    ela_close(el);
    /* anchor end */

    return 0;

ela_err:
    fprintf(stderr, "Source allocation failed: %s\n", strerror(err));
    return 1;
}
