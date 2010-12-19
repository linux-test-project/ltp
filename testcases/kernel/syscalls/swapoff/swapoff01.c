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
 *    TEST IDENTIFIER	: swapoff01
 *
 *    EXECUTED BY	: root / superuser
 *
 *    TEST TITLE	: Basic test for swapoff(2)
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *    This is a Phase I test for the swapoff(2) system call.
 *    It is intended to provide a limited exposure of the system call.
 *   $
 *
 * 	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *	  Create a temporary directory.
 *	  Create file of size 40K ( minimum swapfile size).
 *	  Make this file as swap file using mkswap(8)
 *	  Turn on the swap file.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	  Execute system call
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	  Otherwise, Issue a PASS message.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * swapoff01 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
 * where:
 * 	-c n : Run n copies simultaneously.
 *	-e   : Turn on errno logging.
 *	-i n : Execute test n times.
 *	-I x : Execute test for x seconds.
 *	-p   : Pause for SIGUSR1 before starting
 *	-P x : Pause for x seconds between iterations.
 *	-t   : Turn on syscall timing.
 *
 *RESTRICTIONS:
 * Not compatible with kernel versions below 1.3.2.
 *
 *CHANGES:
 * 2005/01/01  Add extra check to stop test if insufficient disk space in dir
 *             -Ricky Ng-Adam (rngadam@yahoo.com)
 * 2005/01/01  Add extra check to stop test if swap file is on tmpfs
 *             -Ricky Ng-Adam (rngadam@yahoo.com)
 *****************************************************************************/

#include <unistd.h>
#include "test.h"
#include "usctest.h"
#include <errno.h>
#include <stdlib.h>
#include "config.h"
#include "linux_syscall_numbers.h"
#include "swaponoff.h"

static void setup();
static void cleanup();

char *TCID = "swapoff01";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

int main(int ac, char **av)
{

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		if (syscall(__NR_swapon, "./swapfile01", 0) != 0) {
			tst_resm(TWARN, "Failed to turn on the swap file"
				 ", skipping test iteration");
			continue;
		}

		TEST(syscall(__NR_swapoff, "./swapfile01"));

		/* check return code */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL, "swapoff(2) Failed to turn off"
				 " swapfile. System reboot after execution"
				 " of LTP test suite is recommended.");
			tst_resm(TWARN, "It is recommended not to run"
				 " swapon01 and swapon02");
		} else {
			tst_resm(TPASS, "swapoff(2) passed and turned off"
				 " swapfile.");
		}
	}			/*End for TEST_LOOPING */

	/*Clean up and exit */
	cleanup();
	tst_exit();

}

/* setup() - performs all ONE TIME setup for this test */
void setup()
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Check whether we are root */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Test must be run as root");
	}

	TEST_PAUSE;

	tst_tmpdir();

	if (tst_is_cwd_tmpfs()) {
		tst_brkm(TCONF, cleanup,
			 "Cannot do swapon on a file located on a tmpfs filesystem");
	}

	if (tst_is_cwd_nfs()) {
		tst_brkm(TCONF, cleanup,
			 "Cannot do swapon on a file located on a nfs filesystem");
	}

	if (!tst_cwd_has_free(65536)) {
		tst_brkm(TBROK, cleanup,
			 "Insufficient disk space to create swap file");
	}

	/*create file */
	if (system
	    ("dd if=/dev/zero of=swapfile01 bs=1024  count=65536 > tmpfile"
	     " 2>&1 ") != 0) {
		tst_brkm(TBROK, cleanup, "Failed to create file for swap");
	}

	/* make above file a swap file */
	if (system("mkswap swapfile01 > tmpfile 2>&1") != 0) {
		tst_brkm(TBROK, cleanup, "Failed to make swapfile");
	}

}

/*
 * cleanup() - Performs one time cleanup for this test at
 * completion or premature exit
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	tst_rmdir();

}