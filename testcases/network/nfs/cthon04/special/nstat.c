/*
 *	@(#)nstat.c	1.2 98/10/26 Connectathon Testsuite
 *	1.3 Lachman ONC Test Suite source
 *
 * Stat a file n times
 */

#if defined (DOS) || defined (WIN32)
#define DOSorWIN32
#endif

#ifndef DOSorWIN32
#include	<stdio.h>
#include	<stdlib.h>
#include <string.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#ifdef SVR3
#include	<sys/fs/nfs/time.h>
#else
#include	<sys/time.h>
#endif
#endif /* DOSorWIN32 */

#include "../tests.h"

static int stats = 0;

main(argc, argv)
	int argc;
	char *argv[];
{
	struct timeval etim;
	float elapsed;
	register int count;
	register int i;
	struct stat statb;

	if (argc != 2) {
		fprintf(stderr, "usage: %s count\n", argv[0]);
		exit(1);
	}

	count = atoi(argv[1]);
	starttime();
	for (i=0; i<count; i++) {
		if (stat(argv[0], &statb) < 0) {
			fprintf(stderr, "pass %d: can't stat %s: %s\n",
				i, argv[0], strerror(errno));
			exit(1);
		}
		stats++;
	}
	endtime(&etim);
	elapsed = (float)etim.tv_sec + (float)etim.tv_usec / 1000000.0;
	if (elapsed == 0.0) {
		fprintf(stdout, "%d calls 0.0 seconds\n", count);
	} else {
		fprintf(stdout,
		    "%d calls %.2f seconds %.2f calls/sec %.2f msec/call\n",
		    count, elapsed, (float)count / elapsed,
		    1000.0 * elapsed / (float)count);
	}
	exit(0);
}
