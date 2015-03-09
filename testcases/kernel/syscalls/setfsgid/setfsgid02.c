/*
 * Copyright (C) International Business Machines  Corp., 2001
 * Ported by Wayne Boyer
 * Adapted by Dustin Kirkland (k1rkland@us.ibm.com)
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
 *     Testcase to check the basic functionality of setfsgid(2) system
 *     call failures.
 */

#include <stdio.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <errno.h>

#include "test.h"
#include "compat_16.h"

TCID_DEFINE(setfsgid02);
int TST_TOTAL = 1;

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc;

	gid_t gid;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		gid = 1;
		while (getgrgid(gid))
			gid++;

		GID16_CHECK(gid, setfsgid, cleanup);

		TEST(SETFSGID(cleanup, gid));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO,
				"setfsgid() failed unexpectedly");
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
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
}
