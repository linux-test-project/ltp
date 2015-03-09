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
 * Testcase to check the basic functionality of the getuid() system call.
 *
 * For functionality test the return value from getgid() is compared to passwd
 * entry.
 */

#include <pwd.h>
#include <errno.h>

#include "test.h"
#include "compat_16.h"

TCID_DEFINE(getuid03);
int TST_TOTAL = 1;

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	struct passwd *pwent;
	int lc;
	uid_t uid;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		TEST(GETUID(cleanup));

		if (TEST_RETURN == -1)
			tst_brkm(TBROK | TTERRNO, cleanup, "getuid failed");

		uid = getuid();
		pwent = getpwuid(uid);

		if (pwent == NULL)
			tst_resm(TFAIL | TERRNO, "getpwuid failed");

		UID16_CHECK(pwent->pw_uid, getuid, cleanup);

		if (pwent->pw_uid != TEST_RETURN)
			tst_resm(TFAIL, "getpwuid value, %d, "
				 "does not match getuid "
				 "value, %ld", pwent->pw_uid,
				 TEST_RETURN);
		else
			tst_resm(TPASS, "values from getuid "
				 "and getpwuid match");
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
