/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by Wayne Boyer
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
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
 * Testcase to test the different errnos set by setrlimit(2) system call.
 */
#include <pwd.h>
#include <errno.h>
#include "tst_test.h"

static char nobody_uid[] = "nobody";
static struct rlimit rlim;

static struct tcase {
	int resource;
	struct rlimit *rlim;
	int exp_errno;
} tcases[] = {
	{-1, &rlim, EINVAL},
	{RLIMIT_NOFILE, &rlim, EPERM}
};

static void verify_setrlimit(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(setrlimit(tc->resource, tc->rlim));

	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "call succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO != tc->exp_errno) {
		tst_res(TFAIL | TTERRNO,
			"setrlimit() should fail with %s got",
			tst_strerrno(tc->exp_errno));
		return;
	}

	tst_res(TPASS | TTERRNO, "setrlimit() failed as expected");
}

static void setup(void)
{
	struct passwd *ltpuser = SAFE_GETPWNAM(nobody_uid);

	SAFE_SETUID(ltpuser->pw_uid);

	SAFE_GETRLIMIT(RLIMIT_NOFILE, &rlim);
	rlim.rlim_max++;
}

static struct tst_test test = {
	.setup = setup,
	.test = verify_setrlimit,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
};
