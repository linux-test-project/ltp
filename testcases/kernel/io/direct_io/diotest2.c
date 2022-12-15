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
 *      diotest2.c
 *
 * DESCRIPTION
 *	Program tests the combinations of direct and buffered read, write.
 *	The bufsize should be in n*4k size for direct read, write. The offset
 *	value marks the starting position in file from where to start the
 *	read and write. Larger files can be created using the offset parameter.
 *	Test data file can be specified through commandline and is useful
 *	for running test with raw devices as a file.
 *	Test blocks:
 *	[1] Direct read, Buffered write
 *	[2] Direct write, Buffered read
 *	[3] Direct read, Direct write
 *
 * USAGE
 *      diotest2 [-b bufsize] [-o offset] [-i iterations] [-f filename]
 *
 * History
 *	04/22/2002	Narasimha Sharoff nsharoff@us.ibm.com
 *
 * RESTRICTIONS
 *	None
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <errno.h>

#include "diotest_routines.h"

#include "test.h"

char *TCID = "diotest02";	/* Test program identifier.    */
int TST_TOTAL = 3;		/* Total number of test conditions */

#ifdef O_DIRECT

#define	BUFSIZE	4096
#define TRUE	1
#define LEN	30
#define READ_DIRECT 1
#define WRITE_DIRECT 2
#define RDWR_DIRECT 3

int fd1 = -1;
char filename[LEN];
int bufsize = BUFSIZE;

/*
 * runtest: write the data to the file. Read the data from the file and compare.
 *	For each iteration, write data starting at offse+iter*bufsize
 *	location in the file and read from there.
*/
int runtest(int fd_r, int fd_w, int iter, off_t offset, int action)
{
	char *buf1;
	char *buf2;
	int i;

	/* Allocate for buffers */
	if ((buf1 = valloc(bufsize)) == 0) {
		tst_resm(TFAIL, "valloc() buf1 failed: %s", strerror(errno));
		return (-1);
	}
	if ((buf2 = valloc(bufsize)) == 0) {
		tst_resm(TFAIL, "valloc() buf2 failed: %s", strerror(errno));
		return (-1);
	}

	/* seek bufsize*iteration and write. seek and read. verify. */
	for (i = 0; i < iter; i++) {
		fillbuf(buf1, bufsize, i);
		if (lseek(fd_w, offset + iter * bufsize, SEEK_SET) < 0) {
			tst_resm(TFAIL, "lseek before write failed: %s",
				 strerror(errno));
			return (-1);
		}
		if (write(fd_w, buf1, bufsize) < bufsize) {
			tst_resm(TFAIL, "write failed: %s", strerror(errno));
			return (-1);
		}
		if (lseek(fd_r, offset + iter * bufsize, SEEK_SET) < 0) {
			tst_resm(TFAIL, "lseek before read failed: %s",
				 strerror(errno));
			return (-1);
		}
		if (read(fd_r, buf2, bufsize) < bufsize) {
			tst_resm(TFAIL, "read failed: %s", strerror(errno));
			return (-1);
		}
		if (bufcmp(buf1, buf2, bufsize) != 0) {
			tst_resm(TFAIL, "read/write comparision failed");
			return (-1);
		}
	}
	return 0;
}

static void prg_usage(void)
{
	fprintf(stderr,
		"Usage: diotest2 [-b bufsize] [-o offset] [-i iterations] [-f filename]\n");
	exit(1);
}

static void setup(void);
static void cleanup(void);

int main(int argc, char *argv[])
{
	int iter = 100;		/* Iterations. Default 100 */
	off_t offset = 0;	/* Offset. Default 0 */
	int i, action, fd_r, fd_w;
	int fail_count = 0, total = 0, failed = 0;

	/* Options */
	sprintf(filename, "testdata-2.%ld", syscall(__NR_gettid));
	while ((i = getopt(argc, argv, "b:o:i:f:")) != -1) {
		switch (i) {
		case 'b':
			if ((bufsize = atoi(optarg)) <= 0) {
				fprintf(stderr, "bufsize must be > 0\n");
				prg_usage();
			}
			if (bufsize % 4096 != 0) {
				fprintf(stderr,
					"bufsize must be multiple of 4k\n");
				prg_usage();
			}
			break;
		case 'o':
			if ((offset = atoi(optarg)) <= 0) {
				fprintf(stderr, "offset must be > 0\n");
				prg_usage();
			}
			break;
		case 'i':
			if ((iter = atoi(optarg)) <= 0) {
				fprintf(stderr, "iterations must be > 0\n");
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
	if ((fd_w = open(filename, O_WRONLY | O_CREAT, 0666)) < 0)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_WRONLY..) failed", filename);
	if ((fd_r = open(filename, O_DIRECT | O_RDONLY, 0666)) < 0)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_DIRECT|O_RDONLY..) failed", filename);
	if (runtest(fd_r, fd_w, iter, offset, action) < 0) {
		failed = TRUE;
		fail_count++;
		tst_resm(TFAIL, "Read with Direct IO, Write without");
	} else
		tst_resm(TPASS, "Read with Direct IO, Write without");
	close(fd_w);
	close(fd_r);
	unlink(filename);
	total++;

	/* Testblock-2: Write with Direct IO, Read without */
	action = WRITE_DIRECT;
	if ((fd_w = open(filename, O_DIRECT | O_WRONLY | O_CREAT, 0666)) == -1)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_DIRECT|O_WRONLY..) failed", filename);
	if ((fd_r = open(filename, O_RDONLY | O_CREAT, 0666)) == -1)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_RDONLY..) failed", filename);
	if (runtest(fd_r, fd_w, iter, offset, action) < 0) {
		failed = TRUE;
		fail_count++;
		tst_resm(TFAIL, "Write with Direct IO, Read without");
	} else
		tst_resm(TPASS, "Write with Direct IO, Read without");
	close(fd_w);
	close(fd_r);
	unlink(filename);
	total++;

	/* Testblock-3: Read, Write with Direct IO. */
	action = RDWR_DIRECT;
	if ((fd_w = open(filename, O_DIRECT | O_WRONLY | O_CREAT, 0666)) == -1)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_DIRECT|O_WRONLY|O_CREAT, ..) failed",
			 filename);
	if ((fd_r = open(filename, O_DIRECT | O_RDONLY | O_CREAT, 0666)) == -1)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_DIRECT|O_RDONLY|O_CREAT, ..) failed",
			 filename);
	if (runtest(fd_r, fd_w, iter, offset, action) < 0) {
		failed = TRUE;
		fail_count++;
		tst_resm(TFAIL, "Read, Write with Direct IO");
	} else
		tst_resm(TPASS, "Read, Write with Direct IO");
	close(fd_w);
	close(fd_r);
	unlink(filename);
	total++;

	if (failed)
		tst_resm(TINFO, "%d/%d testblocks failed", fail_count, total);
	else
		tst_resm(TINFO, "%d testblocks %d iterations completed",
			 total, iter);
	cleanup();

	tst_exit();

}

static void setup(void)
{
	tst_tmpdir();

	if ((fd1 = open(filename, O_CREAT | O_EXCL, 0600)) == -1)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_CREAT|O_EXCL, ..) failed", filename);
	close(fd1);

	/* Test for filesystem support of O_DIRECT */
	if ((fd1 = open(filename, O_DIRECT, 0600)) == -1)
		tst_brkm(TCONF | TERRNO, cleanup,
			 "open(%s, O_DIRECT, ..) failed", filename);
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
	tst_brkm(TCONF, NULL, "O_DIRECT is not defined.");
}
#endif /* O_DIRECT */
