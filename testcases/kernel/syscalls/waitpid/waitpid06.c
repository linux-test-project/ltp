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
 *	waitpid06.c
 *
 * DESCRIPTION
 *	Tests to see if pid's returned from fork and waitpid are same.
 *
 * ALGORITHM
 *	Check proper functioning of waitpid with pid = -1 and arg = 0
 *
 * USAGE:  <for command-line>
 *      waitpid06 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *      where,  -c n : Run n copies concurrently.
 *              -e   : Turn on errno logging.
 *              -i n : Execute test n times.
 *              -I x : Execute test for x seconds.
 *              -P x : Pause for x seconds between iterations.
 *              -t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *      04/2002 wjhuie sigset cleanups
 *
 * Restrictions
 *	None
 */

#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <test.h>
#include <usctest.h>

void setup(void);
void cleanup(void);

char *TCID = "waitpid06";
int TST_TOTAL = 1;
volatile int intintr;
void inthandlr();
void do_exit();
int flag;

extern int Tst_count;

#define	FAILED	1
#define	MAXKIDS	8

int main(int argc, char **argv)
{
	int lc;				/* loop counter */
	char *msg;			/* message returned from parse_opts */
	int kid_count, fork_kid_pid[MAXKIDS];
	int wait_kid_pid[MAXKIDS], ret_val, status;
	int i, j, k, found;
	int fail = 0;
	int group1, group2;
	int pid;

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, (option_t *)NULL, NULL)) !=
	    (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
		/*NOTREACHED*/
	}

	setup();

	/* check looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		if ((pid = fork()) < 0) {
			tst_resm(TINFO, "Fork Failed, may be OK under stress");
			exit(pid);
		} else if (pid == 0) {
			/*
			 * Child:
			 * Set up to catch SIGINT.  The kids will wait till a
			 * SIGINT has been received before they proceed.
			 */
			if ((sig_t)signal(SIGINT, inthandlr) == SIG_ERR ) {
				tst_resm(TFAIL, "signal SIGINT failed. "
					"errno = %d", errno);
				exit(-1);
			}

			group1 = getpgrp();
			for (kid_count = 0; kid_count < MAXKIDS; kid_count++) {
				if (kid_count == (MAXKIDS / 2)) {
					group2 = setpgrp();
				}
				intintr = 0;
				ret_val = fork();
				if (ret_val == 0) {	/* child */
					do_exit();
					/*NOTREACHED*/
				} else if (ret_val < 0) {
					tst_resm(TFAIL, "Fork kid %d failed. "
						"errno = %d", kid_count,
						errno);
					exit(ret_val);
				}

				/* parent */
				fork_kid_pid[kid_count] = ret_val;
			}

			/* Now send all the kids a SIGINT to tell them to
			 * proceed
			 */
			for (i = 0; i < MAXKIDS; i++) {
				if (kill(fork_kid_pid[i], SIGINT) < 0) {
					tst_resm(TFAIL, "Kill of child %d "
						"failed, errno = %d", i, errno);
					exit(-1);
				}
			}

			/*
			 * Wait till all kids have terminated.  Stash away their
			 * pid's in an array.
			 */
			kid_count = 0;
			errno = 0;
			while (((ret_val = waitpid(-1, &status, 0)) != -1) ||
			       (errno == EINTR)) {
				if (ret_val == -1) {
					continue;
				}

				if (!WIFEXITED(status)) {
					tst_resm(TFAIL, "Child %d did not exit "
						 "normally", ret_val);
					flag = FAILED;
				} else {
					if (WEXITSTATUS(status) != 3) {
						tst_resm(TFAIL, "Child %d"
							"exited with wrong "
							"status", ret_val);
						tst_resm(TFAIL, "Expected 3 "
								"got %d ",
							 WEXITSTATUS(status));
						flag = FAILED;
					}
				}
				wait_kid_pid[kid_count++] = ret_val;
			}

			/*
			 * Check that for every entry in the fork_kid_pid array,
			 * there is a matching pid in the wait_kid_pid array. If
			 * not, it's an error.
			 */
			for (i = 0; i < kid_count; i++) {
				found = 0;
				for (j = 0; j < MAXKIDS; j++) {
					if (fork_kid_pid[j] == wait_kid_pid[i]){
						found = 1;
						break;
					}
				}

				if (!found) {
					tst_resm(TFAIL, "Did not find a "
						"wait_kid_pid for the "
						"fork_kid_pid of %d",
						 fork_kid_pid[j]);
					for (k = 0; k < MAXKIDS; k++) {
						tst_resm(TFAIL,
							"fork_kid_pid[%d] = "
							"%d", k,
							fork_kid_pid[k]);
					}
					for (k = 0; k < kid_count; k++) {
						tst_resm(TFAIL,
							"wait_kid_pid[%d] = "
							"%d", k,
							wait_kid_pid[k]);
					}
					flag = FAILED;
				}
			}

			if (flag) {
				exit(1);
			} else {
				exit(0);
			}
		} else {		/* parent */
			fail = 0;
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
		}
	}
	cleanup();
	/*NOTREACHED*/

  return(0);

}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void
setup(void)
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
void
cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
	/*NOTREACHED*/
}

void
inthandlr()
{
	intintr++;
}

void
wait_for_parent()
{
	int testvar;

	while (!intintr) {
		testvar = 0;
	}
}

void
do_exit()
{
	wait_for_parent();
	exit(3);
}
