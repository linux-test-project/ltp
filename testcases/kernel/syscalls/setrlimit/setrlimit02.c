// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Ported to LTP: Wayne Boyer
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
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

	if (TST_RET != -1) {
		tst_res(TFAIL, "call succeeded unexpectedly");
		return;
	}

	if (TST_ERR != tc->exp_errno) {
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
