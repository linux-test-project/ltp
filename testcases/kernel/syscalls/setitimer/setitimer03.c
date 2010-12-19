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
 *	setitimer03.c
 *
 * DESCRIPTION
 *	setitimer03 - check that a setitimer() call fails as expected
 *		      with incorrect values.
 *
 * ALGORITHM
 *	loop if that option was specified
 *	allocate needed space and set up needed values
 *	issue the system call
 *	check the errno value
 *	  issue a PASS message if we get EINVAL
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	  break any remaining tests
 *	  call cleanup
 *
 * USAGE:  <for command-line>
 *  setitimer03 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
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

char *TCID = "setitimer03";
int TST_TOTAL = 1;

int exp_enos[] = { EINVAL, 0 };

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	struct itimerval *value, *ovalue;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* allocate some space for timer structures */

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
			tst_brkm(TBROK, cleanup, "value malloc failed");
		}

		/* set up some reasonable values */

		value->it_value.tv_sec = 30;
		value->it_value.tv_usec = 0;
		value->it_interval.tv_sec = 0;
		value->it_interval.tv_usec = 0;

		/*
		 * issue the system call with the TEST() macro
		 * ITIMER_REAL = 0, ITIMER_VIRTUAL = 1 and ITIMER_PROF = 2
		 */

		/* make the first value negative to get a failure */
		TEST(setitimer(-ITIMER_PROF, value, ovalue));

		if (TEST_RETURN == 0) {
			tst_resm(TFAIL, "call failed to produce expected error "
				 "- errno = %d - %s", TEST_ERRNO,
				 strerror(TEST_ERRNO));
			continue;
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		switch (TEST_ERRNO) {
		case EINVAL:
			tst_resm(TPASS, "expected failure - errno = %d - %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			break;
		default:
			tst_resm(TFAIL, "call failed to produce expected error "
				 "- errno = %d - %s", TEST_ERRNO,
				 strerror(TEST_ERRNO));
		}

		/*
		 * clean up things in case we are looping
		 */
		free(value);
		free(ovalue);
		value = NULL;
		ovalue = NULL;
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

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

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