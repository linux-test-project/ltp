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
 * 	getuid02.c
 *
 * DESCRIPTION
 *	Testcase to check the basic functionality of the geteuid() system call.
 *
 * USAGE:  <for command-line>
 *  getuid02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
 * 	None
 */

#include <pwd.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"
#include "compat_16.h"

TCID_DEFINE(getuid02);
int TST_TOTAL = 1;

void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	struct passwd *pwent;
	int lc;			/* loop counter */
	char *msg;		/* message returned by parse_opts */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		TEST(GETEUID());

		if (TEST_RETURN == -1)
			tst_brkm(TBROK|TTERRNO, cleanup, "geteuid* failed");

		if (STD_FUNCTIONAL_TEST) {

			pwent = getpwuid(TEST_RETURN);
			if (pwent == NULL)
				tst_resm(TFAIL|TERRNO, "getpwuid failed");
			else if (!UID_SIZE_CHECK(pwent->pw_uid))
				tst_brkm(TBROK, cleanup,
				    "uid = %ld is too large for testing "
				    "via geteuid16", TEST_RETURN);
			else {
				if (pwent->pw_uid != TEST_RETURN)
					tst_resm(TFAIL, "getpwuid value, %d, "
						 "does not match geteuid "
						 "value, %ld", pwent->pw_uid,
						 TEST_RETURN);
				else
					tst_resm(TPASS, "values from geteuid "
						 "and getpwuid match");
			}
		} else
			tst_resm(TPASS, "call succeeded");
	}
	cleanup();
	tst_exit();

}

void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

void cleanup()
{
	TEST_CLEANUP;
}