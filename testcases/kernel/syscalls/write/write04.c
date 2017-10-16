/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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
 *	write04.c
 *
 * DESCRIPTION
 *	Testcase to check that write() sets errno to EAGAIN
 *
 * ALGORITHM
 *	Create a named pipe (fifo), open it in O_NONBLOCK mode, and
 *	attempt to write to it when it is full, write(2) should fail
 *	with EAGAIN.
 *
 * USAGE:  <for command-line>
 *      write04 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *      where,  -c n : Run n copies concurrently.
 *              -e   : Turn on errno logging.
 *              -i n : Execute test n times.
 *              -I x : Execute test for x seconds.
 *              -P x : Pause for x seconds between iterations.
 *              -t   : Turn on syscall timing.
 *
 * HISTORY
 *      ??/???? someone made this testcase but didn't add HISTORY
 *
 * RESTRICTIONS
 *	NONE
 */

#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "tst_test.h"

#define PIPE_SIZE_TEST getpagesize()

void alarm_handler();

char fifo[100] = "fifo";
static sigjmp_buf jmp;
int rfd, wfd;

static void verify_write(void)
{
	struct stat buf;
	int fail;
	int cnt;
	char wbuf[17 * PIPE_SIZE_TEST];
	struct sigaction sigptr;	/* set up signal handler */

	if (mknod(fifo, S_IFIFO | 0777, 0) < 0)
		tst_res(TBROK | TERRNO, "mknod() failed, errno: %d", errno);

	if (stat(fifo, &buf) != 0)
		tst_res(TBROK | TERRNO, "stat() failed, errno: %d", errno);

	if ((buf.st_mode & S_IFIFO) == 0)
		tst_res(TBROK | TERRNO, "Mode does not indicate fifo file");

#if 0
	sigset(SIGALRM, alarm_handler);
#endif
	sigptr.sa_handler = (void (*)(int signal))alarm_handler;
	sigfillset(&sigptr.sa_mask);
	sigptr.sa_flags = 0;
	sigaddset(&sigptr.sa_mask, SIGALRM);
	if (sigaction(SIGALRM, &sigptr, NULL) == -1)
		tst_res(TBROK | TERRNO, "sigaction(): Failed");

//block1:
	tst_res(TINFO, "Enter block 1: test for EAGAIN in write()");
	fail = 0;

	(void)memset((void *)wbuf, 'A', 17 * PIPE_SIZE_TEST);

	/*
	 * open the read end of the pipe
	 */
	if (sigsetjmp(jmp, 1)) {
		tst_res(TBROK | TERRNO, "Error reading fifo, read blocked");
		fail = 1;
	}
	(void)alarm(10);	/* set alarm for 10 seconds */
	rfd = open(fifo, O_RDONLY | O_NONBLOCK);
	(void)alarm(0);
	if (rfd < 0) {
		tst_res(TBROK | TERRNO, "open() for reading the pipe failed");
		fail = 1;
	}

	/*
	 * open the write end of the pipe
	 */
	if (sigsetjmp(jmp, 1))
		tst_res(TBROK | TERRNO, "setjmp() failed");

	(void)alarm(10);	/* set alarm for 10 seconds */
	wfd = open(fifo, O_WRONLY | O_NONBLOCK);
	(void)alarm(0);
	if (wfd < 0) {
		tst_res(TBROK | TERRNO, "open() for writing the pipe failed");
		fail = 1;
	}

	/*
	 * attempt to fill the pipe with some data
	 */
	if (sigsetjmp(jmp, 1)) {
		tst_res(TBROK | TERRNO, "sigsetjmp() failed");
		fail = 1;
	}
	(void)alarm(10);
	cnt = write(wfd, wbuf, 17 * PIPE_SIZE_TEST);
	(void)alarm(0);
	if (cnt == 17 * PIPE_SIZE_TEST) {
		tst_res(TBROK | TERRNO, "Error reading fifo, nozero read");
		fail = 1;
	}

	/*
	 * Now that the fifo is full try and send some more
	 */
	if (sigsetjmp(jmp, 1)) {
		tst_res(TBROK | TERRNO, "sigsetjmp() failed");
		fail = 1;
	}
	(void)alarm(10);
	cnt = write(wfd, wbuf, 8 * PIPE_SIZE_TEST);
	(void)alarm(0);
	if (cnt != -1) {
		tst_res(TBROK | TERRNO, "write() failed to fail when pipe "
			"is full");
		fail = 1;
	} else {
		if (errno != EAGAIN) {
			tst_res(TBROK, "write set bad errno, expected "
				"EAGAIN, got %d", errno);
			fail = 1;
		}
		tst_res(TINFO, "write() succeded in setting errno to "
			"EAGAIN");
	}
	if (fail) {
		tst_res(TFAIL, "Block 1 FAILED");
	} else {
		tst_res(TPASS, "Block 1 PASSED");
	}
	tst_res(TINFO, "Exit block 1");

	/* unlink fifo in case we are looping. */
	unlink(fifo);
}

void alarm_handler(void)
{
	siglongjmp(jmp, 1);
}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
static void setup(void)
{
	/* create a temporary filename */
	sprintf(fifo, "%s.%d", fifo, getpid());
}

static void cleanup(void)
{
	if (rfd > 0)
		SAFE_CLOSE(rfd);
	if (wfd > 0)
		SAFE_CLOSE(wfd);
	unlink(fifo);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_write,
};
