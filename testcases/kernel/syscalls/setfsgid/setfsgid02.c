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
 *	setfsgid02.c
 *
 * DESCRIPTION
 *	Testcase to check the basic functionality of setfsgid(2) system
 *	call failures.
 *
 * ALGORITHM
 *	Call setfsgid() and test the return value when invalid gid used.
 *
 * USAGE:  <for command-line>
 *  setfsgid01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <errno.h>
#ifdef __GLIBC__
#include <sys/fsuid.h>
#endif
#include "test.h"
#include "usctest.h"

char *TCID = "setfsgid02";
int TST_TOTAL = 1;

void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	gid_t gid;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* Check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		gid = 1;
		while (getgrgid(gid)) {
			gid++;
		}

		TEST(setfsgid(gid));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "call failed unexpectedly - errno %d",
				 TEST_ERRNO);
			continue;
		}

		if (!STD_FUNCTIONAL_TEST) {
			tst_resm(TPASS, "call succeeded");
			continue;
		}

		if (TEST_RETURN == gid) {
			tst_resm(TFAIL, "setfsgid() returned %ld, expected %d",
				 TEST_RETURN, gid);
		} else {
			tst_resm(TPASS, "setfsgid() returned expected value : "
				 "%ld", TEST_RETURN);
		}
	}
	cleanup();

	  return EXIT_SUCCESS;
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{

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