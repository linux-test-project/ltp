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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * Test Name: getresuid03
 *
 * Test Description:
 *  Verify that getresuid() will be successful to get the real, effective
 *  and saved user ids after calling process invokes setresuid() to change
 *  the effective uid to that of specified user.
 * $
 * Expected Result:
 *  getresuid() should return with 0 value and the effective user id
 *  should match the euid of specified user, real/saved user ids should
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
 *  getresuid03 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>

#include "test.h"
#include "usctest.h"

extern int getresuid(uid_t *, uid_t *, uid_t *);
extern int setresuid(uid_t, uid_t, uid_t);

char *TCID = "getresuid03";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
uid_t pr_uid, pe_uid, ps_uid;	/* calling process real/effective/saved uid */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	uid_t real_uid,		/* real/eff./saved user id from getresuid() */
	 eff_uid, sav_uid;

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/*
		 * Call getresuid() to get the real/effective/saved
		 * user id's of the calling process after
		 * setreuid() in setup.
		 */
		TEST(getresuid(&real_uid, &eff_uid, &sav_uid));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "getresuid() Failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			continue;
		}
		/*
		 * Perform functional verification if test
		 * executed without (-f) option.
		 */
		if (STD_FUNCTIONAL_TEST) {
			/*
			 * Verify the real/effective/saved uid
			 * values returned by getresuid with the
			 * expected values.
			 */
			if ((real_uid != pr_uid) || (eff_uid != pe_uid) ||
			    (sav_uid != ps_uid)) {
				tst_resm(TFAIL, "real:%d, effective:%d, "
					 "saved-user:%d ids differ",
					 real_uid, eff_uid, sav_uid);
			} else {
				tst_resm(TPASS, "Functionality of getresuid() "
					 "successful");
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}
	}

	cleanup();

	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 *	     Make sure test process uid is root.
 *	     Get the real/effective/saved user id of the calling process.
 *	     Get the user info. of test user "ltpuser1" from /etc/passwd file.
 *	     Set the eff. user id of test process to that of "ltpuser1" user.
 */
void setup()
{
	struct passwd *user_id;	/* passwd struct for test user */

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Check that the test process id is super/root  */
	if (geteuid() != 0) {
		tst_brkm(TBROK, cleanup, "Must be super/root for this test!");
	}

	/* Real user-id of the calling process */
	pr_uid = getuid();

	/* Saved user-id of the calling process. */
	ps_uid = geteuid();

	/* Get effective uid of "ltpuser1" user from passwd file */
	if ((user_id = getpwnam("nobody")) == NULL) {
		tst_brkm(TBROK, cleanup,
			 "getpwnam(nobody) Failed, errno=%d", errno);
	}

	/* Effective user-id of the test-user "ltpuser1" */
	pe_uid = user_id->pw_uid;

	/*
	 * Set the effective user-id of the process to that of
	 * test user "ltpuser1".
	 * The real/saved  user id remains same as of caller.
	 */
	if (setresuid(-1, pe_uid, -1) < 0) {
		tst_brkm(TBROK, cleanup,
			 "setresuid(-1, %d, -1) Fails, errno:%d : %s",
			 ps_uid, errno, strerror(errno));
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Reset the effective/saved uid of the calling process */
	if (setreuid(-1, pr_uid) < 0) {
		tst_brkm(TBROK, NULL, "resetting process effective uid failed");
	}

}
