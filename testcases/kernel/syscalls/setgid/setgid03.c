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
 * 	setgid03.c
 *
 * CALLS
 * 	setgid(1) getgid(2)
 *
 * ALGORITHM
 * 	As root sets the current group id to ltpuser1, verify the results
 *
 * USAGE:  <for command-line>
 *  setgid03 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
 * 	Test must be run as root.
 */
#include <pwd.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"

TCID_DEFINE(setgid03);
int TST_TOTAL = 1;
extern int Tst_count;

char ltpuser1[] = "nobody";
char root[] = "root";
struct passwd *getpwnam(), *ltpuser1pwent, *rootpwent;
int mygid;

static void setup(void);
static void cleanup(void);

#include "compat_16.h"

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	setup();

	/* Check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(setgid(ltpuser1pwent->pw_gid));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "call failed unexpectedly");
			continue;
		}

		if (!STD_FUNCTIONAL_TEST) {
			tst_resm(TPASS, "call succeeded");
			continue;
		}

		if (getgid() != ltpuser1pwent->pw_gid) {
			tst_resm(TFAIL, "setgid failed to set gid to "
				 "ltpuser1's gid");
		} else {
			tst_resm(TPASS, "functionality of getgid() is correct");
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
	/* test must be run as root */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	if ((rootpwent = getpwnam(root)) == NULL) {
		tst_brkm(TBROK, cleanup, "getpwnam failed for "
			 "user id %s", root);
	}

	mygid = getgid();

	if (mygid != rootpwent->pw_gid) {
		tst_brkm(TBROK, cleanup, "real group id is not root");
	}

	if ((ltpuser1pwent = getpwnam(ltpuser1)) == NULL) {
		tst_brkm(TBROK, cleanup, "getpwnam failed for user "
			 "id %s", ltpuser1);
	}

	if (!(GID_SIZE_CHECK(rootpwent->pw_gid))) {
		tst_brkm(TBROK,
			 cleanup,
			 "gid for `%s' is too large for testing setgid16",
			 root);
	}

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
