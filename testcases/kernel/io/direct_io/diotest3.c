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
 *      diotest3.c
 *
 * DESCRIPTION
 *	Fork given number of children. Each child opens the same file, but
 *	uses its own file descriptiors. The child does writes and reads from
 *	its segment in the file. The segment to which the child writes is
 *	determined by childnumber * bufsize. There is no need to use any locks.
 *	Program tests the combinations of buffered/buffered read(), write()
 *	calls.
 *	Test blocks:
 *	[1] Direct Read, Buffered write
 *	[2] Direct Write, Buffered read
 *	[3] Direct Read, Direct Write
 *
 * USAGE
 *	diotest3 [-b bufsize] [-o offset] [-n numchild]
 *			[-i iterations [-f filename]
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
#include <string.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <errno.h>

#include "diotest_routines.h"

#include "test.h"

char *TCID = "diotest03";	/* Test program identifier.    */
int TST_TOTAL = 3;		/* Total number of test conditions */

#ifdef O_DIRECT

#define	BUFSIZE	4096
#define TRUE 1
#define LEN 30
#define	READ_DIRECT 1
#define	WRITE_DIRECT 2
#define	RDWR_DIRECT 3

static int iter = 100;		/* Iterations. Default 100 */
static int bufsize = BUFSIZE;	/* Buffersize. Default 4k */
static int offset = 0;		/* Offset. Default 0 */
static char filename[LEN];

static void prg_usage(void)
{
	fprintf(stderr,
		"Usage: diotest3 [-b bufsize] [-o offset] [-n numchild] [-i iterations] [-f filename]\n");
	exit(1);
}

/*
 * runtest: write the data to the file. Read the data from the file and compare.
 *	For each iteration, write data starting at offse+iter*bufsize
 *	location in the file and read from there.
 *
 * XXX (garrcoop): shouldn't use libltp APIs because it runs forked.
 */
int runtest(int fd_r, int fd_w, int childnum, int action)
{
	char *buf1;
	char *buf2;
	off_t seekoff;
	int i;

	/* Allocate for buffers */
	seekoff = offset + bufsize * childnum;
	if ((buf1 = valloc(bufsize)) == 0) {
		tst_resm(TFAIL | TERRNO, "valloc for buf1 failed");
		return (-1);
	}
	if ((buf2 = valloc(bufsize)) == 0) {
		tst_resm(TFAIL | TERRNO, "valloc for buf2 failed");
		return (-1);
	}

	/* seek, write, read and verify */
	for (i = 0; i < iter; i++) {
		fillbuf(buf1, bufsize, childnum + i);
		if (lseek(fd_w, seekoff, SEEK_SET) < 0) {
			tst_resm(TFAIL | TERRNO, "lseek (fd_w, ..) failed");
			return (-1);
		}
		if (write(fd_w, buf1, bufsize) < bufsize) {
			tst_resm(TFAIL | TERRNO, "write failed");
			return (-1);
		}
		if (action == READ_DIRECT) {
			/* Make sure data is on to disk before read */
			if (fsync(fd_w) < 0) {
				tst_resm(TFAIL | TERRNO, "fsync failed");
				return (-1);
			}
		}
		if (lseek(fd_r, seekoff, SEEK_SET) < 0) {
			tst_resm(TFAIL | TERRNO, "lseek(fd_r, ..) failed");
			return (-1);
		}
		if (read(fd_r, buf2, bufsize) < bufsize) {
			tst_resm(TFAIL | TERRNO, "read failed");
			return (-1);
		}
		if (bufcmp(buf1, buf2, bufsize) != 0) {
			tst_resm(TFAIL,
				 "comparsion failed; child=%d offset=%d",
				 childnum, (int)seekoff);
			return (-1);
		}
	}
	tst_exit();
}

/*
 * child_function: open the file for read and write. Call the runtest routine.
*/
int child_function(int childnum, int action)
{
	int fd_w, fd_r;

	switch (action) {
	case READ_DIRECT:
		if ((fd_w = open(filename, O_WRONLY | O_CREAT, 0666)) < 0) {
			tst_resm(TFAIL | TERRNO,
				 "open(%s, O_WRONLY|O_CREAT, ..) failed",
				 filename);
			return (-1);
		}
		if ((fd_r = open(filename, O_DIRECT | O_RDONLY, 0666)) < 0) {
			tst_resm(TFAIL | TERRNO,
				 "open(%s, O_DIRECT|O_RDONLY, ..) failed",
				 filename);
			close(fd_w);
			return (-1);
		}
		if (runtest(fd_r, fd_w, childnum, action) == -1) {
			tst_resm(TFAIL, "Read Direct-child %d failed",
				 childnum);
			close(fd_w);
			close(fd_r);
			return (-1);
		}
		break;
	case WRITE_DIRECT:
		if ((fd_w =
		     open(filename, O_DIRECT | O_WRONLY | O_CREAT, 0666)) < 0) {
			tst_resm(TFAIL, "fd_w open failed for %s: %s", filename,
				 strerror(errno));
			return (-1);
		}
		if ((fd_r = open(filename, O_RDONLY, 0666)) < 0) {
			tst_resm(TFAIL, "fd_r open failed for %s: %s",
				 filename, strerror(errno));
			close(fd_w);
			return (-1);
		}
		if (runtest(fd_r, fd_w, childnum, action) == -1) {
			tst_resm(TFAIL, "Write Direct-child %d failed",
				 childnum);
			close(fd_w);
			close(fd_r);
			return (-1);
		}
		break;
	case RDWR_DIRECT:
		if ((fd_w =
		     open(filename, O_DIRECT | O_WRONLY | O_CREAT, 0666)) < 0) {
			tst_resm(TFAIL, "fd_w open failed for %s: %s", filename,
				 strerror(errno));
			return (-1);
		}
		if ((fd_r = open(filename, O_DIRECT | O_RDONLY, 0666)) < 0) {
			tst_resm(TFAIL, "fd_r open failed for %s: %s",
				 filename, strerror(errno));
			close(fd_w);
			return (-1);
		}
		if (runtest(fd_r, fd_w, childnum, action) == -1) {
			tst_resm(TFAIL, "RDWR Direct-child %d failed",
				 childnum);
			close(fd_w);
			close(fd_r);
			return (-1);
		}
		break;
	default:
		fprintf(stderr, "Invalid Action Value\n");
		return (-1);
	}
	close(fd_w);
	close(fd_r);
	exit(0);
}

static void setup(void);
static void cleanup(void);
static int fd1 = -1;

int main(int argc, char *argv[])
{
	int *pidlst;
	int numchild = 1;	/* Number of children. Default 5 */
	int i, fail_count = 0, failed = 0, total = 0;

	/* Options */
	sprintf(filename, "testdata-3.%ld", syscall(__NR_gettid));
	while ((i = getopt(argc, argv, "b:o:i:n:f:")) != -1) {
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
		case 'n':
			if ((numchild = atoi(optarg)) <= 0) {
				fprintf(stderr, "no of children must be > 0\n");
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
	if (forkchldrn(&pidlst, numchild, READ_DIRECT, child_function) < 0) {
		failed = TRUE;
		fail_count++;
		tst_resm(TFAIL, "Read with Direct IO, Write without");
	} else {
		if (waitchldrn(&pidlst, numchild) < 0) {
			failed = TRUE;
			fail_count++;
			tst_resm(TFAIL, "Read with Direct IO, Write without");
		} else
			tst_resm(TPASS, "Read with Direct IO, Write without");

	}
	unlink(filename);
	free(pidlst);
	fflush(stdout);
	total++;

	/* Testblock-2: Write with Direct IO, Read without */
	if (forkchldrn(&pidlst, numchild, WRITE_DIRECT, child_function) < 0) {
		failed = TRUE;
		fail_count++;
		tst_resm(TFAIL, "Write with Direct IO, Read without");
	} else {
		if (waitchldrn(&pidlst, numchild) < 0) {
			failed = TRUE;
			fail_count++;
			tst_resm(TFAIL, "Write with Direct IO, Read without");
		} else
			tst_resm(TPASS, "Write with Direct IO, Read without");
	}
	unlink(filename);
	free(pidlst);
	fflush(stdout);
	total++;

	/* Testblock-3: Read, Write with Direct IO. */
	if (forkchldrn(&pidlst, numchild, RDWR_DIRECT, child_function) < 0) {
		failed = TRUE;
		fail_count++;
		tst_resm(TFAIL, "Read, Write with Direct IO");
	} else {
		if (waitchldrn(&pidlst, numchild) < 0) {
			failed = TRUE;
			fail_count++;
			tst_resm(TFAIL, "Read, Write with Direct IO");
		} else
			tst_resm(TPASS, "Read, Write with Direct IO");
	}
	unlink(filename);
	free(pidlst);
	total++;

	if (failed)
		tst_resm(TINFO, "%d/%d testblocks failed", fail_count, total);
	else
		tst_resm(TINFO,
			 "%d testblocks %d iterations with %d children completed",
			 total, iter, numchild);
	cleanup();

	tst_exit();

}

static void setup(void)
{
	tst_tmpdir();

	if ((fd1 = open(filename, O_CREAT | O_EXCL, 0600)) < 0)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_CREAT|O_EXCL, ..) failed", filename);
	close(fd1);

	/* Test for filesystem support of O_DIRECT */
	if ((fd1 = open(filename, O_DIRECT, 0600)) < 0)
		tst_brkm(TCONF, cleanup, "open(%s, O_DIRECT, ..) failed",
			 filename);
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
