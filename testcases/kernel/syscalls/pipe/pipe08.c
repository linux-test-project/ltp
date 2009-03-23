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
#include "usctest.h"

char *TCID = "pipe08";
int TST_TOTAL = 1;
extern int Tst_count;

void setup(void);
void cleanup(void);
void sighandler(int);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	int pipefd[2];		/* fds for pipe read/write */
	char wrbuf[BUFSIZ];
	int written, length;
	int close_stat;		/*  exit status of close(read fd) */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	if (!STD_FUNCTIONAL_TEST) {
		tst_resm(TWARN, "-f option should not be used");
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

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
	}
	cleanup();

	 /*NOTREACHED*/ return 0;
}

/*
 * sighandler - catch signals and look for SIGPIPE
 */
void sighandler(int sig)
{
	if (sig != SIGPIPE) {
		tst_resm(TFAIL, "expected SIGPIPE, got %d", sig);
	} else {
		tst_resm(TPASS, "got expected SIGPIPE signal");
	}
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, sighandler, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}
