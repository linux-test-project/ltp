/*
 *	@(#)test5b.c	1.7	03/12/01 Connectathon Testsuite
 *	1.3 Lachman ONC Test Suite source
 *
 * Test read - will read a file of specified size, contents not looked at
 *
 * Uses the following important system calls against the server:
 *
 *	chdir()
 *	mkdir()		(for initial directory creation if not -m)
 *	open()
 *	read()
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
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#ifdef DOSorWIN32
#include <time.h>
#else
#include <sys/time.h>
#endif
#ifdef MMAP
#include <sys/mman.h>
#endif

#include "../tests.h"

#ifndef MIN
#define MIN(a, b)	((a) < (b) ? (a) : (b))
#endif

#define	BUFSZ	8192
#define	DSIZE	1048576

static int Tflag = 0;	/* print timing */
static int Fflag = 0;	/* test function only;  set count to 1, negate -t */
static int Nflag = 0;	/* Suppress directory operations */

static void
usage()
{
	fprintf(stdout, "usage: %s [-htfn] [size count fname]\n", Myname);
	fprintf(stdout, "  Flags:  h    Help - print this usage info\n");
	fprintf(stdout, "          t    Print execution time statistics\n");
	fprintf(stdout, "          f    Test function only (negate -t)\n");
	fprintf(stdout, "          n    Suppress test directory create operations\n");
}

main(argc, argv)
	int argc;
	char *argv[];
{
	int count = DCOUNT;	/* times to do each file */
	int ct;
	off_t size = DSIZE;
	off_t si;
	int fd;
	off_t bytes = 0;
	int roflags;			/* open read-only flags */
	char *bigfile = "bigfile";
	struct timeval time;
	char *opts;
	char buf[BUFSZ];
	double etime;
#ifdef MMAP
	caddr_t maddr;
#endif

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
		size = getparm(*argv, 1, "size");
		if (size <= 0) {
			usage();
			exit(1);
		}
		argv++;
		argc--;
	}
	if (argc) {
		count = getparm(*argv, 1, "count");
		if (count <= 0) {
			usage();
			exit(1);
		}
		argv++;
		argc--;
	}
	if (argc) {
		bigfile = *argv;
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

	roflags = O_RDONLY;
#ifdef DOSorWIN32
	roflags |= O_BINARY;
#endif

	fprintf(stdout, "%s: read\n", Myname);

	mtestdir(NULL);

	if (Tflag) {
		starttime();
	}

	for (ct = 0; ct < count; ct++) {
		if ((fd = open(bigfile, roflags)) < 0) {
			error("can't open '%s'", bigfile);
			exit(1);
		}
#ifdef MMAP
		maddr = mmap((caddr_t)0, (size_t)size, PROT_READ,
				MAP_PRIVATE, fd, (off_t)0);
		if (maddr == MAP_FAILED) {
			error("can't mmap '%s'", bigfile);
			exit(1);
		}
		if (msync(maddr, (size_t)size, MS_INVALIDATE) < 0) {
			error("can't invalidate pages for '%s'", bigfile);
			exit(1);
		}
		if (munmap(maddr, (size_t)size) < 0) {
			error("can't munmap '%s'", bigfile);
			exit(1);
		}
#endif
		for (si = size; si > 0; si -= bytes) {
			bytes = MIN(BUFSZ, si);
			if (read(fd, buf, bytes) != bytes) {
				error("'%s' read failed", bigfile);
				exit(1);
			}
		}
		close(fd);
	}

	if (Tflag) {
		endtime(&time);
	}

	fprintf(stdout, "\tread %ld byte file %d times", (long)size, count);

	if (Tflag) {
		etime = (double)time.tv_sec + (double)time.tv_usec / 1000000.0;
		if (etime != 0.0) {
			fprintf(stdout, " in %ld.%-2ld seconds (%ld bytes/sec)",
				(long)time.tv_sec, (long)time.tv_usec / 10000,
				(long)((double)size * ((double)count / etime)));
		} else {
			fprintf(stdout, " in %ld.%-2ld seconds (> %ld bytes/sec)",
				(long)time.tv_sec, (long)time.tv_usec / 10000,
				(long)size * count);
		}
	}
	fprintf(stdout, "\n");

	if (unlink(bigfile) < 0) {
		error("can't unlink '%s'", bigfile);
		exit(1);
	}
	complete();
}
