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
 * 	setgid02.c
 *
 * DESCRIPTION
 * 	Testcase to ensure that the setgid() system call sets errno to EPERM
 *
 * ALGORITHM
 *	Call setgid() to set the gid to that of root. Run this test as
 *	ltpuser1, and expect to get EPERM
 *
 * USAGE:  <for command-line>
 *  setgid02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 * 	Must be run as a nonroot user
 */
#include <pwd.h>
#include <errno.h>
#include <test.h>
#include <usctest.h>

TCID_DEFINE(setgid02);
int TST_TOTAL = 1;
extern int Tst_count;

char root[] = "root";
char nobody_uid[] = "nobody";
char nobody_gid[] = "nobody";
struct passwd *ltpuser;

static void setup(void);
static void cleanup(void);

#include "compat_16.h"

int exp_enos[] = { EPERM, 0 };

int main(int ac, char **av)
{
	struct passwd *getpwnam(), *rootpwent;

	int lc;			/* loop counter */
	char *msg;		/* message returned by parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	setup();

	TEST_EXP_ENOS(exp_enos);

	/* check looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		if ((rootpwent = getpwnam(root)) == NULL) {
			tst_brkm(TBROK, cleanup, "getpwnam failed for user id "
				 "%s", root);
		}

		if (!GID_SIZE_CHECK(rootpwent->pw_gid)) {
			tst_brkm(TBROK,
				 cleanup,
				 "gid for `%s' is too large for testing setgid16",
				 root);
		}

		TEST(SETGID(rootpwent->pw_gid));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "call succeeded unexpectedly");
			continue;
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		if (TEST_ERRNO != EPERM) {
			tst_resm(TFAIL, "setgid set invalid errno, expected: "
				 "EPERM, got: %d\n", TEST_ERRNO);
		} else {
			tst_resm(TPASS, "setgid returned EPERM");
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
/* Switch to nobody user for correct error code collection */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}
	ltpuser = getpwnam(nobody_uid);

	if (!GID_SIZE_CHECK(ltpuser->pw_gid)) {
		tst_brkm(TBROK,
			 cleanup,
			 "gid for `%s' is too large for testing setgid16",
			 nobody_gid);
	}

	if (setgid(ltpuser->pw_gid) == -1) {
		tst_resm(TINFO, "setgid failed to "
			 "to set the effective gid to %d", ltpuser->pw_gid);
		perror("setgid");
	}

	if (setuid(ltpuser->pw_uid) == -1) {
		tst_resm(TINFO, "setuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
		perror("setuid");
	}

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
