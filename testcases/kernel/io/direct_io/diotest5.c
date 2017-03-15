/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *      diotest5.c
 *
 * DESCRIPTION
 *	The programs test buffered and direct IO with vector arrays using
 *	readv() and writev() calls.
 *	Test blocks
 *	[1] Direct readv, Buffered writev
 *	[2] Direct writev, Buffered readv
 *	[3] Direct readv, Direct writev
 *	The bufsize should be in n*4k size for direct readv, writev. The offset
 *	value marks the starting position in file from where to start the
 *	write and read. (Using larger offset, larger files can be tested).
 *	The nvector gives vector array size.  Test data file can be
 * 	specified through commandline and is useful for running test with
 * 	raw devices as a file.
 *
 * USAGE
 *      diotest5 [-b bufsize] [-o offset] [-i iterations]
 *			[-v nvector] [-f filename]
 *
 * History
 *	04/29/2002	Narasimha Sharoff nsharoff@us.ibm.com
 *
 * RESTRICTIONS
 *	None
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <errno.h>

#include "diotest_routines.h"

#include "test.h"

char *TCID = "diotest05";	/* Test program identifier.    */
int TST_TOTAL = 3;		/* Total number of test conditions */

#ifdef O_DIRECT

#define	BUFSIZE	4096
#define TRUE 1
#define LEN 30
#define	READ_DIRECT 1
#define	WRITE_DIRECT 2
#define	RDWR_DIRECT 3

static int bufsize = BUFSIZE;	/* Buffer size. Default 4k */
static int iter = 20;		/* Iterations. Default 20 */
static int nvector = 20;	/* Vector array. Default 20 */
static off64_t offset = 0;	/* Start offset. Default 0 */
static char filename[LEN];	/* Test data file */
static int fd1 = -1;
/*
 * runtest: Write the data in vector array to the file. Read the data
 *	from the file into another vectory array and verify. Repeat the test.
*/
int runtest(int fd_r, int fd_w, int iter, off64_t offset, int action)
{
	int i;
	struct iovec *iov1, *iov2, *iovp;

	/* Allocate for buffers and data pointers */
	if ((iov1 =
	     (struct iovec *)valloc(sizeof(struct iovec) * nvector)) == NULL) {
		tst_resm(TFAIL, "valloc() buf1 failed: %s", strerror(errno));
		return (-1);
	}
	if ((iov2 =
	     (struct iovec *)valloc(sizeof(struct iovec) * nvector)) == NULL) {
		tst_resm(TFAIL, "valloc buf2 failed: %s", strerror(errno));
		return (-1);
	}
	for (i = 0, iovp = iov1; i < nvector; iovp++, i++) {
		if ((iovp->iov_base = valloc(bufsize)) == NULL) {
			tst_resm(TFAIL, "valloc for iovp->iov_base: %s",
				 strerror(errno));
			return (-1);
		}
		iovp->iov_len = bufsize;
	}
	for (i = 0, iovp = iov2; i < nvector; iovp++, i++) {
		if ((iovp->iov_base = valloc(bufsize)) == NULL) {
			tst_resm(TFAIL, "valloc, iov2 for iovp->iov_base: %s",
				 strerror(errno));
			return (-1);
		}
		iovp->iov_len = bufsize;
	}

	/* Test */
	for (i = 0; i < iter; i++) {
		vfillbuf(iov1, nvector, i);
		vfillbuf(iov2, nvector, i + 1);
		if (lseek(fd_w, offset, SEEK_SET) < 0) {
			tst_resm(TFAIL, "lseek before writev failed: %s",
				 strerror(errno));
			return (-1);
		}
		if (writev(fd_w, iov1, nvector) < 0) {
			tst_resm(TFAIL, "writev failed: %s", strerror(errno));
			return (-1);
		}
		if (lseek(fd_r, offset, SEEK_SET) < 0) {
			tst_resm(TFAIL, "lseek before readv failed: %s",
				 strerror(errno));
			return (-1);
		}
		if (readv(fd_r, iov2, nvector) < 0) {
			tst_resm(TFAIL, "readv failed: %s", strerror(errno));
			return (-1);
		}
		if (vbufcmp(iov1, iov2, nvector) != 0) {
			tst_resm(TFAIL, "readv/writev comparision failed");
			return (-1);
		}
	}

	/* Cleanup */
	for (i = 0, iovp = iov1; i < nvector; iovp++, i++) {
		free(iovp->iov_base);
	}
	for (i = 0, iovp = iov2; i < nvector; iovp++, i++) {
		free(iovp->iov_base);
	}
	free(iov1);
	free(iov2);
	return 0;
}

static void prg_usage(void)
{
	fprintf(stderr,
		"Usage: diotest5 [-b bufsize] [-o offset] [ -i iteration] [ -v nvector] [-f filename]\n");
	exit(1);
}

static void setup(void);
static void cleanup(void);

int main(int argc, char *argv[])
{
	int i, action, fd_r, fd_w;
	int fail_count = 0, total = 0, failed = 0;

	/* Options */
	sprintf(filename, "testdata-5.%ld", syscall(__NR_gettid));
	while ((i = getopt(argc, argv, "b:o:i:v:f:")) != -1) {
		switch (i) {
		case 'b':
			if ((bufsize = atoi(optarg)) <= 0) {
				fprintf(stderr, "bufsize must be > 0");
				prg_usage();
			}
			if (bufsize % 4096 != 0) {
				fprintf(stderr, "bufsize must be > 0");
				prg_usage();
			}
			break;
		case 'o':
			if ((offset = atoll(optarg)) <= 0) {
				fprintf(stderr, "offset must be > 0");
				prg_usage();
			}
			break;
		case 'i':
			if ((iter = atoi(optarg)) <= 0) {
				fprintf(stderr, "iterations must be > 0");
				prg_usage();
			}
			break;
		case 'v':
			if ((nvector = atoi(optarg)) <= 0) {
				fprintf(stderr, "vector array must be > 0");
				prg_usage();
			}
			break;
		case 'f':
			strcpy(filename, optarg);
			break;
		default:
			prg_usage();
		}
	}

	setup();

	/* Testblock-1: Read with Direct IO, Write without */
	action = READ_DIRECT;
	if ((fd_w = open(filename, O_WRONLY | O_CREAT, 0666)) < 0) {
		tst_brkm(TBROK, cleanup, "fd_w open failed for %s: %s",
			 filename, strerror(errno));
	}
	if ((fd_r = open64(filename, O_DIRECT | O_RDONLY | O_CREAT, 0666)) < 0) {
		tst_brkm(TBROK, cleanup, "fd_r open failed for %s: %s",
			 filename, strerror(errno));
	}
	if (runtest(fd_r, fd_w, iter, offset, action) < 0) {
		failed = TRUE;
		fail_count++;
		tst_resm(TFAIL, "Read with Direct IO, Write without");
	} else
		tst_resm(TPASS, "Read with Direct IO, Write without");

	unlink(filename);
	close(fd_r);
	close(fd_w);
	total++;

	/* Testblock-2: Write with Direct IO, Read without */
	action = WRITE_DIRECT;
	if ((fd_w = open(filename, O_DIRECT | O_WRONLY | O_CREAT, 0666)) < 0) {
		tst_brkm(TBROK, cleanup, "fd_w open failed for %s: %s",
			 filename, strerror(errno));
	}
	if ((fd_r = open64(filename, O_RDONLY | O_CREAT, 0666)) < 0) {
		tst_brkm(TBROK, cleanup, "fd_r open failed for %s: %s",
			 filename, strerror(errno));
	}
	if (runtest(fd_r, fd_w, iter, offset, action) < 0) {
		failed = TRUE;
		fail_count++;
		tst_resm(TFAIL, "Write with Direct IO, Read without");
	} else
		tst_resm(TPASS, "Write with Direct IO, Read without");
	unlink(filename);
	close(fd_r);
	close(fd_w);
	total++;

	/* Testblock-3: Read, Write with Direct IO */
	action = RDWR_DIRECT;
	if ((fd_w = open(filename, O_DIRECT | O_WRONLY | O_CREAT, 0666)) < 0) {
		tst_brkm(TBROK, cleanup, "fd_w open failed for %s: %s",
			 filename, strerror(errno));
	}
	if ((fd_r = open64(filename, O_DIRECT | O_RDONLY | O_CREAT, 0666)) < 0) {
		tst_brkm(TBROK, cleanup, "fd_r open failed for %s: %s",
			 filename, strerror(errno));
	}
	if (runtest(fd_r, fd_w, iter, offset, action) < 0) {
		failed = TRUE;
		fail_count++;
		tst_resm(TFAIL, "Read, Write with Direct IO");
	} else
		tst_resm(TPASS, "Read, Write with Direct IO");
	unlink(filename);
	close(fd_r);
	close(fd_w);
	total++;

	if (failed)
		tst_resm(TINFO, "%d/%d testblocks failed", fail_count, total);
	else
		tst_resm(TINFO,
			 "%d testblocks %d iterations with %d vector array completed",
			 total, iter, nvector);

	cleanup();

	tst_exit();
}

static void setup(void)
{
	tst_tmpdir();

	if ((fd1 = open(filename, O_CREAT | O_EXCL, 0600)) < 0) {
		tst_brkm(TBROK, cleanup, "Couldn't create test file %s: %s",
			 filename, strerror(errno));
	}
	close(fd1);

	/* Test for filesystem support of O_DIRECT */
	if ((fd1 = open(filename, O_DIRECT, 0600)) < 0) {
		tst_brkm(TCONF, cleanup,
			 "O_DIRECT is not supported by this filesystem. %s",
			 strerror(errno));
	}
	close(fd1);
}

static void cleanup(void)
{
	if (fd1 != -1)
		unlink(filename);

	tst_rmdir();

}
#else /* O_DIRECT */

int main()
{

	tst_resm(TCONF, "O_DIRECT is not defined.");
	return 0;
}
#endif /* O_DIRECT */
