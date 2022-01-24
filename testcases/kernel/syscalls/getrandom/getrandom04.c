// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2015 Cedric Hnyda <ced.hnyda@gmail.com>
 *
 * Calls getrandom(2) after having limited the number of available file
 * descriptors to 3 and expects success.
 */

#include <sys/resource.h>
#include "tst_test.h"
#include "lapi/getrandom.h"
#include "lapi/syscalls.h"

static void verify_getrandom(void)
{
	char buf[128];
	struct rlimit lold, lnew;

	SAFE_GETRLIMIT(RLIMIT_NOFILE, &lold);
	lnew.rlim_max = lold.rlim_max;
	lnew.rlim_cur = 3;
	SAFE_SETRLIMIT(RLIMIT_NOFILE, &lnew);

	TEST(tst_syscall(__NR_getrandom, buf, 100, 0));
	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO, "getrandom failed");
	else
		tst_res(TPASS, "getrandom returned %ld", TST_RET);

	SAFE_SETRLIMIT(RLIMIT_NOFILE, &lold);
}

static struct tst_test test = {
	.test_all = verify_getrandom,
};
