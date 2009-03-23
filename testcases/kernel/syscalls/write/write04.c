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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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
#include "test.h"
#include "usctest.h"

#define PIPE_SIZE_TEST getpagesize()

void alarm_handler();
void setup();
void cleanup();

char *TCID = "write04";
int TST_TOTAL = 1;
extern int Tst_count;

/* 0 terminated list of expected errnos */
int exp_enos[] = { 11, 0 };

char fifo[100] = "fifo";
static sigjmp_buf jmp;
int rfd, wfd;

int main(int argc, char **argv)
{
	int lc;
	char *msg;

	struct stat buf;
	int fail;
	int cnt;
	char wbuf[17 * PIPE_SIZE_TEST];
	struct sigaction sigptr;	/* set up signal handler */

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, (option_t *) NULL, NULL)) !=
	    (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	/* global setup */
	setup();

	/*
	 * The following loop checks looping state if -i option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		if (mknod(fifo, S_IFIFO | 0777, 0) < 0) {
			tst_resm(TBROK, "mknod() failed, errno: %d", errno);
			cleanup();
		 /*NOTREACHED*/}
		if (stat(fifo, &buf) != 0) {
			tst_resm(TBROK, "stat() failed, errno: %d", errno);
			cleanup();
		 /*NOTREACHED*/}
		if ((buf.st_mode & S_IFIFO) == 0) {
			tst_resm(TBROK, "Mode does not indicate fifo file");
			cleanup();
		 /*NOTREACHED*/}
#if 0
		sigset(SIGALRM, alarm_handler);
#endif
		sigptr.sa_handler = (void (*)(int signal))alarm_handler;
		sigfillset(&sigptr.sa_mask);
		sigptr.sa_flags = 0;
		sigaddset(&sigptr.sa_mask, SIGALRM);
		if (sigaction(SIGALRM, &sigptr, (struct sigaction *)NULL) == -1) {
			tst_resm(TBROK, "sigaction(): Failed");
			cleanup();
		}
//block1:
		tst_resm(TINFO, "Enter block 1: test for EAGAIN in write()");
		fail = 0;

		(void)memset((void *)wbuf, 'A', 17 * PIPE_SIZE_TEST);

		/*
		 * open the read end of the pipe
		 */
		if (sigsetjmp(jmp, 1)) {
			tst_resm(TBROK, "Error reading fifo, read blocked");
			fail = 1;
		}
		(void)alarm(10);	/* set alarm for 10 seconds */
		rfd = open(fifo, O_RDONLY | O_NONBLOCK);
		(void)alarm(0);
		if (rfd < 0) {
			tst_resm(TBROK, "open() for reading the pipe failed");
			fail = 1;
		}

		/*
		 * open the write end of the pipe
		 */
		if (sigsetjmp(jmp, 1)) {
			tst_resm(TBROK, "setjmp() failed");
			cleanup();
		 /*NOTREACHED*/}
		(void)alarm(10);	/* set alarm for 10 seconds */
		wfd = open(fifo, O_WRONLY | O_NONBLOCK);
		(void)alarm(0);
		if (wfd < 0) {
			tst_resm(TBROK, "open() for writing the pipe failed");
			fail = 1;
		}

		/*
		 * attempt to fill the pipe with some data
		 */
		if (sigsetjmp(jmp, 1)) {
			tst_resm(TBROK, "sigsetjmp() failed");
			fail = 1;
		}
		(void)alarm(10);
		cnt = write(wfd, wbuf, 17 * PIPE_SIZE_TEST);
		(void)alarm(0);
		if (cnt == 17 * PIPE_SIZE_TEST) {
			tst_resm(TBROK, "Error reading fifo, nozero read");
			fail = 1;
		}

		/*
		 * Now that the fifo is full try and send some more
		 */
		if (sigsetjmp(jmp, 1)) {
			tst_resm(TBROK, "sigsetjmp() failed");
			fail = 1;
		}
		(void)alarm(10);
		cnt = write(wfd, wbuf, 8 * PIPE_SIZE_TEST);
		(void)alarm(0);
		if (cnt != -1) {
			tst_resm(TBROK, "write() failed to fail when pipe "
				 "is full");
			fail = 1;
		} else {
			TEST_ERROR_LOG(errno);
			if (errno != EAGAIN) {
				tst_resm(TBROK, "write set bad errno, expected "
					 "EAGAIN, got %d", errno);
				fail = 1;
			}
			tst_resm(TINFO, "read() succeded in setting errno to "
				 "EAGAIN");
		}
		if (fail) {
			tst_resm(TFAIL, "Block 1 FAILED");
		} else {
			tst_resm(TPASS, "Block 1 PASSED");
		}
		tst_resm(TINFO, "Exit block 1");

		/* unlink fifo in case we are looping. */
		unlink(fifo);
	}
	cleanup();
	 /*NOTREACHED*/ return 0;
}

void alarm_handler()
{
	siglongjmp(jmp, 1);
}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -i option.
	 * You want to make sure you do this before you create your temporary
	 * directory.
	 */
	TEST_PAUSE;

	/* Create a unique temporary directory and chdir() to it. */
	tst_tmpdir();

	/* create a temporary filename */
	sprintf(fifo, "%s.%d", fifo, getpid());

}

void cleanup()
{
	/*
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	close(rfd);
	close(wfd);
	unlink(fifo);
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
 /*NOTREACHED*/}
