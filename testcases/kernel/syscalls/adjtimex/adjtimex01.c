/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */
/**********************************************************
 *
 *    TEST IDENTIFIER	: adjtimex01
 *
 *    EXECUTED BY	: root / superuser
 *
 *    TEST TITLE	: Basic test for adjtimex(2)
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	This is a Phase I test for the adjtimex(2) system call.
 *	It is intended to provide a limited exposure of the system call.
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *	  Save current parameters in tim_save
 *
 * 	Test:
 *	 Loop if the proper options are given.
 * 	  call adjtimex with saved timex structure
 *	  Check return value is between 0 & 5
 *		Test passed
 *	  Otherwise
 *		Test failed
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * adjtimex01 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
 *			where,  -c n : Run n copies concurrently.
 *				-e   : Turn on errno logging.
 *				-h   : Show help screen
 *				-f   : Turn off functional testing
 *				-i n : Execute test n times.
 *				-I x : Execute test for x seconds.
 *				-p   : Pause for SIGUSR1 before starting
 *				-P x : Pause for x seconds between iterations.
 *				-t   : Turn on syscall timing.
 *
 ****************************************************************/

#if defined UCLINUX && !__THROW
/* workaround for libc bug causing failure in sys/timex.h */
#define __THROW
#endif

#include <errno.h>
#include <sys/timex.h>
#include "test.h"
#include "usctest.h"

#define SET_MODE ( ADJ_OFFSET | ADJ_FREQUENCY | ADJ_MAXERROR | ADJ_ESTERROR | \
	ADJ_STATUS | ADJ_TIMECONST | ADJ_TICK )

static void setup();
static void cleanup();

char *TCID = "adjtimex01";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

static struct timex tim_save;

int main(int ac, char **av)
{

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL))
	    != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* perform global setup for test */
	setup();

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/* Call adjtimex(2) */
		tim_save.modes = SET_MODE;

		TEST(adjtimex(&tim_save));

		if ((TEST_RETURN >= 0) && (TEST_RETURN <= 5)) {
			tst_resm(TPASS, "adjtimex() returned %ld", TEST_RETURN);
		} else {
			tst_resm(TFAIL|TTERRNO, "Test Failed, adjtimex()"
				 "returned %ld", TEST_RETURN);
		}
	}			/* End for TEST_LOOPING */

	/* cleanup and exit */
	cleanup();

	 /*NOTREACHED*/ return 0;

}				/* End main */

/* setup() - performs all ONE TIME setup for this test */
void setup()
{
	/* Check whether we are root */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}

	tim_save.modes = 0;

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Save current parameters in tim_save */
	if ((adjtimex(&tim_save)) == -1) {
		tst_brkm(TBROK, cleanup, "Failed to save current parameters");
	}
}				/* End setup() */

/*
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
