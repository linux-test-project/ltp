// SPDX-License-Identifier: GPL-2.0-or-later
/*

  Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>

 */
/*

   Test that clock_nanosleep() adds correctly an offset with absolute timeout
   and CLOCK_MONOTONIC inside of a timer namespace.

   After a call to unshare(CLONE_NEWTIME) a new timer namespace is created, the
   process that has called the unshare() can adjust offsets for CLOCK_MONOTONIC
   and CLOCK_BOOTTIME for its children by writing to the '/proc/self/timens_offsets'.

 */

#include <stdlib.h>
#include "tst_safe_clocks.h"
#include "tst_timer.h"
#include "lapi/namespaces_constants.h"
#include "tst_test.h"

#define OFFSET_S 10
#define SLEEP_US 100000

static void verify_clock_nanosleep(void)
{
	struct timespec start, end, sleep_abs;

	SAFE_UNSHARE(CLONE_NEWTIME);

	SAFE_FILE_PRINTF("/proc/self/timens_offsets", "%d %d 0", CLOCK_MONOTONIC, OFFSET_S);

	SAFE_CLOCK_GETTIME(CLOCK_MONOTONIC, &start);

	sleep_abs = tst_timespec_add_us(start, 1000000 * OFFSET_S + SLEEP_US);

	if (!SAFE_FORK()) {
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &sleep_abs, NULL);
		exit(0);
	}

	SAFE_WAIT(NULL);

	SAFE_CLOCK_GETTIME(CLOCK_MONOTONIC, &end);

	long long diff = tst_timespec_diff_us(end, start);

	if (diff > 5 * SLEEP_US) {
		tst_res(TFAIL, "clock_nanosleep() slept too long %lli", diff);
		return;
	}

	if (diff < SLEEP_US) {
		tst_res(TFAIL, "clock_nanosleep() slept too short %lli", diff);
		return;
	}

	tst_res(TPASS, "clock_nanosleep() slept correctly %lli", diff);
}

static struct tst_test test = {
	.test_all = verify_clock_nanosleep,
	.needs_root = 1,
	.forks_child = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_TIME_NS=y",
		NULL
	}

};
