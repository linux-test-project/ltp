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
 * Testcase to check the basic functionality of getegid().
 *
 * For functionality test the return value from getegid() is compared to passwd
 * entry.
 */

#include <pwd.h>
#include <errno.h>

#include "test.h"
#include "compat_16.h"

static void cleanup(void);
static void setup(void);

TCID_DEFINE(getegid02);
int TST_TOTAL = 1;

int main(int ac, char **av)
{
	int lc;
	uid_t euid;
	struct passwd *pwent;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		TEST(GETEGID(cleanup));

		if (TEST_RETURN < 0) {
			tst_brkm(TBROK, cleanup, "This should never happen");
		}

		euid = geteuid();
		pwent = getpwuid(euid);

		if (pwent == NULL)
			tst_brkm(TBROK, cleanup, "geteuid() returned "
				 "unexpected value %d", euid);

		GID16_CHECK(pwent->pw_gid, getegid, cleanup);

		if (pwent->pw_gid != TEST_RETURN) {
			tst_resm(TFAIL, "getegid() return value"
				 " %ld unexpected - expected %d",
				 TEST_RETURN, pwent->pw_gid);
		} else {
			tst_resm(TPASS,
				 "effective group id %ld "
				 "is correct", TEST_RETURN);
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
