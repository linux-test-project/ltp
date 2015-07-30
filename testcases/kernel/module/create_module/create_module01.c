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
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
/**********************************************************
 *
 *    TEST IDENTIFIER   : create_module01
 *
 *    EXECUTED BY       : root / superuser
 *
 *    TEST TITLE        : Basic tests for create_module(2)
 *
 *    TEST CASE TOTAL   : 1
 *
 *    AUTHOR            : Madhu T L <madhu.tarikere@wipro.com>
 *
 *    SIGNALS
 *	Uses SIGUSR1 to pause before test if option set.
 *	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	This is a Phase I test for the create_module(2) system call.
 *	It is intended to provide a limited exposure of the system call.
 *
 *	Setup:
 *	  Setup signal handling.
 *	  Test caller is superuser
 *	  Pause for SIGUSR1 if option specified.
 *	  Initialize modname for each child process
 *
 *	Test:
 *	 Loop if the proper options are given.
 *	  Execute system call
 *	  Check return code, if system call failed (return=-1)
 *		Issue FAIL message with errno.
 *	  Otherwise, Issue PASS message and delete the module entry.
 *
 *	Cleanup:
 *	  Call delete_module system call to remove module entry if exists.
 *	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  create_module01 [-c n] [-e] [-f] [-h] [-i n] [-I x] [-p] [-P x] [-t]
 *		where,  -c n : Run n copies concurrently.
 *			-e   : Turn on errno logging.
 *			-f   : Turn off functional testing
 *			-h   : Show help screen
 *			-i n : Execute test n times.
 *			-I x : Execute test for x seconds.
 *			-p   : Pause for SIGUSR1 before starting
 *			-P x : Pause for x seconds between iterations.
 *			-t   : Turn on syscall timing.
 *
 ****************************************************************/
#include <errno.h>
#include <asm/atomic.h>
#include <linux/module.h>
#include "test.h"

#define MODSIZE 10000		/* Arbitrarily selected MODSIZE */
#define BASEMODNAME "dummy"

static void setup(void);
static void cleanup(void);

char *TCID = "create_module01";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
static char modname[20];	/* Name of the module */

int main(int argc, char **argv)
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;

		/* Test the system call */
		TEST(create_module(modname, MODSIZE));

		/* check return code */
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "create_module() failed errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			tst_resm(TPASS, "create_module() returned 0x%x",
				 TEST_RETURN);
			if (delete_module(modname) != 0) {
				tst_brkm(TBROK, NULL, "Failed to delete"
					 "loadable module entry for %s",
					 modname);
			}
		}
	}

	/* perform global cleanup and exit */
	cleanup();

}

/* setup() - performs all ONE TIME setup for this test */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root();

	if (tst_kvercmp(2, 5, 48) >= 0)
		tst_brkm(TCONF, NULL, "This test will not work on "
			 "kernels after 2.5.48");

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -c option.
	 */
	TEST_PAUSE;

	/* Initialize unique module name for each child process */
	if (sprintf(modname, "%s_%d", BASEMODNAME, getpid()) == -1) {
		tst_brkm(TBROK, NULL, "Failed to initialize module name");
	}
}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 *	completion or premature exit
 */
void cleanup(void)
{
	/*
	 * If module entry is not removed (possible if create_module()
	 * succeeds and signal is caught before execution of delete_module())
	 * attempt to remove it here
	 */
	if (delete_module(modname) == -1) {
		/* With errno, check module exists, if so send msg */
		if (errno != ENOENT) {
			tst_resm(TWARN, "Failed to delete loadable module"
				 "entry for %s errno returned %d", modname,
				 errno);
		}
	}

}
