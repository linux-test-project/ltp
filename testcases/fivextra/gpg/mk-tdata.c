/* mk-tdata.c -  Create some simple random testdata
 * Copyright (C) 1998, 1999, 2000, 2001 Free Software Foundation, Inc.
 *
 * This file is free software; as a special exception the author gives
 * unlimited permission to copy and/or distribute it, with or without
 * modifications, as long as this notice is preserved.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

/* Modified for MCP. -RR */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#ifndef RAND_MAX   /* for SunOS */
#define RAND_MAX 32767
#endif

#ifndef HAVE_RAND /* default for Linux */
#define HAVE_RAND 1
#endif

int
main(int argc, char **argv)
{
    int i, c;
    int limit =0;

    limit = argc > 1 ? atoi(argv[1]) : 0;

    srand(getpid());

    for(i=0; !limit || i < limit; i++ ) {
      #ifdef HAVE_RAND
	c = ((unsigned)(1 + (int) (256.0*rand()/(RAND_MAX+1.0)))-1);
      #else
	c = ((unsigned)(1 + (int) (256.0*random()/(RAND_MAX+1.0)))-1);
      #endif
	putchar(c);
    }
    return 0;
}

