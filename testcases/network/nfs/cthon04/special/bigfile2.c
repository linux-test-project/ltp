/*
 * @(#)bigfile2.c	1.6	2003/12/30 Connectathon Testsuite
 */

/*
 * Write a holey file that walks around a couple file size edges: 2GB (31
 * bits) and 4GB (32 bits).  Note that this test only makes sense if the
 * platform supports files with offsets bigger than 31 bits (32-bit signed
 * integer).
 */

/*
 * Explicitly request Large File interfaces.  If we omit this, some
 * implementations (e.g., Red Hat Linux 6.[12], HPUX 11.0) define
 * _LFS64_LARGEFILE but don't define all the associated types.
 */
#define	_LARGEFILE64_SOURCE	1

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../tests.h"

#define	HIGH_WORD(n) ((unsigned int)(((unsigned int)((n) >> 32)) & 0xffffffff))
#define	LOW_WORD(n) ((unsigned int)((unsigned int)(n) & 0xffffffff))

static char usage[] = "usage: bigfile2 filename";

static char *filename;

/*
 * If the native routines support 64-bit offsets, then just use them.
 * Otherwise, use the Large File Summit transitional routines.
 */

#ifdef NATIVE64
#define	LSEEK	lseek
#define	FSTAT	fstat
typedef off_t offset64;
typedef struct stat stat_info;
#else
#ifdef _LFS64_LARGEFILE
#define	LSEEK	lseek64
#define	FSTAT	fstat64
typedef off64_t offset64;
typedef struct stat64 stat_info;
#endif
#endif

#if defined(_LFS64_LARGEFILE) || defined(NATIVE64)
static void check_around ARGS_((int fd, offset64 where));
#endif

#if !defined(_LFS64_LARGEFILE) && !defined(NATIVE64)

int
main(argc, argv)
	int argc;
	char **argv;
{
	fprintf(stderr, "Skipping this test:\n");
	fprintf(stderr,
		"The platform doesn't appear to support 64-bit offsets.\n");
	exit(0);
}

#else /* 64-bit support */

int
main(argc, argv)
	int argc;
	char **argv;
{
	int fd;
	int oflags;

	/* O_SYNC is to ensure detection of problems when writing */
	oflags = O_RDWR | O_CREAT | O_TRUNC | O_SYNC;

	if (argc != 2) {
		fprintf(stderr, "%s\n", usage);
		exit(1);
	}
	filename = argv[1];

#ifdef _LFS64_LARGEFILE
	oflags |= O_LARGEFILE;
#endif
	fd = open(filename, oflags, 0666);
	if (fd < 0) {
		fprintf(stderr, "can't open %s: %s\n",
			filename, strerror(errno));
		exit(1);
	}

	check_around(fd, ((offset64)0x7fffffff) + 1);

	if (ftruncate(fd, 0) < 0) {
		perror("can't truncate");
		exit(1);
	}

	check_around(fd, ((offset64)(0xffffffffU)) + 1);

	unlink(filename);
	exit(0);
}

/*
 * Write 5 bytes, one at a time, starting at "where"-2.  For each byte,
 * verify that the file length is what we expect and that we can read the
 * byte back again.
 */

static void
check_around(fd, where)
	int fd;
	offset64 where;
{
	char buf;
	int i;
	offset64 start = where - 2;
	int numbytes = 5;
	stat_info statbuf;
	char basechar = '0';

	if (LSEEK(fd, start, SEEK_SET) < 0) {
		fprintf(stderr, "can't do initial seek to 0x%x%08x: %s\n",
			HIGH_WORD(start), LOW_WORD(start),
			strerror(errno));
		exit(1);
	}

	for (i = 0; i < numbytes; i++) {
		buf = basechar + i;
		if (write(fd, &buf, 1) < 0) {
			fprintf(stderr, "can't write at 0x%x%08x: %s\n",
				HIGH_WORD(start + i),
				LOW_WORD(start + i),
				strerror(errno));
			exit(1);
		}
		if (FSTAT(fd, &statbuf) < 0) {
			fprintf(stderr, "can't stat %s: %s\n",
				filename, strerror(errno));
			exit(1);
		}
		if (statbuf.st_size != start + i + 1) {
			fprintf(stderr,
				"expected size 0x%x%08x, got 0x%x%08x\n",
				HIGH_WORD(start + i + 1),
				LOW_WORD(start + i + 1),
				HIGH_WORD(statbuf.st_size),
				LOW_WORD(statbuf.st_size));
			exit(1);
		}
	}

	for (i = 0; i < numbytes; i++) {
		if (LSEEK(fd, start + i, SEEK_SET) < 0) {
			fprintf(stderr,
				"can't seek to 0x%x%08x to reread file: %s\n",
				HIGH_WORD(start + i),
				LOW_WORD(start + i),
				strerror(errno));
			exit(1);
		}
		if (read(fd, &buf, 1) < 0) {
			fprintf(stderr, "can't read at offset 0x%x%08x: %s\n",
				HIGH_WORD(start + i),
				LOW_WORD(start + i),
				strerror(errno));
			exit(1);
		}
		if (buf != basechar + i) {
			fprintf(stderr, "expected `%c', got `%c' ",
				basechar + i, buf);
			fprintf(stderr, "at 0x%x%08x\n",
				HIGH_WORD(start + i),
				LOW_WORD(start + i));
			exit(1);
		}
	}
}

#endif /* 64-bit support */
