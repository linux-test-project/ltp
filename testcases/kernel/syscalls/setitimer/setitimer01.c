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
 *	setitimer01.c
 *
 * DESCRIPTION
 *	setitimer01 - check that a resonable setitimer() call succeeds.
 *
 * ALGORITHM
 *	loop if that option was specified
 *	allocate needed space and set up needed values
 *	issue the system call
 *	check the errno value
 *	  issue a PASS message if we get zero
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	  break any remaining tests
 *	  call cleanup
 *
 * USAGE:  <for command-line>
 *  setitimer01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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

char *TCID = "setitimer01";
int TST_TOTAL = 1;
extern int Tst_count;

#define SEC0	0
#define SEC1	20
#define SEC2	40

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	struct itimerval *value, *ovalue;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* allocate some space for the timer structures */

		if ((value = (struct itimerval *)malloc((size_t)
							sizeof(struct
							       itimerval))) ==
		    NULL) {
			tst_brkm(TBROK, cleanup, "value malloc failed");
		}

		if ((ovalue = (struct itimerval *)malloc((size_t)
							 sizeof(struct
								itimerval))) ==
		    NULL) {
			tst_brkm(TBROK, cleanup, "ovalue malloc failed");
		}

		/* set up some reasonable values */

		value->it_value.tv_sec = SEC1;
		value->it_value.tv_usec = SEC0;
		value->it_interval.tv_sec = 0;
		value->it_interval.tv_usec = 0;
		/*
		 * issue the system call with the TEST() macro
		 * ITIMER_REAL = 0, ITIMER_VIRTUAL = 1 and ITIMER_PROF = 2
		 */

		TEST(setitimer(ITIMER_REAL, value, ovalue));

		if (TEST_RETURN != 0) {
			tst_resm(TFAIL, "call failed - errno = %d - %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			continue;
		}

		if (STD_FUNCTIONAL_TEST) {
			/*
			 * call setitimer again with new values.
			 * the old values should be stored in ovalue
			 */
			value->it_value.tv_sec = SEC2;
			value->it_value.tv_usec = SEC0;

			if ((setitimer(ITIMER_REAL, value, ovalue)) == -1) {
				tst_brkm(TBROK, cleanup, "second setitimer "
					 "call failed");
			}

			if (ovalue->it_value.tv_sec <= SEC1) {
				tst_resm(TPASS, "functionality is correct");
			} else {
				tst_brkm(TFAIL, cleanup, "old timer value is "
					 "not equal to expected value");
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
 * 	       or premature exit.
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
