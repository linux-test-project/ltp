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
 * Test Name: getresuid01
 *
 * Test Description:
 *  Verify that getresuid() will be successful to get the real, effective
 *  and saved user id of the calling process.
 *
 * Expected Result:
 *  getresuid() should return with 0 value and the real/effective/saved
 *  user ids should be equal to that of calling process.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *   	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *   	Verify the Functionality of system call
 *      if successful,
 *      	Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *
 * Usage:  <for command-line>
 *  getresuid01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *  None.
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#include "test.h"
#include "compat_16.h"

char *TCID = "getresuid01";
int TST_TOTAL = 1;
UID_T pr_uid, pe_uid, ps_uid;	/* calling process real/effective/saved uid */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;
	UID_T real_uid,		/* real/eff./saved user id from getresuid() */
	 eff_uid, sav_uid;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Call getresuid() to get the real/effective/saved
		 * user id's of the calling process.
		 */
		TEST(GETRESUID(cleanup, &real_uid, &eff_uid, &sav_uid));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "getresuid() Failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			continue;
		}

		if ((real_uid != pr_uid) || (eff_uid != pe_uid) ||
		    (sav_uid != ps_uid)) {
			tst_resm(TFAIL, "real:%d, effective:%d, "
				 "saved-user:%d ids differ",
				 real_uid, eff_uid, sav_uid);
		} else {
			tst_resm(TPASS, "Functionality of getresuid() "
				 "successful");
		}
	}

	cleanup();
	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 *	     Get the real/effective/saved user id of the calling process.
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Real user-id of the calling process */
	pr_uid = getuid();

	/* Effective user-id of the calling process */
	pe_uid = geteuid();

	/* Saved user-id of the calling process */
	ps_uid = geteuid();

}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void cleanup(void)
{

}
