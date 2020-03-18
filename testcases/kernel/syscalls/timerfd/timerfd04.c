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
#include "tst_safe_clocks.h"
#include "tst_safe_timerfd.h"
#include "tst_timer.h"
#include "lapi/namespaces_constants.h"
#include "tst_test.h"

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

static void verify_timerfd(unsigned int n)
{
	struct timespec start, end;
	struct itimerspec it = {};
	struct tcase *tc = &tcases[n];

	SAFE_UNSHARE(CLONE_NEWTIME);

	SAFE_FILE_PRINTF("/proc/self/timens_offsets", "%d %d 0",
	                 tc->clk_off, tc->off);

	SAFE_CLOCK_GETTIME(tc->clk_id, &start);

	it.it_value = tst_timespec_add_us(start, 1000000 * tc->off + SLEEP_US);

	if (!SAFE_FORK()) {
		uint64_t exp;
		int fd = SAFE_TIMERFD_CREATE(tc->clk_id, 0);

		SAFE_TIMERFD_SETTIME(fd, TFD_TIMER_ABSTIME, &it, NULL);

		SAFE_READ(1, fd, &exp, sizeof(exp));

		if (exp != 1)
			tst_res(TFAIL, "Got %llu expirations", (long long unsigned)exp);

		SAFE_CLOSE(fd);
		exit(0);
	}

	SAFE_WAIT(NULL);

	SAFE_CLOCK_GETTIME(CLOCK_MONOTONIC, &end);

	long long diff = tst_timespec_diff_us(end, start);

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
	.needs_root = 1,
	.forks_child = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_TIME_NS=y",
		NULL
	}
};
