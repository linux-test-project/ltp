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
 *	wait401.c
 *
 * DESCRIPTION
 *	wait401 - check that a call to wait4() correctly waits for a child
 *		  process to exit
 *
 * ALGORITHM
 *	loop if that option was specified
 *	fork a child.
 *	issue the system call
 *	check the return value
 *	  if return value == -1
 *	    issue a FAIL message, break remaining tests and cleanup
 *	  if we are doing functional testing
 *	    issue a PASS message if the wait4 call returned the child's pid
 *	  else
 *	    issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *      wait401 [-c n] [-f] [-e] [-i n] [-I x] [-P x] [-t]
 *      where,  -c n : Run n copies concurrently.
 *		-f   : Turn off functionality Testing.
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
 *	none
 */

#include "test.h"
#include "usctest.h"

#include <errno.h>
#define _USE_BSD
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>

void cleanup(void);
void setup(void);

char *TCID = "wait401";
int TST_TOTAL = 1;

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	pid_t pid;
	int status = 1;
	struct rusage *rusage = NULL;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/*
		 * Allocate some space for the rusage structure
		 */

		if ((rusage = (struct rusage *)malloc(sizeof(struct rusage)))
		    == NULL) {
			tst_brkm(TBROK, cleanup, "malloc() failed");
		}

		pid = FORK_OR_VFORK();

		if (pid == -1) {
			tst_brkm(TBROK, cleanup, "fork() failed");
		}

		if (pid == 0) {	/* this is the child */
			/*
			 * sleep for a moment to let us do the test
			 */
			sleep(1);
			exit(0);
		} else {	/* this is the parent */

			/* call wait4 with the TEST() macro */
			TEST(wait4(pid, &status, 0, rusage));
		}

		if (TEST_RETURN == -1) {
			tst_brkm(TFAIL, cleanup, "%s call failed - errno = %d "
				 ": %s", TCID, TEST_ERRNO,
				 strerror(TEST_ERRNO));
		}

		if (STD_FUNCTIONAL_TEST) {
			/*
			 * The return from this call should be non-zero.
			 */
			if (WIFEXITED(status) == 0) {
				tst_brkm(TFAIL, cleanup,
					 "%s call succeeded but "
					 "WIFEXITED() did not return expected value "
					 "- %d", TCID, WIFEXITED(status));
			} else if (TEST_RETURN != pid) {
				tst_resm(TFAIL, "%s did not return the "
					 "expected value (%d), actual: %ld", TCID,
					 pid, TEST_RETURN);
			} else {

				tst_resm(TPASS,
					 "Received child pid as expected.");
			}
		}
		tst_resm(TPASS, "%s call succeeded", TCID);

		/*
		 * Clean up things in case we are looping.
		 */
		free(rusage);
		rusage = NULL;
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
 *	       or premature exit.
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}
