/*
 *	@(#)test8.c	1.7	2001/08/25 Connectathon Testsuite
 *	1.4 Lachman ONC Test Suite source
 *
 * Test symlink, readlink
 *
 * Uses the following important system calls against the server:
 *
 *	chdir()
 *	mkdir()		(for initial directory creation if not -m)
 *	creat()
 *	symlink()
 *	readlink()
 *	lstat()
 *	unlink()
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
#include <string.h>
#include <sys/types.h>
#ifdef DOSorWIN32
#include <time.h>
#else
#include <sys/time.h>
#endif

#include "../tests.h"

static int Tflag = 0;	/* print timing */
static int Fflag = 0;	/* test function only;  set count to 1, negate -t */
static int Nflag = 0;	/* Suppress directory operations */

#define SNAME "/this/is/a/symlink"	/* symlink prefix */

static void
usage()
{
	fprintf(stdout, "usage: %s [-htfn] [files count fname sname]\n", Myname);
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
	int count = 20;	/* times to do each file */
	int ct;
	char *fname = FNAME;
	char *sname = SNAME;
	struct timeval time;
	char str[MAXPATHLEN];
	char new[MAXPATHLEN];
	char buf[MAXPATHLEN];
	int ret;
	struct stat statb;
	char *opts;
	int oerrno;

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
		sname = *argv;
		argv++;
		argc--;
	}
	if (argc) {
		usage();
		exit(1);
	}

#ifndef S_IFLNK
	fprintf(stdout, "\
%s: symlink and readlink not supported on this client\n", Myname);
#else /* S_IFLNK */
	if (Fflag) {
		Tflag = 0;
		count = 1;
	}

	if (!Nflag)
		testdir(NULL);
	else
		mtestdir(NULL);

	fprintf(stdout, "%s: symlink and readlink\n", Myname);

	if (Tflag) {
		starttime();
	}

	for (ct = 0; ct < count; ct++) {
		for (fi = 0; fi < files; fi++) {
			sprintf(str, "%s%d", fname, fi);
			sprintf(new, "%s%d", sname, fi);
			if (symlink(new, str) < 0) {
				oerrno = errno;
				error("can't make symlink %s", str);
				errno = oerrno;
				if (errno == EOPNOTSUPP)
					complete();
				else
					exit(1);
			}
                        if (lstat(str, &statb) < 0) {
                                error("can't stat %s after symlink", str);
                                exit(1);
                        }
			if ((statb.st_mode & S_IFMT) != S_IFLNK) {
				error("mode of %s not symlink");
				exit(1);
			}
			if ((ret = readlink(str, buf, MAXPATHLEN))
			     != strlen(new)) {
				error("readlink %s ret %d, expect %d",
					str, ret, strlen(new));
				exit(1);
			}
			if (strncmp(new, buf, ret) != 0) {
				error("readlink %s returned bad linkname",
					str);
				exit(1);
			}
			if (unlink(str) < 0) {
				error("can't unlink %s", str);
				exit(1);
			}
		}
	}

	if (Tflag) {
		endtime(&time);
	}
	fprintf(stdout, "\t%d symlinks and readlinks on %d files",
		files * count * 2, files);
	if (Tflag) {
		fprintf(stdout, " in %ld.%-2ld seconds",
		    (long)time.tv_sec, (long)time.tv_usec / 10000);
	}
	fprintf(stdout, "\n");
#endif /* S_IFLNK */
	complete();
}
