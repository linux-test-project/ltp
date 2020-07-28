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
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "test.h"
#include "safe_macros.h"
#include "tst_safe_pthread.h"

char *TCID = "pipe04";
int TST_TOTAL = 1;

int fildes[2];			/* fds for pipe read and write */
int c1func_exit = 0;
int c2func_exit = 0;

void setup(void);
void cleanup(void);
void* c1func(void* parm);
void* c2func(void* parm);
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
	pthread_t tid1, tid2;
	int bytesread;
	int acnt = 0, bcnt = 0;

	char rbuf[BUFSIZ];

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;

		SAFE_PIPE(cleanup, fildes);

		SAFE_PTHREAD_CREATE(&tid1, NULL, c1func, NULL);
		SAFE_PTHREAD_CREATE(&tid2, NULL, c2func, NULL);

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

		// Exit from the thread
		c1func_exit = 1;
		c2func_exit = 1;

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
		
		// Wait for all threads to join
		SAFE_PTHREAD_JOIN(tid1, NULL);
		SAFE_PTHREAD_JOIN(tid2, NULL);
		
		// close pipe descriptors
		if (close(fildes[0]) == -1)
			tst_resm(TFAIL, "Could not close fildes[0] - errno %d",
				 errno);
		if (close(fildes[1]) == -1)
			tst_resm(TFAIL, "Could not close fildes[1] - errno %d",
				 errno);
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

void* c1func(void* parm LTP_ATTRIBUTE_UNUSED)
{
	while (c1func_exit != 1)
		if (write(fildes[1], "bbbbbbbbbbbbbbbbbbbbbbbbb", 25) == -1)
			tst_resm(TBROK | TERRNO, "[child 1] pipe write failed");
	pthread_exit(0);
}

void* c2func(void* parm LTP_ATTRIBUTE_UNUSED)
{
	while (c2func_exit != 1)
		if (write(fildes[1], "AAAAAAAAAAAAAAAAAAAAAAAAA", 25) == -1)
			tst_resm(TBROK | TERRNO, "[child 2] pipe write failed");
	pthread_exit(0);
}

void alarmfunc(int sig LTP_ATTRIBUTE_UNUSED)
{
	/* for some reason tst_brkm doesn't seem to work in a signal handler */
	tst_brkm(TFAIL, cleanup, "one or more children did't die in 60 second "
		 "time limit");
}
