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
 *	waitpid11.c
 *
 * DESCRIPTION
 *	Tests to see if pid's returned from fork and waitpid are same
 *
 * ALGORITHM
 * 	Check proper functioning of waitpid with pid = -1 and arg = 0
 *
 * USAGE:  <for command-line>
 *      waitpid11 [-c n] [-t]
 *      where,  -c n : Run n copies concurrently.
 *              -t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *      04/2002 wjhuie sigset cleanups
 *
 * Restrictions
 * 	None
 */

#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <test.h>
#include <usctest.h>

#define	MAXKIDS	8

char *TCID = "waitpid11";
int TST_TOTAL = 1;
extern int Tst_count;

volatile int intintr;
void setup(void);
void cleanup(void);
void inthandlr();
void wait_for_parent();
void do_exit();
void setup_sigint();
#ifdef UCLINUX
void do_exit_uclinux();
#endif

int fail;

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	int kid_count, ret_val, status;
	int i, j, k, found;
	int group1, group2;
	int fork_kid_pid[MAXKIDS], wait_kid_pid[MAXKIDS];
	int pid;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	 /*NOTREACHED*/}
#ifdef UCLINUX
	maybe_run_child(&do_exit_uclinux, "");
#endif

	setup();

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;
		fail = 0;

		/*
		 * Need to have test run from child as test driver causes
		 * test to be a session leader and setpgrp fails.
		 */

		if ((pid = FORK_OR_VFORK()) != 0) {
			waitpid(pid, &status, 0);
			if (WEXITSTATUS(status) != 0) {
				tst_resm(TFAIL, "child returned bad status");
				fail = 1;
			}
			if (fail) {
				tst_resm(TFAIL, "%s FAILED", TCID);
			} else {
				tst_resm(TPASS, "%s PASSED", TCID);
			}
			cleanup();
		 /*NOTREACHED*/} else if (pid < 0) {
			tst_brkm(TBROK, cleanup, "fork failed");
		 /*NOTREACHED*/}

		/*
		 * Set up to catch SIGINT.  The kids will wait till a SIGINT
		 * has been received before they proceed.
		 */
		setup_sigint();

		group1 = getpgrp();

		for (kid_count = 0; kid_count < MAXKIDS; kid_count++) {
			if (kid_count == (MAXKIDS / 2)) {
				group2 = setpgrp();
			}

			intintr = 0;
			ret_val = FORK_OR_VFORK();
			if (ret_val == 0) {
#ifdef UCLINUX
				if (self_exec(av[0], "") < 0) {
					tst_resm(TFAIL, "self_exec kid %d "
						 "failed", kid_count);
					tst_exit();
				 /*NOTREACHED*/}
#else
				do_exit();
#endif
			 /*NOTREACHED*/}

			if (ret_val < 0) {
				tst_resm(TFAIL, "Fork kid %d failed. errno = "
					 "%d", kid_count, errno);
				tst_exit();
			 /*NOTREACHED*/}

			/* parent */
			fork_kid_pid[kid_count] = ret_val;
		}

#ifdef UCLINUX
		/* Give the kids a chance to setup SIGINT again, since this is
		 * cleared by exec().
		 */
		sleep(3);
#endif

		/* Now send all the kids a SIGINT to tell them to proceed */
		for (i = 0; i < MAXKIDS; i++) {
			if (kill(fork_kid_pid[i], SIGINT) < 0) {
				tst_resm(TFAIL, "Kill of child %d failed, "
					 "errno = %d", i, errno);
				tst_exit();
			 /*NOTREACHED*/}
		}

		/*
		 * Wait till all kids have terminated.  Stash away their
		 * pid's in an array.
		 */
		kid_count = 0;
		errno = 0;
		while (((ret_val = waitpid(0, &status, 0)) != -1) ||
		       (errno == EINTR)) {
			if (ret_val == -1) {
				continue;
			}

			if (!WIFEXITED(status)) {
				tst_resm(TFAIL, "Child %d did not exit "
					 "normally", ret_val);
				fail = 1;
			} else {
				if (WEXITSTATUS(status) != 3) {
					tst_resm(TFAIL, "Child %d exited with "
						 "wrong status", ret_val);
					tst_resm(TFAIL, "Expected 3 got %d",
						 WEXITSTATUS(status));
					fail = 1;
				}
			}
			wait_kid_pid[kid_count++] = ret_val;
		}

		/*
		 * Check that for every entry in the fork_kid_pid array,
		 * there is a matching pid in the wait_kid_pid array.  If
		 * not, it's an error.
		 */
		for (i = 0; i < kid_count; i++) {
			found = 0;
			for (j = (MAXKIDS / 2); j < MAXKIDS; j++) {
				if (fork_kid_pid[j] == wait_kid_pid[i]) {
					found = 1;
					break;
				}
			}
			if (!found) {
				tst_resm(TFAIL, "Did not find a wait_kid_pid "
					 "for the fork_kid_pid of %d",
					 wait_kid_pid[i]);
				for (k = 0; k < MAXKIDS; k++) {
					tst_resm(TFAIL, "fork_kid_pid[%d] = "
						 "%d", k, fork_kid_pid[k]);
				}
				for (k = 0; k < kid_count; k++) {
					tst_resm(TFAIL, "wait_kid_pid[%d] = "
						 "%d", k, wait_kid_pid[k]);
				}
				fail = 1;
			}
		}

		if (kid_count != (MAXKIDS / 2)) {
			tst_resm(TFAIL, "Wrong number of children waited on "
				 "for pid = 0");
			tst_resm(TFAIL, "Expected 4 got %d", kid_count);
			fail = 1;
		}

		/* Make sure can pickup children in a diff. process group */

		kid_count = 0;
		errno = 0;
		while (((ret_val = waitpid(-(group1), &status, 0)) != -1) ||
		       (errno == EINTR)) {
			if (ret_val == -1) {
				continue;
			}
			if (!WIFEXITED(status)) {
				tst_resm(TFAIL, "Child %d did not exit "
					 "normally", ret_val);
				fail = 1;
			} else {
				if (WEXITSTATUS(status) != 3) {
					tst_resm(TFAIL, "Child %d exited with "
						 "wrong status", ret_val);
					tst_resm(TFAIL, "Expected 3 got %d",
						 WEXITSTATUS(status));
					fail = 1;
				}
			}
			wait_kid_pid[kid_count++] = ret_val;
		}

		/*
		 * Check that for every entry in the fork_kid_pid array,
		 * there is a matching pid in the wait_kid_pid array.  If
		 * not, it's an error.
		 */
		for (i = 0; i < kid_count; i++) {
			found = 0;
			for (j = 0; j < (MAXKIDS / 2); j++) {
				if (fork_kid_pid[j] == wait_kid_pid[i]) {
					found = 1;
					break;
				}
			}
			if (!found) {
				tst_resm(TFAIL, "Did not find a wait_kid_pid "
					 "for the fork_kid_pid of %d",
					 fork_kid_pid[j]);
				for (k = 0; k < MAXKIDS; k++) {
					tst_resm(TFAIL, "fork_kid_pid[%d] = "
						 "%d", k, fork_kid_pid[k]);
				}
				for (k = 0; k < kid_count; k++) {
					tst_resm(TFAIL, "wait_kid_pid[%d] = "
						 "%d", k, wait_kid_pid[k]);
				}
				fail = 1;
			}
		}
		if (kid_count != (MAXKIDS / 2)) {
			tst_resm(TFAIL, "Wrong number of children waited on "
				 "for pid = 0");
			tst_resm(TFAIL, "Expected 4 got %d", kid_count);
			fail = 1;
		}

		if (fail) {
			tst_resm(TFAIL, "Test FAILED");
			exit(1);
		} else {
			tst_resm(TPASS, "Test PASSED");
			exit(0);
		}
	}

	return 0;

}

/*
 * setup_sigint()
 *	sets up a SIGINT handler
 */
void setup_sigint(void)
{
	if ((sig_t) signal(SIGINT, inthandlr) == SIG_ERR) {
		tst_resm(TFAIL, "signal SIGINT failed, errno = %d", errno);
		tst_exit();
	 /*NOTREACHED*/}
}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{
	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -c option.
	 */
	TEST_PAUSE;
}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 *	completion or premature exit
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
 /*NOTREACHED*/}

void inthandlr()
{
	intintr++;
}

void wait_for_parent()
{
	int testvar;

	while (!intintr) {
		testvar = 0;
	}
}

void do_exit()
{
	wait_for_parent();
	exit(3);
}

#ifdef UCLINUX
/*
 * do_exit_uclinux()
 *	Sets up SIGINT handler again, then calls do_exit
 */
void do_exit_uclinux()
{
	setup_sigint();
	do_exit();
}
#endif
