/*
 *	@(#)test3.c	1.7	00/12/30 Connectathon Testsuite
 *	1.5 Lachman ONC Test Suite source
 *
 * Test lookup up and down across mount points
 *
 * Uses the following important system calls against the server:
 *
 *	chdir()
 *	getcwd()
 *	stat()
 */

#if defined (DOS) || defined (WIN32)
/* If Dos, Windows or Win32 */
#define DOSorWIN32
#endif

#ifndef DOSorWIN32
#include <sys/param.h>
#include <unistd.h>
#endif

#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef DOSorWIN32
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <sys/types.h>

#include "../tests.h"

static int Tflag = 0;	/* print timing */
static int Fflag = 0;	/* test function only;  set count to 1, negate -t */
static int Nflag = 0;	/* Suppress directory operations */

static void
usage()
{
	fprintf(stdout, "usage: %s [-htfn] [count]\n", Myname);
	fprintf(stdout, "  Flags:  h    Help - print this usage info\n");
	fprintf(stdout, "          t    Print execution time statistics\n");
	fprintf(stdout, "          f    Test function only (negate -t)\n");
	fprintf(stdout, "          n    Suppress test directory create operations\n");
}

main(argc, argv)
	int argc;
	char *argv[];
{
	int count = 250;		/* times to do test */
	int ct;
	struct timeval time;
	struct stat statb;
	char *opts;
	char path[MAXPATHLEN];

	umask(0);
	setbuf(stdout, NULL);
	Myname = *argv++;
	argc--;
	while (argc && **argv == '-') {
		for (opts = &argv[0][1]; *opts; opts++) {
			switch (*opts) {
				case 'h':	/* help */
					usage();
					exit(1);
					break;

				case 't':	/* time */
					Tflag++;
					break;
				
				case 'f':	/* funtionality */
					Fflag++;
					break;
				
				case 'n':	/* No Test Directory create */
					Nflag++;
					break;

				default:
					error("unknown option '%c'", *opts);
					usage();
					exit(1);
			}
		}
		argc--;
		argv++;
	}

	if (argc) {
		count = getparm(*argv, 1, "count");
		argv++;
		argc--;
	}
	if (argc) {
		usage();
		exit(1);
	}

	if (Fflag) {
		Tflag = 0;
		count = 1;
	}

	fprintf(stdout, "%s: lookups across mount point\n", Myname);

	if (!Nflag)
		testdir(NULL);
	else
		mtestdir(NULL);

	if (Tflag) {
		starttime();
	}

	for (ct = 0; ct < count; ct++) {
		if (getcwd(path, sizeof(path)) == NULL) {
			fprintf(stderr, "%s: getcwd failed\n", Myname);
			exit(1);
		}
		if (stat(path, &statb) < 0) {
			error("can't stat %s after getcwd", path);
			exit(1);
		}
	}

	if (Tflag) {
		endtime(&time);
	}
	fprintf(stdout, "\t%d getcwd and stat calls", count * 2);
	if (Tflag) {
		fprintf(stdout, " in %ld.%-2ld seconds",
		    (long)time.tv_sec, (long)time.tv_usec / 10000);
	}
	fprintf(stdout, "\n");
	complete();
}
