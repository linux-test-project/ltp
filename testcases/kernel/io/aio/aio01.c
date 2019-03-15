/*
 *
 *   Copyright (c) International Business Machines  Corp., 2003
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
 *      aiotest1.c
 *
 * DESCRIPTION
 *      Perform aio read, write operations for given number of requests.
 *      Submit i/o for each request individually.
 *      Repeat the test for each of the following cases and measure time.
 *              Testblock1: Write one request at a time.
 *              Testblock2: Read one request at a time.
 *              Testblock3: Prepare, Write one request at a time.
 *              Testblock4: Prepare, Read one request at a time.
 *              Testblock5: Prepare, Write/Read one request at a time.
 *              Testblock6: Prepare, Write/Read/Verify one request at a time.
 *
 * Author
 * 08/24/2002   Narasimha Sharoff       nsharoff@us.ibm.com
*/

/*
 * History
 *      04/18/2003      nsharoff@us.ibm.com
 *      		Updated
 *      05/21/2003      Paul Larson	plars@linuxtestproject.org
 *      		Rewrote the test under LTP, using LTP test harness
 *      		and other minor improvements and fixes
*/

#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "test.h"
#include "config.h"

char *TCID = "aio01";
int TST_TOTAL = 6;

#ifdef HAVE_LIBAIO
#include <libaio.h>

static void help(void);
static void setup(void);
static void cleanup(void);

#define mapsize (1 << 14)

int fd;
char *maddr;

size_t bufsize;			/* Size of I/O, 8k default */
io_context_t io_ctx;		/* I/O Context */
struct iocb **iocbs;		/* I/O Control Blocks */
char *srcbuf, *dstbuf;
char fname[128];
char tbuf[80];
int pos, nr;
struct stat s;

struct test_case_t {
	off_t newsize;
	char *desc;
} TC[] = {
	{
	mapsize - 8192, "ftruncate mmaped file to a smaller size"}, {
	mapsize + 1024, "ftruncate mmaped file to a larger size"}, {
0, "ftruncate mmaped file to 0 size"},};

int main(int argc, char **argv)
{
	int i, j, sec, usec;
	int failflag = 0;
	int bflag = 0, nflag = 0, Fflag = 0;
	char *optb, *optn, *optF;
	struct io_event event;
	static struct timespec ts;
	struct timeval stv, etv;

	option_t options[] = {
		{"b:", &bflag, &optb},
		{"n:", &nflag, &optn},
		{"F:", &Fflag, &optF},
		{NULL, NULL, NULL}
	};

	tst_parse_opts(argc, argv, options, &help);

	bufsize = (bflag ? atoi(optb) : 8192);
	nr = (nflag ? atoi(optn) : 10);
	if (Fflag) {
		sprintf(fname, "%s", optF);
	} else {
		sprintf(fname, "aiofile");
	}

	setup();

/* TEST 1 */
	pos = 0;
	gettimeofday(&stv, NULL);
	io_prep_pwrite(iocbs[0], fd, srcbuf, bufsize, pos);
	for (i = 0; i < nr; i++) {
		ts.tv_sec = 30;
		ts.tv_nsec = 0;
		do {
			TEST(io_submit(io_ctx, 1, iocbs));
		} while (TEST_RETURN == -EAGAIN);
		if (TEST_RETURN < 0) {
			tst_resm(TFAIL, "Test 1: io_submit failed - retval=%ld"
				 ", errno=%d", TEST_RETURN, TEST_ERRNO);
			failflag = 1;
			continue;
		}
		while (io_getevents(io_ctx, 1, 1, &event, &ts) != 1) ;
		gettimeofday(&etv, NULL);
	}
	if (!failflag) {
		sec = etv.tv_sec - stv.tv_sec;
		usec = etv.tv_usec - stv.tv_usec;
		if (usec < 0) {
			usec += 1000000;
			sec--;
		}
		tst_resm(TPASS, "Test 1: %d writes in %3d.%06d sec",
			 nr, sec, usec);
	}

/* TEST 2 */
	pos = 0;
	failflag = 0;
	gettimeofday(&stv, NULL);
	io_prep_pread(iocbs[0], fd, dstbuf, bufsize, pos);
	for (i = 0; i < nr; i++) {
		ts.tv_sec = 30;
		ts.tv_nsec = 0;
		do {
			TEST(io_submit(io_ctx, 1, iocbs));
		} while (TEST_RETURN == -EAGAIN);
		if (TEST_RETURN < 0) {
			tst_resm(TFAIL, "Test 2: io_submit failed - retval=%ld"
				 ", errno=%d", TEST_RETURN, TEST_ERRNO);
			failflag = 1;
			continue;
		}
		while (io_getevents(io_ctx, 1, 1, &event, &ts) != 1) ;
		gettimeofday(&etv, NULL);
	}
	if (!failflag) {
		sec = etv.tv_sec - stv.tv_sec;
		usec = etv.tv_usec - stv.tv_usec;
		if (usec < 0) {
			usec += 1000000;
			sec--;
		}
		tst_resm(TPASS, "Test 2: %d reads in %3d.%06d sec",
			 nr, sec, usec);
	}

/* TEST 3 */
	pos = 0;
	failflag = 0;
	gettimeofday(&stv, NULL);
	for (i = 0; i < nr; i++) {
		io_prep_pwrite(iocbs[0], fd, srcbuf, bufsize, pos);
		ts.tv_sec = 30;
		ts.tv_nsec = 0;
		do {
			TEST(io_submit(io_ctx, 1, iocbs));
		} while (TEST_RETURN == -EAGAIN);
		if (TEST_RETURN < 0) {
			tst_resm(TFAIL, "Test 3: io_submit failed - retval=%ld"
				 ", errno=%d", TEST_RETURN, TEST_ERRNO);
			failflag = 1;
			continue;
		}
		while (io_getevents(io_ctx, 1, 1, &event, &ts) != 1) ;
		gettimeofday(&etv, NULL);
	}
	if (!failflag) {
		sec = etv.tv_sec - stv.tv_sec;
		usec = etv.tv_usec - stv.tv_usec;
		if (usec < 0) {
			usec += 1000000;
			sec--;
		}
		tst_resm(TPASS, "Test 3: %d prep,writes in %3d.%06d sec",
			 nr, sec, usec);
	}

/* TEST 4 */
	pos = 0;
	failflag = 0;
	gettimeofday(&stv, NULL);
	for (i = 0; i < nr; i++) {
		io_prep_pread(iocbs[0], fd, dstbuf, bufsize, pos);
		ts.tv_sec = 30;
		ts.tv_nsec = 0;
		do {
			TEST(io_submit(io_ctx, 1, iocbs));
		} while (TEST_RETURN == -EAGAIN);
		if (TEST_RETURN < 0) {
			tst_resm(TFAIL, "Test 4: io_submit failed - retval=%ld"
				 ", errno=%d", TEST_RETURN, TEST_ERRNO);
			failflag = 1;
			continue;
		}
		while (io_getevents(io_ctx, 1, 1, &event, &ts) != 1) ;
		gettimeofday(&etv, NULL);
	}
	if (!failflag) {
		sec = etv.tv_sec - stv.tv_sec;
		usec = etv.tv_usec - stv.tv_usec;
		if (usec < 0) {
			usec += 1000000;
			sec--;
		}
		tst_resm(TPASS, "Test 4: %d prep,reads in %3d.%06d sec",
			 nr, sec, usec);
	}

/* TEST 5 */
	pos = 0;
	failflag = 0;
	gettimeofday(&stv, NULL);
	for (i = 0; i < nr; i++) {
		io_prep_pwrite(iocbs[0], fd, srcbuf, bufsize, pos);
		ts.tv_sec = 30;
		ts.tv_nsec = 0;
		do {
			TEST(io_submit(io_ctx, 1, iocbs));
		} while (TEST_RETURN == -EAGAIN);
		if (TEST_RETURN < 0) {
			tst_resm(TFAIL, "Test 5: write io_submit failed - "
				 "retval=%ld, errno=%d", TEST_RETURN,
				 TEST_ERRNO);
			failflag = 1;
			continue;
		}
		while (io_getevents(io_ctx, 1, 1, &event, &ts) != 1) ;
		io_prep_pread(iocbs[0], fd, dstbuf, bufsize, pos);
		ts.tv_sec = 30;
		ts.tv_nsec = 0;
		do {
			TEST(io_submit(io_ctx, 1, iocbs));
		} while (TEST_RETURN == -EAGAIN);
		if (TEST_RETURN < 0) {
			tst_resm(TFAIL, "Test 5: read io_submit failed - "
				 "retval=%ld, errno=%d", TEST_RETURN,
				 TEST_ERRNO);
			failflag = 1;
			continue;
		}
		while (io_getevents(io_ctx, 1, 1, &event, &ts) != 1) ;
		gettimeofday(&etv, NULL);
	}
	if (!failflag) {
		sec = etv.tv_sec - stv.tv_sec;
		usec = etv.tv_usec - stv.tv_usec;
		if (usec < 0) {
			usec += 1000000;
			sec--;
		}
		tst_resm(TPASS, "Test 5: %d reads and writes in %3d.%06d sec",
			 nr, sec, usec);
	}

/* TEST 6 */
	pos = 0;
	failflag = 0;
	gettimeofday(&stv, NULL);
	for (i = 0; i < nr; i++) {
		io_prep_pwrite(iocbs[0], fd, srcbuf, bufsize, pos);
		ts.tv_sec = 30;
		ts.tv_nsec = 0;
		do {
			TEST(io_submit(io_ctx, 1, iocbs));
		} while (TEST_RETURN == -EAGAIN);
		if (TEST_RETURN < 0) {
			tst_resm(TFAIL, "Test 6: write io_submit failed - "
				 "retval=%ld, errno=%d", TEST_RETURN,
				 TEST_ERRNO);
			failflag = 1;
			continue;
		}
		while (io_getevents(io_ctx, 1, 1, &event, &ts) != 1) ;
		io_prep_pread(iocbs[0], fd, dstbuf, bufsize, pos);
		ts.tv_sec = 30;
		ts.tv_nsec = 0;
		do {
			TEST(io_submit(io_ctx, 1, iocbs));
		} while (TEST_RETURN == -EAGAIN);
		if (TEST_RETURN < 0) {
			tst_resm(TFAIL, "Test 6: read io_submit failed - "
				 "retval=%ld, errno=%d", TEST_RETURN,
				 TEST_ERRNO);
			failflag = 1;
			continue;
		}
		while (io_getevents(io_ctx, 1, 1, &event, &ts) != 1) ;
		for (j = 0; j < (int)bufsize; j++) {
			if (srcbuf[j] != dstbuf[j]) {
				tst_resm(TFAIL, "Test 6: compare failed - "
					 "read: %c, " "actual: %c",
					 dstbuf[j], srcbuf[j]);
				break;
			}
		}
		gettimeofday(&etv, NULL);
	}
	if (!failflag) {
		sec = etv.tv_sec - stv.tv_sec;
		usec = etv.tv_usec - stv.tv_usec;
		if (usec < 0) {
			usec += 1000000;
			sec--;
		}
		tst_resm(TPASS, "Test 6: %d read,write,verify in %d.%06d sec",
			 i, sec, usec);
	}

	cleanup();

	tst_exit();
}

static void help(void)
{
	printf("  -b n    Buffersize\n");
	printf("  -n n    Number of requests\n");
	printf("  -F s    Filename to run the tests against\n");
}

static void setup(void)
{
	int ret;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if option was specified */
	TEST_PAUSE;

	tst_tmpdir();

	if ((fd = open(fname, O_RDWR | O_CREAT, 0600)) < 0)
		tst_brkm(TFAIL, cleanup, "failed to open %s "
			 "file, errno: %d", fname, errno);
	stat(fname, &s);
	if ((iocbs = malloc(sizeof(int) * nr)) == NULL)
		tst_brkm(TFAIL, cleanup, "malloc for iocbs failed - "
			 "errno: %d", errno);
	if ((iocbs[0] = malloc(sizeof(struct iocb))) == NULL)
		tst_brkm(TFAIL, cleanup, "malloc for iocbs elements failed - "
			 "errno: %d", errno);
	if (S_ISCHR(s.st_mode)) {
		if ((ret =
		     posix_memalign((void **)&srcbuf, bufsize, bufsize)) != 0)
			tst_brkm(TFAIL, cleanup,
				 "posix_memalign for srcbuf "
				 "failed - errno: %d", errno);
		if ((ret =
		     posix_memalign((void **)&dstbuf, bufsize, bufsize)) != 0)
			tst_brkm(TFAIL, cleanup,
				 "posix_memalign for dstbuf "
				 "failed - errno: %d", errno);
	} else {
		if ((srcbuf = malloc(sizeof(char) * bufsize)) == NULL)
			tst_brkm(TFAIL, cleanup, "malloc for srcbuf "
				 "failed - errno: %d", errno);
		if ((dstbuf = malloc(sizeof(char) * bufsize)) == NULL)
			tst_brkm(TFAIL, cleanup, "malloc for dstbuf "
				 "failed - errno: %d", errno);
	}
	memset((void *)srcbuf, 65, bufsize);
	if ((ret = io_queue_init(1, &io_ctx)) != 0)
		tst_brkm(TFAIL, cleanup, "io_queue_init failed: %s",
			 strerror(ret));
}

static void cleanup(void)
{
	free(dstbuf);
	free(srcbuf);
	free(iocbs[0]);
	free(iocbs);
	close(fd);
	io_queue_release(io_ctx);
	tst_rmdir();
}

#else
int main(void)
{
	tst_brkm(TCONF, NULL, "test requires libaio and it's development packages");
}
#endif
