
/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
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
 * 	sigpending02.c
 *
 * DESCRIPTION
 * 	Test to see the the proper errors are returned by sigpending
 *$
 * ALGORITHM
 * 	test 1:
 * 	Call sigpending(sigset_t*=-1), it should return -1 with errno EFAULT
 *
 * USAGE:  <for command-line>
 *         -c n    Run n copies concurrently
 *         -e      Turn on errno logging
 *         -f      Turn off functional testing
 *         -h      Show this help screen
 *         -i n    Execute test n times
 *         -I x    Execute test for x seconds
 *         -p      Pause for SIGUSR1 before starting
 *         -P x    Pause for x seconds between iterations
 *         -t      Turn on syscall timing
 *
 * HISTORY
 *	02/2002 Written by Paul Larson
 *
 * RESTRICTIONS
 * 	None
 */
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"
#include "usctest.h"

void setup();
void help();
void cleanup();

char *TCID = "sigpending02";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
extern int Tst_nobuf;

int exp_enos[] = { EFAULT, 0 };

/***********************************************************************
 * Main
 ***********************************************************************/
int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	sigset_t *sigset;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

    /***************************************************************
     * perform global setup for test
     ***************************************************************/
	setup();

	/* set sigset to point to an invalid location */
	sigset = (sigset_t *) - 1;

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

    /***************************************************************
     * check looping state
     ***************************************************************/
	/* TEST_LOOPING() is a macro that will make sure the test continues
	 * looping according to the standard command line args.
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		TEST(sigpending(sigset));

		/* check return code */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			if (TEST_ERRNO != EFAULT)
				tst_brkm(TFAIL, cleanup,
					 "sigpending() Failed with wrong "
					 "errno, expected errno=%d, got errno=%d : %s",
					 EFAULT, TEST_ERRNO,
					 strerror(TEST_ERRNO));
			else
				tst_resm(TPASS,
					 "expected failure - errno = %d : %s",
					 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			tst_brkm(TFAIL, cleanup,
				 "sigpending() Failed, expected "
				 "return value=-1, got %ld", TEST_RETURN);
		}
	}			/* End for TEST_LOOPING */

    /***************************************************************
     * cleanup and exit
     ***************************************************************/
	cleanup();

	return 0;
}				/* End main */

/***************************************************************
 * help
 ***************************************************************/
void help()
{
	printf("test\n");
}

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup()
{
	TEST_PAUSE;
}

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 ***************************************************************/
void cleanup()
{
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}
