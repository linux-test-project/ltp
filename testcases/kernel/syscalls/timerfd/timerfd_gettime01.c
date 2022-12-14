// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 *
 * DESCRIPTION
 *  Verify that,
 *   1. fd is not a valid file descriptor, EBADF would return.
 *   2. curr_value is not valid a pointer, EFAULT would return.
 *   3. fd is not a valid timerfd file descriptor, EINVAL would return.
 */

#define _GNU_SOURCE

#include "time64_variants.h"
#include "tst_timer.h"
#include "tst_safe_timerfd.h"

char *TCID = "timerfd_gettime01";

static int bad_clockfd = -1;
static int clockfd;
static int fd;
static void *bad_addr;

static struct test_case_t {
	int *fd;
	struct tst_its *curr_value;
	int exp_errno;
} test_cases[] = {
	{&bad_clockfd, NULL, EBADF},
	{&clockfd, NULL, EFAULT},
	{&fd, NULL, EINVAL},
};

static struct time64_variants variants[] = {
#if (__NR_timerfd_gettime != __LTP__NR_INVALID_SYSCALL)
	{ .tfd_gettime = sys_timerfd_gettime, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_timerfd_gettime64 != __LTP__NR_INVALID_SYSCALL)
	{ .tfd_gettime = sys_timerfd_gettime64, .desc = "syscall time64 with kernel spec"},
#endif
};

static void setup(void)
{
	tst_res(TINFO, "Testing variant: %s", variants[tst_variant].desc);
	bad_addr = tst_get_bad_addr(NULL);

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
		its = tst_its_get(test->curr_value);

	TEST(tv->tfd_gettime(*test->fd, its));

	if (TST_RET != -1) {
		tst_res(TFAIL, "timerfd_gettime() succeeded unexpectedly");
		return;
	}

	if (TST_ERR == test->exp_errno) {
		tst_res(TPASS | TTERRNO,
			"timerfd_gettime() failed as expected");
	} else {
		tst_res(TFAIL | TTERRNO,
			"timerfd_gettime() failed unexpectedly; expected: "
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
