/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  Ported by Wayne Boyer
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Testcase to ensure that the setgid() system call sets errno to EPERM
 *
 * ALGORITHM
 *	Call setgid() to set the gid to that of root. Run this test as
 *	ltpuser1, and expect to get EPERM
 */
#include <pwd.h>
#include <errno.h>

#include "test.h"
#include "compat_16.h"

TCID_DEFINE(setgid02);
int TST_TOTAL = 1;

static char root[] = "root";
static char nobody_uid[] = "nobody";
static char nobody_gid[] = "nobody";
static struct passwd *ltpuser;

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	struct passwd *getpwnam(), *rootpwent;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		if ((rootpwent = getpwnam(root)) == NULL) {
			tst_brkm(TBROK, cleanup, "getpwnam failed for user id "
				 "%s", root);
		}

		GID16_CHECK(rootpwent->pw_gid, setgid, cleanup);

		TEST(SETGID(cleanup, rootpwent->pw_gid));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "call succeeded unexpectedly");
			continue;
		}

		if (TEST_ERRNO != EPERM) {
			tst_resm(TFAIL, "setgid set invalid errno, expected: "
				 "EPERM, got: %d\n", TEST_ERRNO);
		} else {
			tst_resm(TPASS, "setgid returned EPERM");
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_require_root();

	/* Switch to nobody user for correct error code collection */
	ltpuser = getpwnam(nobody_uid);
	if (ltpuser == NULL)
		tst_brkm(TBROK, cleanup, "getpwnam failed for user id %s",
			nobody_uid);

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

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
}
