/*
 *	@(#)test7a.c	1.6	99/08/29 Connectathon Testsuite
 *	1.3 Lachman ONC Test Suite source
 *
 * Test rename
 *
 * Uses the following important system calls against the server:
 *
 *	chdir()
 *	mkdir()		(for initial directory creation if not -m)
 *	creat()
 *	stat()
 *	rename()
 *	unlink()
 */

#if defined (DOS) || defined (WIN32)
/* If Dos, Windows or Win32 */
#define DOSorWIN32
#endif

#ifndef DOSorWIN32
#include <sys/param.h>
#endif

#include <sys/types.h>
#ifdef DOSorWIN32
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#include "../tests.h"

static int Tflag = 0;	/* print timing */
static int Fflag = 0;	/* test function only;  set count to 1, negate -t */
static int Nflag = 0;	/* Suppress directory operations */

#define NNAME "newfile."	/* new filename for rename */

static void
usage()
{
	fprintf(stdout, "usage: %s [-htfn] [files count fname nname]\n", Myname);
	fprintf(stdout, "  Flags:  h    Help - print this usage info\n");
	fprintf(stdout, "          t    Print execution time statistics\n");
	fprintf(stdout, "          f    Test function only (negate -t)\n");
	fprintf(stdout, "          n    Suppress test directory create operations\n");
}

main(argc, argv)
	int argc;
	char *argv[];
{
	int files = 10;		/* number of files in each dir */
	int fi;
	int count = 10;	/* times to do each file */
	int ct;
	int totfiles = 0;
	int totdirs = 0;
	char *fname = FNAME;
	char *nname = NNAME;
	struct timeval time;
	char str[MAXPATHLEN];
	char new[MAXPATHLEN];
	struct stat statb;
	char *opts;

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
		files = getparm(*argv, 1, "files");
		argv++;
		argc--;
	}
	if (argc) {
		count = getparm(*argv, 1, "count");
		argv++;
		argc--;
	}
	if (argc) {
		fname = *argv;
		argv++;
		argc--;
	}
	if (argc) {
		nname = *argv;
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

	fprintf(stdout, "%s: rename\n", Myname);

	if (!Nflag)
		testdir(NULL);
	else
		mtestdir(NULL);

	dirtree(1, files, 0, fname, DNAME, &totfiles, &totdirs);

	if (Tflag) {
		starttime();
	}

	for (ct = 0; ct < count; ct++) {
		for (fi = 0; fi < files; fi++) {
			sprintf(str, "%s%d", fname, fi);
			sprintf(new, "%s%d", nname, fi);
			if (rename(str, new) < 0) {
				error("can't rename %s to %s", str, new);
				exit(1);
			}
			if (stat(str, &statb) == 0) {
				error("%s exists after rename", str);
				exit(1);
			}
			if (stat(new, &statb) < 0) {
				error("can't stat %s after rename", new);
				exit(1);
			}
			if (rename(new, str) < 0) {
				error("can't rename %s to %s", new, str);
				exit(1);
			}
			if (stat(new, &statb) == 0) {
				error("%s exists after rename", new);
				exit(1);
			}
			if (stat(str, &statb) < 0) {
				error("can't stat %s after rename", str);
				exit(1);
			}
		}
	}

	if (Tflag) {
		endtime(&time);
	}
	fprintf(stdout, "\t%d renames on %d files",
		files * count * 2, files);
	if (Tflag) {
		fprintf(stdout, " in %ld.%-2ld seconds",
		    (long)time.tv_sec, (long)time.tv_usec / 10000);
	}
	fprintf(stdout, "\n");

	/* Cleanup files left around */
	rmdirtree(1, files, 0, fname, DNAME, &totfiles, &totdirs, 1);

	complete();
}
