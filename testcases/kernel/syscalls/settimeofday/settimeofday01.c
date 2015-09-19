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
 * 	settimeofday01.c
 *
 * DESCRIPTION
 *	Testcase to check the basic functionality of settimeofday().
 *
 * ALGORITHM
 *	Setup:
 *	  Setup signal handling.
 *	  Check that we are the proper user.
 *	  Setup expected errnos.
 *	  Pause for SIGUSER1 if option specified.
 *	  Save the current time values.
 *	Loop if the proper options are given.
 *	  Call settimeofday and verify the time was changed.
 *	  Call settimeofday with invalid Args and verify that the call fails.
 *	Cleanup:
 *	  Restore the original time values.
 *	  Print errno log and/or timing stats if options given.
 *
 * USAGE:  <for command-line>
 *	settimeofday01 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions
 * 	Must be run as root.
 */

#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include "test.h"

#define	FAILED		1
#define	VAL_SEC		100
#define	VAL_MSEC	100
#define ACCEPTABLE_DELTA	500	/* in milli-seconds */

char *TCID = "settimeofday01";
int TST_TOTAL = 1;
time_t save_tv_sec, save_tv_usec;
struct timeval tp, tp1, tp2;

void setup(void);
void cleanup(void);

#if !defined(UCLINUX)

int main(int argc, char **argv)
{
	int lc;
	suseconds_t delta;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		int condition_number = 1;
		/* reset tst_count in case we are looping */
		tst_count = 0;

		gettimeofday(&tp, NULL);
		tp.tv_sec += VAL_SEC;
		tp.tv_usec += VAL_MSEC;

		TEST(settimeofday(&tp, NULL));
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "Error Setting Time, errno=%d",
				 TEST_ERRNO);
		}

		if ((gettimeofday(&tp2, (struct timezone *)&tp1)) == -1) {
			tst_resm(TBROK, "Error Getting Time, errno=%d", errno);
		}

		if (tp2.tv_sec > tp.tv_sec) {
			delta =
			    (suseconds_t) (tp2.tv_sec - tp.tv_sec) * 1000 +
			    (tp2.tv_usec - tp.tv_usec) / 1000;
		} else {
			delta =
			    (suseconds_t) (tp.tv_sec - tp2.tv_sec) * 1000 +
			    (tp.tv_usec - tp2.tv_usec) / 1000;
		}

		if (delta > -ACCEPTABLE_DELTA && delta < ACCEPTABLE_DELTA) {
			tst_resm(TPASS, "Test condition %d successful",
				 condition_number++);
		} else {
			tst_resm(TFAIL, "Test condition %d failed",
				 condition_number++);
		}

		/* Invalid Args : Error Condition where tp = NULL */
		TEST(settimeofday((struct timeval *)-1, NULL));
		if (TEST_RETURN == -1) {
			tst_resm(TPASS, "Test condition %d successful",
				 condition_number++);
		} else {
			tst_resm(TFAIL, "Test condition %d failed",
				 condition_number);
		}

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
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{
	tst_require_root();

	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -c option.
	 */
	TEST_PAUSE;

	/* Save the current time values */
	if ((gettimeofday(&tp, (struct timezone *)&tp1)) == -1) {
		tst_brkm(TBROK, cleanup, "gettimeofday failed. "
			 "errno=%d", errno);
	}
	save_tv_sec = tp.tv_sec;
	save_tv_usec = tp.tv_usec;
}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 *	completion or premature exit
 */
void cleanup(void)
{
	/* restore the original time values. */
	tp.tv_sec = save_tv_sec;
	tp.tv_usec = save_tv_usec;
	if ((settimeofday(&tp, NULL)) == -1) {
		tst_resm(TWARN, "FATAL COULD NOT RESET THE CLOCK");
		tst_resm(TFAIL, "Error Setting Time, errno=%d", errno);
	}

}
