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
 *	setitimer02.c
 *
 * DESCRIPTION
 *	setitimer02 - check that a setitimer() call fails as expected
 *		      with incorrect values.
 *
 * ALGORITHM
 *	loop if that option was specified
 *	allocate needed space and set up needed values
 *	issue the system call
 *	check the errno value
 *	  issue a PASS message if we get EFAULT
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	  break any remaining tests
 *	  call cleanup
 *
 * USAGE:  <for command-line>
 *  setitimer02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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

#include <errno.h>
#include <sys/time.h>

void cleanup(void);
void setup(void);

char *TCID = "setitimer02";
int TST_TOTAL = 1;

#if !defined(UCLINUX)

int main(int ac, char **av)
{
	int lc;
	struct itimerval *value;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		/* allocate some space for a timer structure */
		if ((value = malloc((size_t)sizeof(struct itimerval))) ==
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

		/* call with a bad address */
		TEST(setitimer(ITIMER_REAL, value, (struct itimerval *)-1));

		if (TEST_RETURN == 0) {
			tst_resm(TFAIL, "call failed to produce EFAULT error "
				 "- errno = %d - %s", TEST_ERRNO,
				 strerror(TEST_ERRNO));
			continue;
		}

		switch (TEST_ERRNO) {
		case EFAULT:
			tst_resm(TPASS, "expected failure - errno = %d - %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			break;
		default:
			tst_resm(TFAIL, "call failed to produce EFAULT error "
				 "- errno = %d - %s", TEST_ERRNO,
				 strerror(TEST_ERRNO));
		}

		/*
		 * clean up things in case we are looping
		 */
		free(value);
		value = NULL;
	}

	cleanup();
	tst_exit();

}

#else

int main(void)
{
	tst_resm(TINFO, "test is not available on uClinux");
	tst_exit();
}

#endif /* if !defined(UCLINUX) */

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

}
