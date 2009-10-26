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
#include <test.h>
#include <usctest.h>
#include "compat_16.h"

TCID_DEFINE(getuid02);
int TST_TOTAL = 1;
extern int Tst_count;

void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned by parse_opts */

	struct passwd *getpwuid(), *pwent;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* check looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(GETEUID());

		if (TEST_RETURN < 0) {
			tst_brkm(TBROK, cleanup, "This should never happen");
		}

		if (STD_FUNCTIONAL_TEST) {

			pwent = getpwuid(TEST_RETURN);
			if (pwent == NULL) {
				tst_resm(TFAIL, "geteuid() returned unexpected "
					 "value %ld", TEST_RETURN);
			} else if (!UID_SIZE_CHECK(pwent->pw_uid)) {
				tst_brkm(TBROK,
					 cleanup,
					 "uid(%ld) is too large for testing geteuid16",
					 TEST_RETURN);
			} else {
				if (pwent->pw_uid != TEST_RETURN) {
					tst_resm(TFAIL, "getpwuid() value, %d, "
						 "does not match geteuid() "
						 "value, %ld", pwent->pw_uid,
						 TEST_RETURN);
				} else {
					tst_resm(TPASS, "values from geteuid()"
						 " and getpwuid() match");
				}
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}
	}
	cleanup();

	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
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

	/* exit with return code appropriate for results */
	tst_exit();
}
