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
/**************************************************************************
 * 
 *    TEST IDENTIFIER	: munlock02
 * 
 *    EXECUTED BY	: root / superuser
 * 
 *    TEST TITLE	: Test for checking basic error conditions for
 *    			  munlock(2)
 * 
 *    TEST CASE TOTAL	: 2
 * 
 *    AUTHOR		: Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 * 
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 * 	Check for basic errors returned by munlock(2) system call.
 *
 * 	Verify that munlock(2) returns -1 and sets errno to
 *
 * 	1) ENOMEM - If process exceed maximum  number of locked pages.
 * 
 * 	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 * 
 * 	Test:
 *	 Loop if the proper options are given.
 *	  Do necessary setup for each test.
 *	  Execute system call
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	  Otherwise, Issue a PASS message.
 * 
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 * 
 * USAGE:  <for command-line>
 *  munlock02 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
 *		where,		-c n : Run n copies concurrently
 *				-e   : Turn on errno logging.
 *				-h   : Show this help screen
 *				-i n : Execute test n times.
 *				-I x : Execute test for x seconds.
 *				-p   : Pause for SIGUSR1 before starting
 *				-P x : Pause for x seconds between iterations.
 *				-t   : Turn on syscall timing.
 *
 * RESTRICTIONS
 *	Test must run as root.
 *****************************************************************************/
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pwd.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

char *TCID = "munlock02";		/* Test program identifier.    */
int TST_TOTAL = 1;			/* Total number of test cases. */
extern int Tst_count;			/* TestCase counter for tst_* routine */

int exp_enos[] = { ENOMEM, 0 };
#define LEN	1024

void *addr1;

struct test_case_t {
	void **addr;
	int len;
	int error;
	char *edesc;
} TC[] = {
	{&addr1, LEN * 10, ENOMEM, "address range out of address space" },
};

int main(int ac, char **av)
{
	int lc, i;		/* loop counter */
	char *msg;		/* message returned from parse_opts */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != (char *) NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/* perform global setup for test */
	setup();

	/* check looping state */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			TEST(munlock(*(TC[i].addr), TC[i].len));

			/* check return code */
			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				if (TEST_ERRNO != TC[i].error)
					tst_brkm(TFAIL, cleanup,
						 "munlock() Failed with wrong "
						 "errno, expected errno=%s, "
						 "got errno=%d : %s",
						 TC[i].edesc, TEST_ERRNO,
						 strerror(TEST_ERRNO));
				else
					tst_resm(TPASS,
						 "expected failure - errno "
						 "= %d : %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
			} else {
				tst_brkm(TFAIL, cleanup,
					 "munlock() Failed, expected "
					 "return value=-1, got %d",
					 TEST_RETURN);
			}
		}
	}			/* End for TEST_LOOPING */

	/* cleanup and exit */
	cleanup();

	return 0;
}				/* End main */


/* setup() - performs all ONE TIME setup for this test. */

void setup()
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	addr1 = (char *) malloc(LEN);
	if (addr1 == NULL)
		tst_brkm(TFAIL, cleanup, "malloc failed");
	TEST(mlock(addr1, LEN));

	/* check return code */
	if (TEST_RETURN == -1) {
		tst_brkm(TFAIL, cleanup, "mlock(%p, %d) Failed with return=%d,"
			"errno=%d : %s", addr1, LEN, TEST_RETURN,
			TEST_ERRNO, strerror(TEST_ERRNO));
	}

	TEST_PAUSE;

	return;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup()
{
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();

	return;
}
