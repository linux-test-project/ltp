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
 *	kill01.c
 *
 * DESCRIPTION
 *	Test case to check the basic functionality of kill().
 *
 * ALGORITHM
 *	call setup
 *	loop if the -i option was given
 *	fork a child
 *	execute the kill system call
 *	check the return value
 *	if return value is -1
 *		issue a FAIL message, break remaining tests and cleanup
 *	if we are doing functional testing
 *		if the process was terminated with the expected signal.
 *			issue a PASS message
 *		otherwise
 *			issue a FAIL message
 *	call cleanup
 *
 * USAGE
 *  kill01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	This test should be ran as a non-root user.
 */

#include "test.h"

#include <signal.h>
#include <errno.h>
#include <sys/wait.h>

void cleanup(void);
void setup(void);
void do_child(void);

char *TCID = "kill01";
int TST_TOTAL = 1;

#define TEST_SIG SIGKILL

int main(int ac, char **av)
{
	int lc;
	pid_t pid;
	int exno, status, nsig;

	tst_parse_opts(ac, av, NULL, NULL);
#ifdef UCLINUX
	maybe_run_child(&do_child, "");
#endif

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;
		status = 1;
		exno = 1;
		pid = FORK_OR_VFORK();
		if (pid < 0) {
			tst_brkm(TBROK, cleanup, "Fork of child failed");
		} else if (pid == 0) {
#ifdef UCLINUX
			if (self_exec(av[0], "") < 0) {
				tst_brkm(TBROK, cleanup,
					 "self_exec of child failed");
			}
#else
			do_child();
#endif
		} else {
			TEST(kill(pid, TEST_SIG));
			waitpid(pid, &status, 0);
		}

		if (TEST_RETURN == -1) {
			tst_brkm(TFAIL, cleanup, "%s failed - errno = %d : %s",
				 TCID, TEST_ERRNO, strerror(TEST_ERRNO));
		}

		/*
		 * Check to see if the process was terminated with the
		 * expected signal.
		 */
		nsig = WTERMSIG(status);
		if (nsig == TEST_SIG) {
			tst_resm(TPASS, "received expected signal %d",
				 nsig);
		} else {
			tst_resm(TFAIL,
				 "expected signal %d received %d",
				 TEST_SIG, nsig);
		}
	}

	cleanup();
	tst_exit();
}

/*
 * do_child()
 */
void do_child(void)
{
	int exno = 1;

	pause();
	exit(exno);
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{

	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * or premature exit.
 */
void cleanup(void)
{

}
