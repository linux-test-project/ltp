/*
 *	@(#)test6.c	1.5	99/08/29 Connectathon Testsuite
 *	1.4 Lachman ONC Test Suite source
 *
 * Test readdir
 *
 * Uses the following important system/library calls against the server:
 *
 *	chdir()
 *	mkdir()		(for initial directory creation if not -m)
 *	creat()
 *	unlink()
 *	opendir(), rewinddir(), readdir(), closedir()
 */

#if defined (DOS) || defined (WIN32)
/* If Dos, Windows or Win32 */
#define DOSorWIN32
#endif

#ifndef DOSorWIN32
#include <sys/param.h>
#include <unistd.h>
#include <string.h>
#ifdef use_directs
#include <sys/dir.h>
#else
#include <dirent.h>
#endif
#endif /* DOSorWIN32 */

#include <sys/types.h>
#include <sys/stat.h>
#ifdef DOSorWIN32
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#include "../tests.h"

static int Tflag = 0;	/* print timing */
static int Fflag = 0;	/* test function only;  set count to 1, negate -t */
static int Nflag = 0;	/* Suppress directory operations */
static int Iflag = 0;	/* Ignore non-test files dir entries */

#define MAXFILES 512		/* maximum files allowed for this test */
#define BITMOD	 8		/* bits per u_char */
static unsigned char bitmap[MAXFILES / BITMOD];

#define BIT(x)    (bitmap[(x) / BITMOD] &   (1 << ((x) % BITMOD)) )
#define SETBIT(x) (bitmap[(x) / BITMOD] |=  (1 << ((x) % BITMOD)) )
#define CLRBIT(x) (bitmap[(x) / BITMOD] &= ~(1 << ((x) % BITMOD)) )

static void
usage()
{
	fprintf(stdout, "usage: %s [-htfni] [files count fname]\n", Myname);
	fprintf(stdout, "  Flags:  h    Help - print this usage info\n");
	fprintf(stdout, "          t    Print execution time statistics\n");
	fprintf(stdout, "          f    Test function only (negate -t)\n");
	fprintf(stdout, "          n    Suppress test directory create operations\n");
	fprintf(stdout, "          i    Ignore non-test files dir entries\n");
}

main(argc, argv)
	int argc;
	char *argv[];
{
#ifdef use_directs
	struct direct *dp;
#else
	struct dirent *dp;
#endif
	char *fname = FNAME;
	int files = 200;	/* number of files in each dir */
	int fi;
	int count = 200;	/* times to read dir */
	int ct;
	int entries = 0;
	int totfiles = 0;
	int totdirs = 0;
	DIR *dir;
	struct timeval time;
	char *p, str[MAXPATHLEN];
	char *opts;
	int err, i, dot, dotdot;
	int nmoffset;

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

				case 'i':	/* ignore spurious files */
					Iflag++;
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
		usage();
		exit(1);
	}

	nmoffset = strlen(fname);

	if (Fflag) {
		Tflag = 0;
		count = 1;
	}

	if (count > files) {
		error("count (%d) can't be greater than files (%d)",
			count, files);
		exit(1);
	}

	if (files > MAXFILES) {
		error("too many files requested (max is %d)", MAXFILES);
		exit(1);
	}

	fprintf(stdout, "%s: readdir\n", Myname);

	if (!Nflag)
		testdir(NULL);
	else
		mtestdir(NULL);

	dirtree(1, files, 0, fname, DNAME, &totfiles, &totdirs);

	if (Tflag) {
		starttime();
	}

	if ((dir = opendir(".")) == NULL) {
		error("can't opendir %s", ".");
		exit(1);
	}

	for (ct = 0; ct < count; ct++) {
		rewinddir(dir);
		dot = 0;
		dotdot = 0;
		err = 0;
		for (i = 0; i < sizeof(bitmap); i++)
			bitmap[i] = 0;
		while ((dp = readdir(dir)) != NULL) {
			entries++;
			if (strcmp(".", dp->d_name) == 0) {
				if (dot) {
					/* already read dot */
					error("'.' dir entry read twice");
					exit(1);
				}
				dot++;
				continue;
			} else if (strcmp("..", dp->d_name) == 0) {
				if (dotdot) {
					/* already read dotdot */
					error("'..' dir entry read twice");
					exit(1);
				}
				dotdot++;
				continue;
			}

			/*
			 * at this point, should have entry of the form
			 *  fname%d
			 */
			/* If we don't have our own directory, ignore
			   such errors (if Iflag set). */
			if (strncmp(dp->d_name, fname, nmoffset)) {
				if (Iflag)
					continue;
				else {
					error("unexpected dir entry '%s'",
						dp->d_name);
					exit(1);
				}
			}

			/* get ptr to numeric part of name */
			p = dp->d_name + nmoffset;
			fi = atoi(p);
			if (fi < 0 || fi >= MAXFILES) {
				error("unexpected dir entry '%s'",
					dp->d_name);
				exit(1);
			}
			if (BIT(fi)) {
				error("duplicate '%s' dir entry read",
					dp->d_name);
				err++;
			} else
				SETBIT(fi);
		}	/* end readdir loop */
		if (!dot) {
			error("didn't read '.' dir entry, pass %d", ct);
			err++;
		}
		if (!dotdot) {
			error("didn't read '..' dir entry, pass %d", ct);
			err++;
		}
		for (fi = 0; fi < ct; fi++) {
			if (BIT(fi)) {
				sprintf(str, "%s%d", fname, fi);
				error("unlinked '%s' dir entry read pass %d",
					str, ct);
				err++;
			}
		}
		for (fi = ct; fi < files; fi++) {
			if (!BIT(fi)) {
				sprintf(str, "%s%d", fname, fi);
				error("\
didn't read expected '%s' dir entry, pass %d", str, ct);
				err++;
			}
		}
		if (err) {
			error("Test failed with %d errors", err);
			exit(1);
		}
		sprintf(str, "%s%d", fname, ct);
		if (unlink(str) < 0) {
			error("can't unlink %s", str);
			exit(1);
		}
	}

	closedir(dir);

	if (Tflag) {
		endtime(&time);
	}
	fprintf(stdout, "\t%d entries read, %d files",
		entries, files);
	if (Tflag) {
		fprintf(stdout, " in %ld.%-2ld seconds",
		    (long)time.tv_sec, (long)time.tv_usec / 10000);
	}
	fprintf(stdout, "\n");
	rmdirtree(1, files, 0, fname, DNAME, &totfiles, &totdirs, 1);
	complete();
}
