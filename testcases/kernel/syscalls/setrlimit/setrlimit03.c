// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (c) 2017 Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*\
 * Test :manpage:`setrlimit(2)` errnos:
 *
 * - EPERM when the super-user tries to increase RLIMIT_NOFILE  beyond the
 *   system limit.
 * - EINVAL when rlim->rlim_cur is greater than rlim->rlim_max.
 */

#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <linux/fs.h>
#include "tst_test.h"

#if !defined(NR_OPEN)
// Taken from definition in /usr/include/linux/fs.h
# define NR_OPEN (1024*1024)
#endif

#define NR_OPEN_PATH "/proc/sys/fs/nr_open"

static struct rlimit rlim1, rlim2;
static unsigned int nr_open = NR_OPEN;

static struct tcase {
	struct rlimit *rlimt;
	int exp_err;
} tcases[] = {
	{&rlim1, EPERM},
	{&rlim2, EINVAL}
};

static void verify_setrlimit(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(setrlimit(RLIMIT_NOFILE, tc->rlimt));
	if (TST_RET != -1) {
		tst_res(TFAIL, "call succeeded unexpectedly "
			"(nr_open=%u rlim_cur=%lu rlim_max=%lu)", nr_open,
			(unsigned long)(tc->rlimt->rlim_cur),
			(unsigned long)(tc->rlimt->rlim_max));
		return;
	}

	if (TST_ERR != tc->exp_err) {
		tst_res(TFAIL | TTERRNO, "setrlimit() should fail with %s, got",
			tst_strerrno(tc->exp_err));
	} else {
		tst_res(TPASS | TTERRNO, "setrlimit() failed as expected");
	}
}

static void setup(void)
{
	if (!access(NR_OPEN_PATH, F_OK))
		SAFE_FILE_SCANF(NR_OPEN_PATH, "%u", &nr_open);

	SAFE_GETRLIMIT(RLIMIT_NOFILE, &rlim1);
	rlim2.rlim_max = rlim1.rlim_cur;
	rlim2.rlim_cur = rlim1.rlim_max + 1;
	rlim1.rlim_max = nr_open + 1;
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_setrlimit,
	.needs_root = 1
};
