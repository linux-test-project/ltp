// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007-2008 ,Hitachi, Ltd
 * Authors: Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,
 *              Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,
 *              Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>
 * LTP authors:
 *              Manas Kumar Nayak maknayak@in.ibm.com>
 *              Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 *              Cyril Hrubis <chrubis@suse.cz>
 */

#include <errno.h>

#include "tst_test.h"
#include "lapi/syscalls.h"
#include "lapi/posix_clocks.h"

static struct timespec *res;

static struct test_case {
	char *name;
	clockid_t clk_id;
	int ret;
	int err;
} tcase[] = {
	{"REALTIME", CLOCK_REALTIME, 0, 0},
	{"MONOTONIC", CLOCK_MONOTONIC, 0, 0},
	{"PROCESS_CPUTIME_ID", CLOCK_PROCESS_CPUTIME_ID, 0, 0},
	{"THREAD_CPUTIME_ID", CLOCK_THREAD_CPUTIME_ID, 0, 0},
	{"CLOCK_MONOTONIC_RAW", CLOCK_MONOTONIC_RAW, 0, 0,},
	{"CLOCK_REALTIME_COARSE", CLOCK_REALTIME_COARSE, 0, 0,},
	{"CLOCK_MONOTONIC_COARSE", CLOCK_MONOTONIC_COARSE, 0, 0,},
	{"CLOCK_BOOTTIME", CLOCK_BOOTTIME, 0, 0,},
	{"CLOCK_REALTIME_ALARM", CLOCK_REALTIME_ALARM, 0, 0,},
	{"CLOCK_BOOTTIME_ALARM", CLOCK_BOOTTIME_ALARM, 0, 0,},
	{"-1", -1, -1, EINVAL},
};

static const char *variant_desc[] = {
	"default (vdso or syscall)",
	"syscall",
	"syscall with NULL res parameter" };

static void setup(void)
{
	tst_res(TINFO, "Testing variant: %s", variant_desc[tst_variant]);
}

static void do_test(unsigned int i)
{
	switch (tst_variant) {
	case 0:
		TEST(clock_getres(tcase[i].clk_id, res));
		break;
	case 1:
		TEST(tst_syscall(__NR_clock_getres, tcase[i].clk_id, res));
		break;
	case 2:
		TEST(tst_syscall(__NR_clock_getres, tcase[i].clk_id, NULL));
		break;
	}

	if (TST_RET != tcase[i].ret) {
		if (TST_ERR == EINVAL) {
			tst_res(TCONF, "clock_getres(%s, ...) NO SUPPORTED", tcase[i].name);
			return;
		}

		tst_res(TFAIL | TTERRNO, "clock_getres(%s, ...) failed", tcase[i].name);
		return;
	}

	if (TST_ERR != tcase[i].err) {
		tst_res(TFAIL,
			"clock_getres(%s, ...) failed unexpectedly: %s, expected: %s",
			tcase[i].name, tst_strerrno(TST_ERR), tst_strerrno(tcase[i].err));
		return;
	}

	tst_res(TPASS, "clock_getres(%s, ...) succeeded", tcase[i].name);
}

static struct tst_test test = {
	.test = do_test,
	.tcnt = ARRAY_SIZE(tcase),
	.test_variants = 3,
	.setup = setup,
	.bufs = (struct tst_buffers []) {
		{&res, .size = sizeof(*res)},
		{},
	}
};
