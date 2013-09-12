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

#include <pwd.h>
#include <errno.h>

#include "test.h"
#include "usctest.h"
#include "compat_16.h"

TCID_DEFINE(geteuid02);
int TST_TOTAL = 1;

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	struct passwd *pwent;
	int lc;
	char *msg;
	uid_t uid;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		TEST(GETEUID(cleanup));

		if (TEST_RETURN == -1)
			tst_brkm(TBROK | TTERRNO, cleanup, "geteuid* failed");

		if (STD_FUNCTIONAL_TEST) {
			uid = geteuid();
			pwent = getpwuid(uid);

			if (pwent == NULL)
				tst_resm(TFAIL | TERRNO, "getpwuid failed");

			UID16_CHECK(pwent->pw_uid, geteuid, cleanup);
			if (pwent->pw_uid != TEST_RETURN)
				tst_resm(TFAIL, "getpwuid value, %d, "
					 "does not match geteuid "
					 "value, %ld", pwent->pw_uid,
					 TEST_RETURN);
			else
				tst_resm(TPASS, "values from geteuid "
					 "and getpwuid match");
		} else
			tst_resm(TPASS, "call succeeded");
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
	TEST_CLEANUP;
}
