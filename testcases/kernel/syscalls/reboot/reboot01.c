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
 *    TEST IDENTIFIER	: reboot01
 *
 *    EXECUTED BY	: root / superuser
 *
 *    TEST TITLE	: Basic test for reboot(2)
 *
 *    TEST CASE TOTAL	: 2
 *
 *    AUTHOR		: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *    This is a Phase I test for the reboot(2) system call.
 *    It is intended to provide a limited exposure of the system call.
 *   $
 *
 * 	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *	  setting the flag value for two tests.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	 for two test cases for two flag values
 *	  Execute system call
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	  Otherwise, Issue a PASS message.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * reboot01 [-c n] [-e] [-i n] [-I x] [-p x] [-t] [-h] [-f] [-p]
 * where:
 * 	-c n : run the test for n number of times.
 *	-e   : Turn on errno logging.
 *	-i n : Execute test n times.
 *	-I x : Execute test for x seconds.
 *	-p   : Pause for SIGUSR1 before starting
 *	-P x : Pause for x seconds between iterations.
 *	-t   : Turn on syscall timing.
 *
 *RESTRICTIONS:
 *for lib4 and lib5 reboot(2) system call is implemented as
 *int reboot(int magic, int magic2, int flag, void *arg); This test case
 *is written for int reboot(int flag); which is implemented under glibc
 *Therefore this testcase may not work under libc4 and libc5 libraries
 *****************************************************************************/

#include <unistd.h>
#include <sys/reboot.h>
#include "test.h"
#include <errno.h>
#include <linux/reboot.h>

static void setup();
static void cleanup();

char *TCID = "reboot01";
int TST_TOTAL = 2;

static int flag[2] = { LINUX_REBOOT_CMD_CAD_ON, LINUX_REBOOT_CMD_CAD_OFF };

static const char *option_message[] = { "LINUX_REBOOT_CMD_CAD_ON",
	"LINUX_REBOOT_CMD_CAD_OFF"
};

int main(int ac, char **av)
{

	int lc, i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			TEST(reboot(flag[i]));
			/* check return code */
			if (TEST_RETURN == -1) {
				tst_resm(TFAIL, "reboot(2) Failed for "
					 "option %s", option_message[i]);
			} else {
				tst_resm(TPASS, "reboot(2) Passed for "
					 "option %s", option_message[i]);
			}
		}		/*End of TEST CASE LOOPING */
	}			/*End for TEST_LOOPING */

	/*Clean up and exit */
	cleanup();

	tst_exit();
}

/* setup() - performs all ONE TIME setup for this test */
void setup(void)
{
	tst_require_root();

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
