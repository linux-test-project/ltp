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
 *	kill03.c
 *
 * DESCRIPTION
 *	Test case to check that kill fails when given an invalid signal.
 *
 * ALGORITHM
 *	call setup
 *	loop if the -i option was given
 *	fork a child
 *	execute the kill system call with an invalid signal
 *	check the return value
 *	if return value is not -1
 *		issue a FAIL message, break remaining tests and cleanup
 *	if we are doing functional testing
 *		if the errno was set to 22 (invalid argument).
 *			issue a PASS message
 *		otherwise
 *			issue a FAIL message
 *	call cleanup
 *
 * USAGE
 *  kill03 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
 *	none
 */

#include "test.h"
#include "usctest.h"

#include <signal.h>
#include <errno.h>
#include <sys/wait.h>

void cleanup(void);
void setup(void);
void do_child(void);

char *TCID = "kill03";
int TST_TOTAL = 1;

extern int Tst_count;

int exp_enos[] = { EINVAL, 0 };

#define TEST_SIG 2000

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	pid_t pid;
	int exno, status;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}
#ifdef UCLINUX
	maybe_run_child(&do_child, "");
#endif

	setup();		/* global setup */

	TEST_EXP_ENOS(exp_enos);

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;
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
			kill(pid, SIGKILL);
			waitpid(pid, &status, 0);
		}

		if (TEST_RETURN != -1) {
			tst_brkm(TFAIL, cleanup, "%s failed - errno = %d : %s "
				 "Expected a return value of -1 got %ld",
				 TCID, TEST_ERRNO, strerror(TEST_ERRNO),
				 TEST_RETURN);
		 /*NOTREACHED*/}

		if (STD_FUNCTIONAL_TEST) {
			/*
			 * Check to see if the errno was set to the expected
			 * value of 22 : EINVAL.
			 */
			TEST_ERROR_LOG(TEST_ERRNO);
			if (TEST_ERRNO == EINVAL) {
				tst_resm(TPASS, "errno set to %d : %s, as "
					 "expected", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL, "errno set to %d : %s expected "
					 "%d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO), 22,
					 strerror(22));
			}
		}
	}
	cleanup();

	 /*NOTREACHED*/ return 0;
}

/*
 * do_child()
 */
void do_child()
{
	int exno = 1;

	pause();
	 /*NOTREACHED*/ exit(exno);
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{
	/* Pause if that option was specified */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * or premature exit.
 */
void cleanup(void)
{
	/*
	 * print timing status if that option was specified.
	 * print errno log if that option was specified
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}
