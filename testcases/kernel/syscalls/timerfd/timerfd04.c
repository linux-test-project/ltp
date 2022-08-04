// SPDX-License-Identifier: GPL-2.0-or-later
/*

  Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>

 */
/*

   Test that timerfd adds correctly an offset with absolute expiration time.

   After a call to unshare(CLONE_NEWTIME) a new timer namespace is created, the
   process that has called the unshare() can adjust offsets for CLOCK_MONOTONIC
   and CLOCK_BOOTTIME for its children by writing to the '/proc/self/timens_offsets'.

 */

#include <stdlib.h>
#include "time64_variants.h"
#include "tst_safe_clocks.h"
#include "tst_safe_timerfd.h"
#include "tst_timer.h"
#include "lapi/namespaces_constants.h"

#define SLEEP_US 40000

static struct tcase {
	int clk_id;
	int clk_off;
	int off;
} tcases[] = {
	{CLOCK_MONOTONIC, CLOCK_MONOTONIC, 10},
	{CLOCK_BOOTTIME, CLOCK_BOOTTIME, 10},

	{CLOCK_MONOTONIC, CLOCK_MONOTONIC, -10},
	{CLOCK_BOOTTIME, CLOCK_BOOTTIME, -10},
};

static struct time64_variants variants[] = {
#if (__NR_timerfd_settime != __LTP__NR_INVALID_SYSCALL)
	{ .clock_gettime = sys_clock_gettime, .tfd_settime = sys_timerfd_settime, .ts_type = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_timerfd_settime64 != __LTP__NR_INVALID_SYSCALL)
	{ .clock_gettime = sys_clock_gettime64, .tfd_settime = sys_timerfd_settime64, .ts_type = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

static void setup(void)
{
	tst_res(TINFO, "Testing variant: %s", variants[tst_variant].desc);
}

static void verify_timerfd(unsigned int n)
{
	struct time64_variants *tv = &variants[tst_variant];
	struct tst_ts start, end;
	struct tst_its it;
	struct tcase *tc = &tcases[n];

	start.type = end.type = it.type = tv->ts_type;
	SAFE_UNSHARE(CLONE_NEWTIME);

	SAFE_FILE_PRINTF("/proc/self/timens_offsets", "%d %d 0",
	                 tc->clk_off, tc->off);

	if (tv->clock_gettime(tc->clk_id, tst_ts_get(&start))) {
		tst_res(TFAIL | TERRNO, "clock_gettime(2) failed for clock %s",
			tst_clock_name(tc->clk_id));
		return;
	}

	end = tst_ts_add_us(start, 1000000 * tc->off + SLEEP_US);
	tst_its_set_interval_sec(&it, 0);
	tst_its_set_interval_nsec(&it, 0);
	tst_its_set_value_from_ts(&it, end);

	if (!SAFE_FORK()) {
		uint64_t exp;
		int fd = SAFE_TIMERFD_CREATE(tc->clk_id, 0);

		if (tv->tfd_settime(fd, TFD_TIMER_ABSTIME, tst_its_get(&it), NULL)) {
			tst_res(TFAIL | TERRNO, "timerfd_settime() failed");
			return;
		}

		SAFE_READ(1, fd, &exp, sizeof(exp));

		if (exp != 1)
			tst_res(TFAIL, "Got %llu expirations", (long long unsigned)exp);

		SAFE_CLOSE(fd);
		exit(0);
	}

	SAFE_WAIT(NULL);

	if (tv->clock_gettime(tc->clk_id, tst_ts_get(&end))) {
		tst_res(TFAIL | TERRNO, "clock_gettime(2) failed for clock %s",
			tst_clock_name(tc->clk_id));
		return;
	}

	long long diff = tst_ts_diff_us(end, start);

	if (diff > 5 * SLEEP_US) {
		tst_res(TFAIL, "timerfd %s slept too long %lli",
		        tst_clock_name(tc->clk_id), diff);
		return;
	}

	if (diff < SLEEP_US) {
		tst_res(TFAIL, "timerfd %s slept too short %lli",
		        tst_clock_name(tc->clk_id), diff);
		return;
	}

	tst_res(TPASS, "timerfd %s slept correctly %lli",
		tst_clock_name(tc->clk_id), diff);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_timerfd,
	.test_variants = ARRAY_SIZE(variants),
	.setup = setup,
	.needs_root = 1,
	.forks_child = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_TIME_NS=y",
		NULL
	}
};
