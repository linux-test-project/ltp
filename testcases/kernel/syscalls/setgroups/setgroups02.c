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
 * Test Name: setgroups02
 *
 * Test Description:
 *  Verify that, only root process can invoke setgroups() system call to
 *  set the supplementary group IDs of the process.
 *
 * Expected Result:
 *  The call succeeds in setting all the supplementary group IDs of the
 *  calling process. The new group should be set in the process  supplemental
 *  group list.
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
 *  setgroups02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>

#include "test.h"
#include "usctest.h"

#include "compat_16.h"

#define TESTUSER	"nobody"

TCID_DEFINE(setgroups02);	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test conditions */
extern int Tst_count;		/* Test Case counter for tst_* routines */
GID_T groups_list[NGROUPS];	/* Array to hold gids for getgroups() */

struct passwd *user_info;	/* struct. to hold test user info */
void setup();			/* setup function for the test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc, i;		/* loop counters */
	char *msg;		/* message returned from parse_opts */
	int gidsetsize = 1;	/* only one GID, the GID of TESTUSER */
	int PASS_FLAG = 0;	/* used for checking group array */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* Perform global setup for test */
	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * Call setgroups() to set supplimentary group IDs of
		 * the calling super-user process to gid of TESTUSER.
		 */
		TEST(SETGROUPS(gidsetsize, groups_list));

		/* check return code of setgroups(2) */
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "setgroups(%d, groups_list) Failed, "
				 "errno=%d : %s", gidsetsize, TEST_ERRNO,
				 strerror(TEST_ERRNO));
			continue;
		}

		/*
		 * Perform functional verification if test
		 * executed without (-f) option.
		 */
		if (STD_FUNCTIONAL_TEST) {
			/*
			 * Call getgroups(2) to verify that
			 * setgroups(2) successfully set the
			 * supp. gids of TESTUSER.
			 */
			groups_list[0] = '\0';
			if (GETGROUPS(gidsetsize, groups_list) < 0) {
				tst_brkm(TFAIL, cleanup, "getgroups() Fails, "
					 "error=%d", errno);
			}
			for (i = 0; i < NGROUPS; i++) {
				if (groups_list[i] == user_info->pw_gid) {
					tst_resm(TPASS,
						 "Functionality of setgroups"
						 "(%d, groups_list) successful",
						 gidsetsize);
					PASS_FLAG = 1;
				}
			}
			if (PASS_FLAG == 0) {
				tst_resm(TFAIL, "Supplimentary gid %d not set "
					 "for the process", user_info->pw_gid);
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}
	}			/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test.
 *
 *  Make sure the test process uid is root.
 *  Get the supplimentrary group id of test user from /etc/passwd file.
 */
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Make sure the calling process is super-user only */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Must be ROOT to run this test.");
	}

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Get the group id info. of TESTUSER from /etc/passwd */
	if ((user_info = getpwnam(TESTUSER)) == NULL) {
		tst_brkm(TFAIL, cleanup, "getpwnam(2) of %s Failed", TESTUSER);
	}

	if (!GID_SIZE_CHECK(user_info->pw_gid)) {
		tst_brkm(TBROK,
			 cleanup,
			 "gid returned from getpwnam is too large for testing setgroups16");
	}

	groups_list[0] = user_info->pw_gid;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}
