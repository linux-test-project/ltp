/*
 *	@(#)stat2.c	1.2 98/10/26 Connectathon Testsuite
 *	1.3 Lachman ONC Test Suite source
 *
 * create a bunch of files and stat them repeatedly
 */

#if defined (DOS) || defined (WIN32)
#define DOSorWIN32
#endif

#ifndef DOSorWIN32
#include	 <stdio.h>
#include	 <stdlib.h>
#include <unistd.h>
#include	 <sys/types.h>
#include	 <sys/stat.h>
#include	 <fcntl.h>
#include <string.h>
#ifdef SVR3
#include	 <sys/fs/nfs/time.h>
#else
#include	 <sys/time.h>
#endif
#endif /* DOSorWIN32 */

#include "../tests.h"

static int stats = 0;

int
main(argc, argv)
	int argc;
	char *argv[];
{
	struct timeval etim;
	float elapsed;
	int files, count;
	register int pass, filenum;
	char name[256];
	struct stat statb;

	if (argc != 4) {
		fprintf(stderr, "usage: %s dir files count\n", argv[0]);
		exit(1);
	}

	if (mkdir(argv[1], 0777) < 0) {
		perror(argv[1]);
	}
	if (chdir(argv[1]) < 0) {
		fprintf(stderr, "can't chdir to %s: %s\n",
			argv[1], strerror(errno));
		exit(1);
	}
	files = atoi(argv[2]);
	count = atoi(argv[3]);
	for (filenum = 0; filenum < files; filenum++) {
		sprintf(name, "%d", filenum);
		close(creat(name, 0666));
	}

	starttime();
	for (pass = 0; pass < count; pass++) {
		for (filenum = 0; filenum < files; filenum++) {
			sprintf(name, "%d", filenum);
			if (stat(name, &statb) < 0) {
				fprintf(stderr,
					"pass %d: stat of %s failed: %s\n",
					pass, name, strerror(errno));
				exit(1);
			}
			stats++;
		}
	}
	endtime(&etim);
	elapsed = (float)etim.tv_sec + (float)etim.tv_usec / 1000000.0;
	fprintf(stdout, "%d calls in %f seconds (%f calls/sec)\n",
	    stats, elapsed, (float)stats / elapsed);
	exit(0);
}
