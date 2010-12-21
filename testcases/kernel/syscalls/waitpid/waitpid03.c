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
 *	waitpid03.c
 *
 * DESCRIPTION
 *	Check that parent waits unitl specific child has returned.
 *
 * ALGORITHM
 *	Parent forks numerous (22 = MAXUPRC - 3) children, and starts waits :
 *	Should only wait for the specific child, a second wait on the same
 *	child should return with -1 and not one of the other zombied
 *	children.
 *
 * USAGE:  <for command-line>
 *      waitpid03 [-c n] [-i n] [-I x] [-P x] [-t]
 *      where,  -c n : Run n copies concurrently.
 *              -i n : Execute test n times.
 *              -I x : Execute test for x seconds.
 *              -P x : Pause for x seconds between iterations.
 *              -t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *	04/2002 wjhuie sigset cleanups
 *
 * Restrictions
 *	None
 */

#define DEBUG 0

#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include "test.h"
#include "usctest.h"

void do_child(int);
void setup(void);
void cleanup(void);

char *TCID = "waitpid03";
int TST_TOTAL = 1;

#define	MAXUPRC	25

int condition_number;

#ifdef UCLINUX
void do_child_uclinux(void);
static int ikids_uclinux;
#endif

int main(int argc, char **argv)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	int status, pid[25], ret;

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, NULL, NULL)) !=
	    NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	 }
#ifdef UCLINUX
	maybe_run_child(&do_child, "d", &ikids_uclinux);
#endif

	setup();

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		int ikids = 0;
		Tst_count = 0;

		/*
		 * Set SIGTERM to SIG_DFL as test driver sets up to ignore
		 * SIGTERM
		 */
		if ((sig_t) signal(SIGTERM, SIG_DFL) == SIG_ERR) {
			tst_resm(TFAIL, "Signal SIGTERM failed, errno = %d",
				 errno);

		}

		while (++ikids < MAXUPRC) {
			if ((pid[ikids] = FORK_OR_VFORK()) > 0) {
				if (DEBUG)
					tst_resm(TINFO, "child # %d", ikids);
			} else if (pid[ikids] == -1) {
				tst_resm(TFAIL, "cannot open fork #%d", ikids);
			} else {
#ifdef UCLINUX
				if (self_exec(argv[0], "d", ikids) < 0) {
					tst_resm(TFAIL, "cannot self_exec #%d",
						 ikids);
				}
#else
				do_child(ikids);
#endif
			}
		}

		for (ikids = 1; ikids < MAXUPRC; ikids++) {
			if (DEBUG)
				tst_resm(TINFO, "Killing #%d", ikids);
			kill(pid[ikids], 15);
		}

		condition_number = 1;

		/* Wait on one specific child */
		if (DEBUG)
			tst_resm(TINFO, "Waiting for child:#%d", MAXUPRC / 2);
		ret = waitpid(pid[MAXUPRC / 2], &status, 0);
		if (ret != pid[MAXUPRC / 2]) {
			tst_resm(TFAIL, "condition %d test failed. "
				 "waitpid(%d) returned %d.",
				 condition_number, pid[MAXUPRC / 2], ret);
		} else {
			tst_resm(TPASS, "Got correct child PID");
		}
		condition_number++;

		/*
		 * Child has already been waited on, waitpid should return
		 * -1
		 */
		ret = waitpid(pid[MAXUPRC / 2], &status, 0);
		if (ret != -1) {
			tst_resm(TFAIL, "condition %d test failed",
				 condition_number);
		} else {
			tst_resm(TPASS, "Condition %d test passed",
				 condition_number);
		}
		condition_number++;
	}
	cleanup();
	tst_exit();

}

/*
 * do_child()
 */
void do_child(int ikids)
{
	if (DEBUG)
		tst_resm(TINFO, "child:%d", ikids);
	pause();
	exit(0);
}

#ifdef UCLINUX
/*
 * do_child_uclinux()
 *	run do_child with the appropriate ikids variable
 */
void do_child_uclinux()
{
	do_child(ikids_uclinux);
}
#endif

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

 }