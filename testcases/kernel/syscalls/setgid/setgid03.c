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
 * ALGORITHM
 *	As root sets the current group id to ltpuser1, verify the results
 */
#include <pwd.h>
#include <errno.h>

#include "test.h"
#include <compat_16.h>

TCID_DEFINE(setgid03);
int TST_TOTAL = 1;

static char ltpuser1[] = "nobody";
static char root[] = "root";
static struct passwd *ltpuser1pwent, *rootpwent;
static gid_t mygid;

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		TEST(SETGID(cleanup, ltpuser1pwent->pw_gid));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "call failed unexpectedly");
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
	tst_exit();
}

static void setup(void)
{
	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

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

	GID16_CHECK(ltpuser1pwent->pw_gid, setgid, cleanup);
}

static void cleanup(void)
{
}
