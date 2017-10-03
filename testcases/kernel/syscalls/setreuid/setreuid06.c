/*
 * Copyright (c) International Business Machines  Corp., 2001
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
 *
 * Ported by John George
 */

/*
 * Test that EPERM is set when setreuid is given an invalid user id.
 */

#include <sys/wait.h>
#include <limits.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "test.h"
#include "safe_macros.h"
#include "compat_16.h"

#define INVAL_USER		 (USHRT_MAX-2)

TCID_DEFINE(setreuid06);
int TST_TOTAL = 1;

static struct passwd *ltpuser;

static void setup(void);
static void cleanup(void);

int main(int argc, char **argv)
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		TEST(SETREUID(cleanup, -1, INVAL_USER));
		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "%s did not fail as expected", TCID);
		} else if (TEST_ERRNO == EPERM) {
			tst_resm(TPASS, "setreuid set errno to EPERM as "
				 "expected");
		} else {
			tst_resm(TFAIL, "setreuid FAILED, expected 1 but "
				 "returned %d", TEST_ERRNO);
		}

	}
	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_require_root();

	tst_sig(FORK, DEF_HANDLER, cleanup);

	umask(0);

	ltpuser = getpwnam("nobody");
	if (ltpuser == NULL)
		tst_brkm(TBROK, NULL, "nobody must be a valid user.");

	SAFE_SETUID(NULL, ltpuser->pw_uid);

	TEST_PAUSE;
}

static void cleanup(void)
{
}
