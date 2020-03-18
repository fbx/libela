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
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ela/ela.h>
#include <ela/backend.h>
#include <string.h>

static  struct ela_el *el = NULL;
static int cpt = 3;

static
int set_nonblocking(int fd)
{
#if defined(O_NONBLOCK)
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    int flags = 1;
    return ioctl(fd, FIOBIO, &flags);
#endif
}

static
void cb(struct ela_event_source *source, int fd,
        uint32_t mask, void *data)
{
    printf("callback. Flags: %02x\n", (int)mask);

    if ( mask & ELA_EVENT_READABLE ) {
        printf("Readable !\n");
        char buffer[1024];
        int ret = read(fd, buffer, 1024);
        (void)ret;

        if ( --cpt >= 0 )
            printf("%d left\n", cpt);
        else
            ela_exit(el);
    }
}

int main(int argc, char **argv)
{
    el = ela_create(argc > 1 ? argv[1] : NULL);

    if ( el == NULL ) {
        fprintf(stderr, "No suitable event loop\n");
        return 1;
    }

    printf("Using the %s backend\n", el->backend->name);

    struct ela_event_source *source;

    ela_error_t err = ela_source_alloc(el, cb, el, &source);

    set_nonblocking(0);

    if ( err )
        goto ela_err;

    const int fd = 0;

    /* anchor source_fd */
    ela_set_fd(el, source, fd, ELA_EVENT_READABLE);
    /* anchor end */

    ela_add(el, source);

    ela_run(el);

    ela_source_free(el, source);
    ela_close(el);

    return 0;

ela_err:
    fprintf(stderr, "Source allocation failed: %s\n", strerror(err));
    return 1;
}
