// SPDX-License-Identifier: GPL-2.0-or-later
/*

  Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>

 */
/*

  Basic test for timer namespaces.

  After a call to unshare(CLONE_NEWTIME) a new timer namespace is created, the
  process that has called the unshare() can adjust offsets for CLOCK_MONOTONIC
  and CLOCK_BOOTTIME for its children by writing to the '/proc/self/timens_offsets'.

  The child processes also switch to the initial parent namespace and checks
  that the offset is set to 0.

 */

#define _GNU_SOURCE
#include "tst_safe_clocks.h"
#include "tst_timer.h"
#include "lapi/namespaces_constants.h"

static struct tcase {
	int clk_id;
	int clk_off;
	int off;
} tcases[] = {
	{CLOCK_MONOTONIC, CLOCK_MONOTONIC, 10},
	{CLOCK_BOOTTIME, CLOCK_BOOTTIME, 10},

	{CLOCK_MONOTONIC, CLOCK_MONOTONIC, -10},
	{CLOCK_BOOTTIME, CLOCK_BOOTTIME, -10},

	{CLOCK_MONOTONIC_RAW, CLOCK_MONOTONIC, 100},
	{CLOCK_MONOTONIC_COARSE, CLOCK_MONOTONIC, 100},
};

static struct tst_ts now, then, parent_then;
static int parent_ns;

static struct test_variants {
	int (*func)(clockid_t clk_id, void *ts);
	enum tst_ts_type type;
	char *desc;
} variants[] = {
	{ .func = libc_clock_gettime, .type = TST_LIBC_TIMESPEC, .desc = "vDSO or syscall with libc spec"},

#if (__NR_clock_gettime != __LTP__NR_INVALID_SYSCALL)
	{ .func = sys_clock_gettime, .type = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_clock_gettime64 != __LTP__NR_INVALID_SYSCALL)
	{ .func = sys_clock_gettime64, .type = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

static void child(struct test_variants *tv, struct tcase *tc)
{
	long long diff;

	if (tv->func(tc->clk_id, tst_ts_get(&then))) {
		tst_res(TFAIL | TERRNO, "clock_gettime(%s) failed",
			tst_clock_name(tc->clk_id));
		return;
	}

	SAFE_SETNS(parent_ns, CLONE_NEWTIME);

	if (tv->func(tc->clk_id, tst_ts_get(&parent_then))) {
		tst_res(TFAIL | TERRNO, "clock_gettime(%s) failed",
			tst_clock_name(tc->clk_id));
		return;
	}

	diff = tst_ts_diff_ms(then, now);

	if (diff/1000 != tc->off) {
		tst_res(TFAIL, "Wrong offset (%s) read %llims",
		        tst_clock_name(tc->clk_id), diff);
	} else {
		tst_res(TPASS, "Offset (%s) is correct %llims",
		        tst_clock_name(tc->clk_id), diff);
	}

	diff = tst_ts_diff_ms(parent_then, now);

	if (diff/1000) {
		tst_res(TFAIL, "Wrong offset (%s) read %llims",
		        tst_clock_name(tc->clk_id), diff);
	} else {
		tst_res(TPASS, "Offset (%s) is correct %llims",
		        tst_clock_name(tc->clk_id), diff);
	}
}

static void verify_ns_clock(unsigned int n)
{
	struct test_variants *tv = &variants[tst_variant];
	struct tcase *tc = &tcases[n];

	SAFE_UNSHARE(CLONE_NEWTIME);

	SAFE_FILE_PRINTF("/proc/self/timens_offsets", "%d %d 0",
	                 tc->clk_off, tc->off);

	if (tv->func(tc->clk_id, tst_ts_get(&now))) {
		tst_res(TFAIL | TERRNO, "%d clock_gettime(%s) failed",
			__LINE__, tst_clock_name(tc->clk_id));
		return;
	}

	if (!SAFE_FORK())
		child(tv, tc);
}

static void setup(void)
{
	struct test_variants *tv = &variants[tst_variant];

	now.type = then.type = parent_then.type = tv->type;
	tst_res(TINFO, "Testing variant: %s", variants[tst_variant].desc);
	parent_ns = SAFE_OPEN("/proc/self/ns/time_for_children", O_RDONLY);
}

static void cleanup(void)
{
	SAFE_CLOSE(parent_ns);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_ns_clock,
	.test_variants = ARRAY_SIZE(variants),
	.needs_root = 1,
	.forks_child = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_TIME_NS=y",
		NULL
	}
};
