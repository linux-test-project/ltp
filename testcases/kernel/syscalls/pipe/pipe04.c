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
 *	pipe04.c
 *
 * DESCRIPTION
 * 	Check that processes are killable, even when they are still writing
 * 	data to a pipe.
 *
 * ALGORITHM
 * 	1. Open a pipe
 * 	2. fork a two children that will write to the pipe
 * 	3. read a bit from both children
 * 	3. kill both children and wait to make sure they die
 *
 * USAGE:  <for command-line>
 *  pipe04 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	11/2002 Ported by Paul Larson
 */
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "test.h"
#include "safe_macros.h"

char *TCID = "pipe04";
int TST_TOTAL = 1;

int fildes[2];			/* fds for pipe read and write */

void setup(void);
void cleanup(void);
void c1func(void);
void c2func(void);
void alarmfunc(int);

ssize_t do_read(int fd, void *buf, size_t count)
{
	ssize_t n;

	do {
		n = read(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}

int main(int ac, char **av)
{
	int lc;
	pid_t c1pid, c2pid;
	int wtstatus;
	int bytesread;
	int acnt = 0, bcnt = 0;

	char rbuf[BUFSIZ];

	tst_parse_opts(ac, av, NULL, NULL);
#ifdef UCLINUX
	maybe_run_child(&c1func, "ndd", 1, &fildes[0], &fildes[1]);
	maybe_run_child(&c2func, "ndd", 2, &fildes[0], &fildes[1]);
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;

		SAFE_PIPE(cleanup, fildes);

		if ((c1pid = FORK_OR_VFORK()) == -1)
			tst_brkm(TBROK, cleanup, "fork() failed - "
				 "errno %d", errno);
		if (c1pid == 0)
#ifdef UCLINUX
			if (self_exec(av[0], "ndd", 1, fildes[0], fildes[1]) <
			    0) {
				tst_brkm(TBROK, cleanup, "self_exec failed");
			}
#else
			c1func();
#endif
		if ((c2pid = FORK_OR_VFORK()) == -1)
			tst_brkm(TBROK, cleanup, "fork() failed - "
				 "errno %d", errno);
		if (c2pid == 0)
#ifdef UCLINUX
			if (self_exec(av[0], "ndd", 2, fildes[0], fildes[1]) <
			    0) {
				tst_brkm(TBROK, cleanup, "self_exec failed");
			}
#else
			c2func();
#endif

		/* PARENT */
		if (close(fildes[1]) == -1)
			tst_resm(TWARN, "Could not close fildes[1] - errno %d",
				 errno);
		/*
		 * Read a bit from the children first
		 */
		while ((acnt < 100) && (bcnt < 100)) {
			bytesread = do_read(fildes[0], rbuf, sizeof(rbuf));
			if (bytesread < 0) {
				tst_resm(TFAIL, "Unable to read from pipe, "
					 "errno=%d", errno);
				break;
			}
			switch (rbuf[1]) {
			case 'A':
				acnt++;
				break;
			case 'b':
				bcnt++;
				break;
			default:
				tst_resm(TFAIL, "Got bogus '%c' "
					 "character", rbuf[1]);
				break;
			}
		}

		/*
		 * Try to kill the children
		 */
		if (kill(c1pid, SIGKILL) == -1)
			tst_resm(TFAIL, "failed to kill child 1, errno=%d",
				 errno);
		if (kill(c2pid, SIGKILL) == -1)
			tst_resm(TFAIL, "failed to kill child 1, errno=%d",
				 errno);

		/*
		 * Set action for the alarm
		 */
		if (signal(SIGALRM, alarmfunc) == SIG_ERR)
			tst_resm(TWARN | TERRNO, "call to signal failed");
		/*
		 * Set an alarm for 60 seconds just in case the child
		 * processes don't die
		 */
		alarm(60);
		if (waitpid(c1pid, &wtstatus, 0) != -1) {
			if (wtstatus != SIGKILL)
				tst_resm(TFAIL | TERRNO,
					 "unexpected wait status " "%d",
					 wtstatus);
			else
				tst_resm(TPASS, "Child 1 killed while "
					 "writing to a pipe");
		}
		if (waitpid(c2pid, &wtstatus, 0) != -1) {
			if (!WIFSIGNALED(wtstatus) ||
			    WTERMSIG(wtstatus) != SIGKILL)
				tst_resm(TFAIL | TERRNO,
					 "unexpected wait status " "%d",
					 wtstatus);
			else
				tst_resm(TPASS, "Child 2 killed while "
					 "writing to a pipe");
		}
		if (alarm(0) <= 0)
			tst_resm(TWARN, "call to alarm(0) failed");
	}
	cleanup();

	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup(void)
{
}

void c1func(void)
{
	if (close(fildes[0]) == -1)
		tst_resm(TWARN, "Could not close fildes[0] - errno %d", errno);
	while (1)
		if (write(fildes[1], "bbbbbbbbbbbbbbbbbbbbbbbbb", 25) == -1)
			tst_resm(TBROK | TERRNO, "[child 1] pipe write failed");
}

void c2func(void)
{
	if (close(fildes[0]) == -1)
		tst_resm(TWARN, "Could not close fildes[0] - errno %d", errno);
	while (1)
		if (write(fildes[1], "AAAAAAAAAAAAAAAAAAAAAAAAA", 25) == -1)
			tst_resm(TBROK | TERRNO, "[child 2] pipe write failed");
}

void alarmfunc(int sig LTP_ATTRIBUTE_UNUSED)
{
	/* for some reason tst_brkm doesn't seem to work in a signal handler */
	tst_brkm(TFAIL, cleanup, "one or more children did't die in 60 second "
		 "time limit");
}
