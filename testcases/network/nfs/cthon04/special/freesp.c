/*
 * @(#)freesp.c	1.1	98/12/19 Connectathon Testsuite
 */


/*
 * Write a file and truncate it using fcntl(...F_FREESP).  Make sure the
 * new size is correctly indicated.
 *
 * Usage: freesp [filename]
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../tests.h"

/*
 * Size of a full and partial buffer.  The idea is to write a few copies of
 * the full buffer and then write a partial buffer, so that the resulting
 * file is not an even number of pages long.  This may not be significant,
 * but this is the way test cases have worked that provoked failures in the
 * past.
 */

#define	BUFSIZE		8192
#define PARTIAL_BUF	42
#define NUMBUFS		3		/* full buffers to write */

static char buf[BUFSIZE];

static char *filename = "freesp.dat";

static void verify_size ARGS_((int, off_t));

#ifndef F_FREESP

/*ARGUSED*/
int
main(argc, argv)
	int argc;
	char **argv;
{
	printf("fcntl(...F_FREESP...) not available on this platform.\n");
	exit(0);
}

#else /* F_FREESP */

int
main(argc, argv)
	int argc;
	char **argv;
{
	int fd;
	int i;
	flock_t clear;

	memset(buf, '%', BUFSIZE);

	if (argc > 1)
		filename = argv[1];

	fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (fd < 0) {
		fprintf(stderr, "can't open %s: %s\n", filename,
			strerror(errno));
		exit(1);
	}

	/*
	 * Put some bits into the file.
	 */

	for (i = 0; i < NUMBUFS; i++) {
		if (write(fd, buf, BUFSIZE) < 0) {
			fprintf(stderr, "can't write %s: %s\n",
				filename, strerror(errno));
			exit(1);
		}
	}
	verify_size(fd, NUMBUFS * BUFSIZE);

	if (write(fd, buf, PARTIAL_BUF) < 0) {
		fprintf(stderr, "can't write %s: %s\n",
			filename, strerror(errno));
		exit(1);
	}

	/*
	 * Rewind and truncate the file.
	 */

	if (lseek(fd, 0, SEEK_SET) < 0) {
		fprintf(stderr, "can't seek to 0: %s\n",
			strerror(errno));
		exit(1);
	}
	clear.l_start = 0;
	clear.l_whence = SEEK_SET;
	clear.l_len = 0;		/* entire file */
	if (fcntl(fd, F_FREESP, &clear) < 0) {
		fprintf(stderr, "can't clear %s: %s\n",
			filename, strerror(errno));
		exit(1);
	}

	/*
	 * Recheck the size.
	 */

	verify_size(fd, 0);
	
	close(fd);
	unlink(filename);
	exit(0);
}

/*
 * Verify that the file size is "expected" bytes.  Complain and exit if
 * this isn't the case.
 * Side effects: fd's offset is moved to the end of the file.
 */

static void
verify_size(fd, expected)
	int fd;
	off_t expected;
{
	off_t actual;

	actual = lseek(fd, 0, SEEK_END);
	if (actual < 0) {
		fprintf(stderr, "can't get size: %s\n",
			strerror(errno));
		exit(1);
	}
	if (actual != expected) {
		fprintf(stderr, "expected size: %ld, got: %ld\n",
			(long)expected, (long)actual);
		exit(1);
	}
}

#endif /* F_FREESP */
