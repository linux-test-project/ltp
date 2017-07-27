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
 *    TEST IDENTIFIER	: sysfs01
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for sysfs(2)
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 *    SIGNALS
 *	Uses SIGUSR1 to pause before test if option set.
 *	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *    This is a Phase I test for the sysfs(2) system call.
 *    This test is carried out for option 1 for sysfs(2).
 *    It is intended to provide a limited exposure of the system call.
 *
 *
 *	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *	 Loop if the proper options are given.
 *	  Execute system call
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	  Otherwise, Issue a PASS message.
 *
 *	Cleanup:
 *	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * sysfs01 [-c n]  [-e] [-i n] [-I x] [-p x] [-t] [-h] [-f] [-p]
 *  where:
 *	-c n : run n copies simultaneously.
 *	-e   : Turn on errno logging.
 *	-i n : Execute test n times.
 *	-I x : Execute test for x seconds.
 *	-p   : Pause for SIGUSR1 before starting
 *	-P x : Pause for x seconds between iterations.
 *	-t   : Turn on syscall timing.
 *
 *RESTRICTIONS:
 *There is no glibc or libc support
 *Kernel should be compiled with proc filesystem support
 ******************************************************************************/

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "test.h"
#include "lapi/syscalls.h"

static void setup();
static void cleanup();

char *TCID = "sysfs01";
int TST_TOTAL = 1;

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/* option 1, buf holds fs name */
		TEST(ltp_syscall(__NR_sysfs, 1, "proc"));

		/* check return code */
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "sysfs(2) Failed for "
				 "option 1 and set errno to %d", TEST_ERRNO);
		} else {
			tst_resm(TPASS, "sysfs(2) Passed for " "option 1");
		}
	}			/*End of TEST_LOOPING */

	/*Clean up and exit */
	cleanup();
	tst_exit();

}

/* setup() - performs all ONE TIME setup for this test */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

}

/*
 * cleanup() - Performs one time cleanup for this test at
 * completion or premature exit
 */

void cleanup(void)
{

}
