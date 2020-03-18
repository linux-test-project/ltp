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
#include "tst_test.h"

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

static struct timespec now;
static int parent_ns;

static void child(struct tcase *tc)
{
	struct timespec then;
	struct timespec parent_then;
	long long diff;

	SAFE_CLOCK_GETTIME(tc->clk_id, &then);

	SAFE_SETNS(parent_ns, CLONE_NEWTIME);

	SAFE_CLOCK_GETTIME(tc->clk_id, &parent_then);

	diff = tst_timespec_diff_ms(then, now);

	if (diff/1000 != tc->off) {
		tst_res(TFAIL, "Wrong offset (%s) read %llims",
		        tst_clock_name(tc->clk_id), diff);
	} else {
		tst_res(TPASS, "Offset (%s) is correct %llims",
		        tst_clock_name(tc->clk_id), diff);
	}

	diff = tst_timespec_diff_ms(parent_then, now);

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
	struct tcase *tc = &tcases[n];

	SAFE_UNSHARE(CLONE_NEWTIME);

	SAFE_FILE_PRINTF("/proc/self/timens_offsets", "%d %d 0",
	                 tc->clk_off, tc->off);

	SAFE_CLOCK_GETTIME(tc->clk_id, &now);

	if (!SAFE_FORK())
		child(tc);
}

static void setup(void)
{
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
	.needs_root = 1,
	.forks_child = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_TIME_NS=y",
		NULL
	}
};
