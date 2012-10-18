/*
 *
 *   Copyright (C) International Business Machines  Corp., 2001
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
 * NAME
 *	setfsuid03.c
 *
 * DESCRIPTION
 *	Testcase to test the basic functionality of the setfsuid(2) system
 *	call when called by a user other than root.
 *
 * ALGORITHM
 *	Call setfsuid(2) and test the uid returned by setfsuid(2).
 *	If the action are allowed,
 *	then the testcase fails.
 *
 * USAGE:  <for command-line>
 *  setfsuid01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *      04/2003 Adapted by Dustin Kirkland (k1rkland@us.ibm.com)
 *
 * RESTRICTIONS
 *	None
 */
#include <stdio.h>
#include <unistd.h>
#ifdef __GLIBC__
#include <sys/fsuid.h>
#endif
#include <sys/types.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"
#include <pwd.h>

void setup(void);
void cleanup(void);

char *TCID = "setfsuid03";
int TST_TOTAL = 1;
char nobody_uid[] = "nobody";
struct passwd *ltpuser;

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	uid_t uid;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	uid = geteuid();
	uid = 1;
	while (!getpwuid(uid)) {
		uid++;
	}

	/* Check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(setfsuid(uid));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "call failed unexpectedly - errno %d",
				 TEST_ERRNO);
			continue;
		}

		if (!STD_FUNCTIONAL_TEST) {
			tst_resm(TPASS, "call succeeded");
			continue;
		}

		if (TEST_RETURN == uid) {
			tst_resm(TFAIL,
				 "setfsuid() returned %ld, expected anything but %d",
				 TEST_RETURN, uid);
		} else {
			tst_resm(TPASS, "setfsuid() returned expected value : "
				 "%ld", TEST_RETURN);
		}
	}
	cleanup();
	tst_exit();
	tst_exit();

 }

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* Switch to nobody user for correct error code collection */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Test must be run as root");
	}
	ltpuser = getpwnam(nobody_uid);
	if (setuid(ltpuser->pw_uid) == -1) {
		tst_resm(TINFO, "setuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
		perror("setuid");
	}

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;
}
