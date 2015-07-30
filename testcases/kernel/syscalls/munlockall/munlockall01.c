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
/**************************************************************************
 *
 *    TEST IDENTIFIER	: munlockall01
 *
 *    EXECUTED BY	: root / superuser
 *
 *    TEST TITLE	: Basic test for munlockall(2)
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: sowmya adiga<sowmya.adiga@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	This is a phase I test for the munlockall(2) system call.
 *	It is intended to provide a limited exposure of the system call.
 *
 * 	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *        Execute system call
 *	  Check return code, if system call failed (return=-1)
 *	  Log the errno and Issue a FAIL message.
 *	  Otherwise, Issue a PASS message.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  munlockall01 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
 *		where,		-c n : Run n copies concurrently
 *	               		-e   : Turn on errno logging.
 *				-h   : Show this help screen
 *				-i n : Execute test n times.
 *				-I x : Execute test for x seconds.
 *				-p   : Pause for SIGUSR1 before starting
 *                      	-P x : Pause for x seconds between iterations.
 *                       	 t   : Turn on syscall timing.
 *
 * RESTRICTIONS
 * Must be root/superuser to run it.
 *****************************************************************************/
#include <errno.h>
#include <sys/mman.h>
#include "test.h"

void setup();
void cleanup();

char *TCID = "munlockall01";
int TST_TOTAL = 1;

#if !defined(UCLINUX)

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	/* check looping state */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		TEST(munlockall());

		/* check return code */
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO, "munlockall() Failed with"
				 " return=%ld", TEST_RETURN);
		} else {
			tst_resm(TPASS, "munlockall() passed with"
				 " return=%ld ", TEST_RETURN);

		}
	}

	/* cleanup and exit */
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

/* setup() - performs all ONE TIME setup for this test. */
void setup(void)
{
	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup(void)
{
}
