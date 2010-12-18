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
 *	getitimer02.c
 *
 * DESCRIPTION
 *	getitimer02 - check that a getitimer() call fails as expected
 *		      with an incorrect second argument.
 *
 * ALGORITHM
 *	loop if that option was specified
 *	call getitimer() with an incorrect second argument.
 *	check the errno value
 *	  issue a PASS message if we get EFAULT - errno 14
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	  break any remaining tests
 *	  call cleanup
 *
 * USAGE:  <for command-line>
 *  getitimer02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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

char *TCID = "getitimer02";
int TST_TOTAL = 1;

#if !defined(UCLINUX)

void cleanup(void);
void setup(void);

int exp_enos[] = { EFAULT, 0 };

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

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
		 * issue the system call with the TEST() macro
		 * ITIMER_REAL = 0, ITIMER_VIRTUAL = 1 and ITIMER_PROF = 2
		 */

		/* call with a bad address */
		TEST(getitimer(ITIMER_REAL, (struct itimerval *)-1));

		if (TEST_RETURN == 0) {
			tst_resm(TFAIL, "call failed to produce "
				 "expected error - errno = %d - %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			continue;
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		switch (TEST_ERRNO) {
		case EFAULT:
			tst_resm(TPASS, "expected failure - errno = %d - %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			break;
		default:
			tst_resm(TFAIL, "call failed to produce "
				 "expected error - errno = %d - %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
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

#else

int main(void)
{
	tst_resm(TINFO, "test is not available on uClinux");
	tst_exit();
}

#endif /* if !defined(UCLINUX) */