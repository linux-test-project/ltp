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
 * Test Name: getresgid03
 *
 * Test Description:
 *  Verify that getresgid() will be successful to get the real, effective
 *  and saved user ids after calling process invokes setresgid() to change
 *  the effective gid to that of specified user.
 *
 * Expected Result:
 *  getresgid() should return with 0 value and the effective user id
 *  should match the egid of specified user, real/saved user ids should
 *  remain unchanged.
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
 *	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *	Verify the Functionality of system call
 *      if successful,
 *		Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *
 * Usage:  <for command-line>
 *  getresgid03 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
 *  This test should be run by 'super-user' (root) only.
 *
 */
#define _GNU_SOURCE 1

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>

#include "test.h"

/*
 * Don't forget to remove USE_LEGACY_COMPAT_16_H from Makefile after
 * rewriting all tests to the new API.
 */
#include "compat_16.h"

char *TCID = "getresgid03";
int TST_TOTAL = 1;
GID_T pr_gid, pe_gid, ps_gid;	/* calling process real/effective/saved gid */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;
	GID_T real_gid,		/* real/eff./saved user id from getresgid() */
	 eff_gid, sav_gid;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Call getresgid() to get the real/effective/saved
		 * user id's of the calling process after
		 * setregid() in setup.
		 */
		TEST(GETRESGID(cleanup, &real_gid, &eff_gid, &sav_gid));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "getresgid() Failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			continue;
		}
		/*
		 * Verify the real/effective/saved gid
		 * values returned by getresgid with the
		 * expected values.
		 */
		if ((real_gid != pr_gid) || (eff_gid != pe_gid) ||
		    (sav_gid != ps_gid)) {
			tst_resm(TFAIL, "real:%d, effective:%d, "
				 "saved-user:%d ids differ",
				 real_gid, eff_gid, sav_gid);
		} else {
			tst_resm(TPASS, "Functionality of getresgid() "
				 "successful");
		}
	}

	cleanup();
	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 *	     Make sure test process gid is root.
 *	     Get the real/effective/saved user id of the calling process.
 *	     Get the user info. of test user "ltpuser1" from /etc/passwd file.
 *	     Set the eff. user id of test process to that of "ltpuser1" user.
 */
void setup(void)
{
	struct passwd *user_id;	/* passwd struct for test user */

	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Real user-id of the calling process */
	pr_gid = getgid();

	/* Saved user-id of the calling process. */
	ps_gid = getegid();

	/* Get effective gid of "ltpuser1" user from passwd file */
	if ((user_id = getpwnam("nobody")) == NULL) {
		tst_brkm(TBROK, cleanup,
			 "getpwnam(nobody) Failed, errno=%d", errno);
	}

	/* Effective user-id of the test-user "ltpuser1" */
	pe_gid = user_id->pw_gid;

	/*
	 * Set the effective user-id of the process to that of
	 * test user "ltpuser1".
	 * The real/saved  user id remains same as of caller.
	 */
	if (setresgid(-1, pe_gid, -1) < 0) {
		tst_brkm(TBROK, cleanup,
			 "setresgid(-1, %d, -1) Fails, errno:%d : %s",
			 ps_gid, errno, strerror(errno));
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void cleanup(void)
{

	/* Reset the effective/saved gid of the calling process */
	if (setregid(-1, pr_gid) < 0) {
		tst_brkm(TBROK, NULL, "resetting process effective gid failed");
	}

}
