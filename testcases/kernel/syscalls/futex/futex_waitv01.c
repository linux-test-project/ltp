// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test verifies EINVAL for futex_waitv syscall.
 */

#include <time.h>
#include "tst_test.h"
#include "lapi/futex.h"
#include "futex2test.h"
#include "tst_safe_clocks.h"

static uint32_t *futex;
static struct futex_waitv *waitv;

static void setup(void)
{
	futex = SAFE_MALLOC(sizeof(uint32_t));
	*futex = FUTEX_INITIALIZER;
}

static void init_timeout(struct timespec *to)
{
	SAFE_CLOCK_GETTIME(CLOCK_MONOTONIC, to);
	to->tv_sec++;
}

static void init_waitv(void)
{
	waitv->uaddr = (uintptr_t)&futex;
	waitv->flags = FUTEX_32 | FUTEX_PRIVATE_FLAG;
	waitv->val = 0;
}

static void test_invalid_flags(void)
{
	struct timespec to;

	init_waitv();
	init_timeout(&to);

	/* Testing a waiter without FUTEX_32 flag */
	waitv->flags = FUTEX_PRIVATE_FLAG;

	TST_EXP_FAIL(futex_waitv(waitv, 1, 0, &to, CLOCK_MONOTONIC), EINVAL,
		     "futex_waitv with invalid flags");
}

static void test_unaligned_address(void)
{
	struct timespec to;

	init_waitv();
	init_timeout(&to);

	waitv->uaddr = 1;

	TST_EXP_FAIL(futex_waitv(waitv, 1, 0, &to, CLOCK_MONOTONIC), EINVAL,
		     "futex_waitv with unligned address");
}

static void test_null_address(void)
{
	struct timespec to;

	init_waitv();
	init_timeout(&to);

	waitv->uaddr = 0x00000000;

	TST_EXP_FAIL(futex_waitv(waitv, 1, 0, &to, CLOCK_MONOTONIC), EFAULT,
		     "futex_waitv address is NULL");
}

static void test_null_waiters(void)
{
	struct timespec to;

	init_timeout(&to);

	TST_EXP_FAIL(futex_waitv(NULL, 1, 0, &to, CLOCK_MONOTONIC), EINVAL,
		     "futex_waitv waiters are NULL");
}

static void test_invalid_clockid(void)
{
	struct timespec to;

	init_waitv();
	init_timeout(&to);

	TST_EXP_FAIL(futex_waitv(waitv, 1, 0, &to, CLOCK_TAI), EINVAL,
		     "futex_waitv invalid clockid");
}

static void run(void)
{
	test_invalid_flags();
	test_unaligned_address();
	test_null_address();
	test_null_waiters();
	test_invalid_clockid();
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.min_kver = "5.16",
	.bufs =
		(struct tst_buffers[]){
			{ &waitv, .size = sizeof(struct futex_waitv) },
			{},
		},
};
