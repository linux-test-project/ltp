
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"

void setup();
void help();
void cleanup();

char *TCID = "sigpending02";
int TST_TOTAL = 1;

/***********************************************************************
 * Main
 ***********************************************************************/
int main(int ac, char **av)
{
	int lc;
	sigset_t *sigset;

	tst_parse_opts(ac, av, NULL, NULL);

    /***************************************************************
     * perform global setup for test
     ***************************************************************/
	setup();

	/* set sigset to point to an invalid location */
	sigset = (sigset_t *) - 1;

    /***************************************************************
     * check looping state
     ***************************************************************/
	/* TEST_LOOPING() is a macro that will make sure the test continues
	 * looping according to the standard command line args.
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		TEST(sigpending(sigset));

		/* check return code */
		if (TEST_RETURN == -1) {
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
	}

    /***************************************************************
     * cleanup and exit
     ***************************************************************/
	cleanup();
	tst_exit();

}

/***************************************************************
 * help
 ***************************************************************/
void help(void)
{
	printf("test\n");
}

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup(void)
{
	TEST_PAUSE;
}

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 ***************************************************************/
void cleanup(void)
{
}
