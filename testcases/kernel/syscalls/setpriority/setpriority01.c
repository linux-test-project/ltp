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
 *	setpriority01.c
 *
 * DESCRIPTION
 *	setpriority01 - set the priority for the test process lower.
 *
 * CALLS
 *	setpriority()
 *
 * ALGORITHM
 *	loop if that option was specified
 *	issue the system call
 *	read back the priority with getpriority()
 *	check the errno value
 *	  if errno == 0 and the new priority == the set priority
 *	  then issue a PASS message
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	  break any remaining tests
 *	  call cleanup
 *
 * USAGE:  <for command-line>
 *  setpriority01 [-c n] [-f] [-i n] [-I x] [-p x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *
 * RESTRICTIONS
 *	none
 */

#include "test.h"
#include "usctest.h"

#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>

void cleanup(void);
void setup(void);

char *TCID = "setpriority01";
int TST_TOTAL = 1;
extern int Tst_count;

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int priority;
	int new_val = 2;	/* lower our priority from 0 to new_val */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/*
		 * Lower the priority of this process from a default of 0
		 * to a new value of 2.  Then read the value using getpriority()
		 * to verify the change.
		 */

		/*
		 * We need to be careful here. We could possibly set our
		 * priority lower using PRIO_USER and then have no way
		 * to set it back.  So, let's use PRIO_PROCESS and make
		 * sure we affect this test only.
		 */

		/* call the system call with the TEST() macro */
		TEST(setpriority(PRIO_PROCESS, 0, new_val));

		if (TEST_RETURN != 0) {
			tst_resm(TFAIL, "call failed - errno = %d - "
				 "%s", TEST_ERRNO, strerror(TEST_ERRNO));
			continue;
		}

		if (STD_FUNCTIONAL_TEST) {
			/* get the priority that we just set */
			priority = getpriority(PRIO_PROCESS, 0);

			if (errno == -1) {
				tst_brkm(TBROK, cleanup, "getpriority call "
					 "failed - errno = %d - %s", errno,
					 strerror(errno));
			}

			if (priority == new_val) {
				tst_resm(TPASS, "functionality is correct");
			} else {
				tst_resm(TFAIL, "current priority (%d) and new "
					 "priority (%d) do not match",
					 priority, new_val);
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}
	}

	cleanup();

	 /*NOTREACHED*/ return 0;

}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
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

	/* exit with return code appropriate for results */
	tst_exit();
}
