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
 *	waitpid01.c
 *
 * DESCRIPTION
 *	Check that when a child kills itself by generating an alarm
 *	exception, the waiting parent is correctly notified.
 *
 * ALGORITHM
 *	Fork a child that sets an alarm. When the alarm goes off, causing
 *	the death of the child, the parent checks that SIG_ALRM was returned
 *
 * USAGE:  <for command-line>
 *      waitpid01 [-c n] [-i n] [-I x] [-P x] [-t]
 *      where,  -c n : Run n copies concurrently.
 *              -i n : Execute test n times.
 *              -I x : Execute test for x seconds.
 *              -P x : Pause for x seconds between iterations.
 *              -t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions
 *	None
 */

#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "test.h"

static void setup(void);
static void cleanup(void);

char *TCID = "waitpid01";
int TST_TOTAL = 1;

int main(int argc, char **argv)
{
	int lc;

	int pid, npid, sig, nsig;
	int exno, nexno, status;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		exno = 1;
		sig = 14;

		pid = FORK_OR_VFORK();
		if (pid < 0) {
			tst_brkm(TFAIL, cleanup, "Fork Failed");
		} else if (pid == 0) {
			alarm(2);
			pause();
			exit(exno);
		} else {
			errno = 0;
			while (((npid = waitpid(pid, &status, 0)) != -1) ||
			       (errno == EINTR)) {
				if (errno == EINTR)
					continue;

				if (npid != pid) {
					tst_resm(TFAIL, "waitpid error: "
						 "unexpected pid returned");
				} else {
					tst_resm(TPASS,
						 "recieved expected pid");
				}

				nsig = WTERMSIG(status);

				/*
				 * nsig is the signal number returned by
				 * waitpid
				 */
				if (nsig != sig) {
					tst_resm(TFAIL, "waitpid error: "
						 "unexpected signal "
						 "returned");
				} else {
					tst_resm(TPASS, "recieved expected "
						 "signal");
				}

				/*
				 * nexno is the exit number returned by
				 * waitpid
				 */
				nexno = WEXITSTATUS(status);
				if (nexno != 0) {
					tst_resm(TFAIL, "signal error: "
						 "unexpected exit number "
						 "returned");
				}
			}
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	TEST_PAUSE;
}

static void cleanup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);
}
