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
 * NAME
 *	getgid03.c
 *
 * DESCRIPTION
 *	Testcase to check the basic functionality of getgid().
 *
 * ALGORITHM
 *	call setup
 *	loop if that option was specified
 *	Execute getgid() call using TEST macro
 *	if not expected value
 *	   break remaining tests and cleanup
 *	if STD_FUNCTIONAL_TEST
 *	   Execute getuid() call
 *	   Execute getpwduid() call
 *	   if the passwd entry is NULL
 *	      break the remaining tests and cleanup
 *	   else if pwent->pw_gid != TEST_RETURN
 *	      print failure message
 *	   else
 *	      print pass message
 *	else
 *	   print pass message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  getgid03 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	none
 */

#include <pwd.h>
#include <errno.h>

#include "test.h"
#include "usctest.h"

#include "compat_16.h"

void cleanup(void);
void setup(void);

TCID_DEFINE(getgid03);
int TST_TOTAL = 1;

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int uid;
	struct passwd *pwent;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	 }

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(GETGID());

		if (TEST_RETURN < 0) {
			tst_brkm(TBROK, cleanup, "This should never happen");
		}

		if (STD_FUNCTIONAL_TEST) {
			uid = getuid();
			pwent = getpwuid(uid);

			if (pwent == NULL) {
				tst_brkm(TBROK, cleanup, "getuid() returned "
					 "unexpected value %d", uid);
			} else if (!GID_SIZE_CHECK(pwent->pw_gid)) {
				tst_brkm(TBROK,
					 cleanup,
					 "gid for uid %d is too large for testing getgid16",
					 uid);
			} else {
				if (pwent->pw_gid != TEST_RETURN) {
					tst_resm(TFAIL, "getgid() return value "
						 "%ld unexpected - expected %d",
						 TEST_RETURN, pwent->pw_gid);
				} else {
					tst_resm(TPASS, "group id %ld returned "
						 "correctly", TEST_RETURN);
				}
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}
	}
	cleanup();

	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup()
{
	/*
	 * print timing status if that option was specified.
	 * print errno log if that option was specified
	 */
	TEST_CLEANUP;

}