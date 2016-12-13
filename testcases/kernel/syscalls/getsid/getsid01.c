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
 *	getsid01.c
 *
 * DESCRIPTION
 *	getsid01 - call getsid() and make sure it succeeds
 *
 * ALGORITHM
 *	loop if that option was specified
 *	issue the system call
 *	check the return value
 *	if return value == -1
 *	  issue a FAIL message, break remaining tests and cleanup
 *	if we are doing functional testing
 *	  save the return value from the getsid() call - the session ID
 *	  fork a child and get the child's session ID
 *	  if the child's session ID == the parent's session ID
 *	    issue a PASS message
 *	  else
 *	    issue a FAIL message
 *	else issue a PASS message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  getsid01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	none
 */
#define _GNU_SOURCE 1

#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include "test.h"

void cleanup(void);
void setup(void);

char *TCID = "getsid01";
int TST_TOTAL = 1;

pid_t p_sid;

int main(int ac, char **av)
{
	int lc;
	pid_t pid, c_pid, c_sid;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		/* call the system call with the TEST() macro */

		TEST(getsid(0));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "call failed - errno = %d "
				 "- %s", TEST_ERRNO, strerror(TEST_ERRNO));
			continue;
		}

		/* save the return value in a global variable */
		p_sid = TEST_RETURN;

		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "could not fork");
		}

		if (pid == 0) {	/* child */
			if ((c_pid = getpid()) == -1) {
				tst_resm(TINFO, "getpid failed in "
					 "functionality test");
				exit(1);
			}

			/*
			 * if the session ID of the child is the
			 * same as the parent then things look good
			 */

			if ((c_sid = getsid(0)) == -1) {
				tst_resm(TINFO, "getsid failed in "
					 "functionality test");
				exit(2);
			}

			if (c_sid == p_sid) {
				tst_resm(TPASS, "session ID is "
					 "correct");
			} else {
				tst_resm(TFAIL, "session ID is "
					 "not correct");
			}
			exit(0);

		} else {
			waitpid(pid, NULL, 0);
		}
	}

	cleanup();
	tst_exit();
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{

}
