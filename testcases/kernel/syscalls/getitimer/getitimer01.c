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
 *	getitimer01.c
 *
 * DESCRIPTION
 *	getitimer01 - check that a correct call to getitimer() succeeds
 *
 * ALGORITHM
 *	loop if that option was specified
 *	allocate needed space
 *	issue the system call
 *	check the errno value
 *	  issue a PASS message if we get zero
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	  break any remaining tests
 *	  call cleanup
 *
 * USAGE:  <for command-line>
 *  getitimer01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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

void cleanup(void);
void setup(void);

char *TCID = "getitimer01";
int TST_TOTAL = 1;

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	struct itimerval *value;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* allocate space for the itimerval structure */

		if ((value = (struct itimerval *)malloc((size_t)
							sizeof(struct
							       itimerval))) ==
		    NULL) {
			tst_brkm(TBROK, cleanup, "value malloc failed");
		}

		/* call the system call with the TEST() macro */

		TEST(getitimer(ITIMER_REAL, value));

		if (TEST_RETURN != 0) {
			tst_resm(TFAIL, "call failed - errno = %d - %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		}

		if (STD_FUNCTIONAL_TEST) {

			/*
			 * Since ITIMER_REAL is effectively disabled (we did
			 * not set it before the getitimer call), the elements
			 * in it_value should be zero.
			 */
			if ((value->it_value.tv_sec == 0) &&
			    (value->it_value.tv_usec == 0)) {
				tst_resm(TPASS, "functional test passed");
			} else {
				tst_resm(TFAIL, "timer values are non zero");
			}
		} else {
			tst_resm(TPASS, "call succeeded");
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

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}