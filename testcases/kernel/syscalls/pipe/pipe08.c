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
 *	pipe08.c
 *
 * DESCRIPTION
 *	Check that a SIGPIPE signal is generated when a write is
 *	attempted on an empty pipe.
 *
 * ALGORITHM
 *	1. Write to a pipe after closing the read side.
 *	2. Check for the signal SIGPIPE to be received.
 *
 * USAGE:  <for command-line>
 *  pipe08 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * USAGE
 *	pipe08
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	None
 */
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "test.h"

char *TCID = "pipe08";
int TST_TOTAL = 1;

void setup(void);
void cleanup(void);
void sighandler(int);

int main(int ac, char **av)
{
	int lc;

	int pipefd[2];		/* fds for pipe read/write */
	char wrbuf[BUFSIZ];
	int written, length;
	int close_stat;		/*  exit status of close(read fd) */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;

		TEST(pipe(pipefd));

		if (TEST_RETURN != 0) {
			tst_resm(TFAIL, "call failed unexpectedly");
			continue;
		}

		if ((close_stat = close(pipefd[0])) == -1) {
			tst_brkm(TBROK, cleanup, "close of read side failed");
		}

		strcpy(wrbuf, "abcdefghijklmnopqrstuvwxyz\0");
		length = strlen(wrbuf);

		/*
		 * the SIGPIPE signal will be caught here or else
		 * the program will dump core when the signal is
		 * sent
		 */
		written = write(pipefd[1], wrbuf, length);
		if (written > 0)
			tst_brkm(TBROK, cleanup, "write succeeded unexpectedly");
	}
	cleanup();
	tst_exit();

}

/*
 * sighandler - catch signals and look for SIGPIPE
 */
void sighandler(int sig)
{
	if (sig != SIGPIPE)
		tst_resm(TFAIL, "expected SIGPIPE, got %d", sig);
	else
		tst_resm(TPASS, "got expected SIGPIPE signal");
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(NOFORK, sighandler, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup(void)
{
}
