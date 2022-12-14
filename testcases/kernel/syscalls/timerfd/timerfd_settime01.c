// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 *
 * DESCRIPTION
 *  Verify that,
 *   1. fd is not a valid file descriptor, EBADF would return.
 *   2. old_value is not valid a pointer, EFAULT would return.
 *   3. fd is not a valid timerfd file descriptor, EINVAL would return.
 *   4. flags is invalid, EINVAL would return.
 */

#define _GNU_SOURCE

#include "time64_variants.h"
#include "tst_timer.h"
#include "lapi/timerfd.h"

static int bad_clockfd = -1;
static int clockfd;
static int fd;
static void *bad_addr;

static struct test_case_t {
	int *fd;
	int flags;
	struct tst_its *old_value;
	int exp_errno;
} test_cases[] = {
	{&bad_clockfd, 0, NULL, EBADF},
	{&clockfd, 0, NULL, EFAULT},
	{&fd, 0, NULL, EINVAL},
	{&clockfd, -1, NULL, EINVAL},
};

static struct tst_its new_value;

static struct time64_variants variants[] = {
#if (__NR_timerfd_settime != __LTP__NR_INVALID_SYSCALL)
	{ .tfd_settime = sys_timerfd_settime, .ts_type = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_timerfd_settime64 != __LTP__NR_INVALID_SYSCALL)
	{ .tfd_settime = sys_timerfd_settime64, .ts_type = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

static void setup(void)
{
	struct time64_variants *tv = &variants[tst_variant];

	tst_res(TINFO, "Testing variant: %s", tv->desc);
	bad_addr = tst_get_bad_addr(NULL);
	new_value.type = tv->ts_type;

	clockfd = timerfd_create(CLOCK_REALTIME, 0);
	if (clockfd == -1) {
		tst_brk(TFAIL | TERRNO, "timerfd_create() fail");
		return;
	}

	fd = SAFE_OPEN("test_file", O_RDWR | O_CREAT, 0644);
}

static void cleanup(void)
{
	if (clockfd > 0)
		close(clockfd);

	if (fd > 0)
		close(fd);
}

static void run(unsigned int n)
{
	struct time64_variants *tv = &variants[tst_variant];
	struct test_case_t *test = &test_cases[n];
	void *its;

	if (test->exp_errno == EFAULT)
		its = bad_addr;
	else
		its = tst_its_get(test->old_value);

	TEST(tv->tfd_settime(*test->fd, test->flags, tst_its_get(&new_value),
			     its));

	if (TST_RET != -1) {
		tst_res(TFAIL, "timerfd_settime() succeeded unexpectedly");
		return;
	}

	if (TST_ERR == test->exp_errno) {
		tst_res(TPASS | TTERRNO,
			"timerfd_settime() failed as expected");
	} else {
		tst_res(TFAIL | TTERRNO,
			"timerfd_settime() failed unexpectedly; expected: "
			"%d - %s", test->exp_errno, strerror(test->exp_errno));
	}
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(test_cases),
	.test_variants = ARRAY_SIZE(variants),
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
