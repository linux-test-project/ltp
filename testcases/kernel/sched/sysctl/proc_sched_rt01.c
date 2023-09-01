// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * [Description]
 *
 * Sanity tests for the /proc/sys/kernel/sched_r* files.
 *
 * - The sched_rt_period_us range is 1 to INT_MAX
 *   try invalid values and check for EINVAL
 *
 * - The sched_rt_runtime_us range is -1 to INT_MAX
 *   try invalid values and check for EINVAL
 *
 * - The sched_rt_runtime_us must be less or equal to sched_rt_period_us
 *
 * - Reset sched_rr_timeslice_ms to default value by writing -1 and check that
 *   we get the default value on next read.
 *
 * This is a regression test for a commits:
 *
 *  - c1fc6484e1fb ("sched/rt: sysctl_sched_rr_timeslice show default timeslice after reset")
 *  - 079be8fc6309 ("sched/rt: Disallow writing invalid values to sched_rt_period_us")
 */

#include <stdio.h>
#include "tst_test.h"

#define RT_PERIOD_US "/proc/sys/kernel/sched_rt_period_us"
#define RT_RUNTIME_US "/proc/sys/kernel/sched_rt_runtime_us"
#define RR_TIMESLICE_MS "/proc/sys/kernel/sched_rr_timeslice_ms"

static int period_fd;
static int runtime_fd;

static void rr_timeslice_ms_reset(void)
{
	long timeslice_ms;

	SAFE_FILE_PRINTF(RR_TIMESLICE_MS, "-1");
	SAFE_FILE_SCANF(RR_TIMESLICE_MS, "%li", &timeslice_ms);

	TST_EXP_EXPR(timeslice_ms > 0,
		"timeslice_ms > 0 after reset to default");
}

static void rt_period_us_einval(void)
{
	TST_EXP_FAIL(write(period_fd, "0", 2), EINVAL,
		"echo 0 > "RT_PERIOD_US);
	TST_EXP_FAIL(write(period_fd, "-1", 2), EINVAL,
		"echo -1 > "RT_PERIOD_US);
}

static void rt_runtime_us_einval(void)
{
	TST_EXP_FAIL(write(runtime_fd, "-2", 2), EINVAL,
		"echo -2 > "RT_RUNTIME_US);
}

static void rt_runtime_us_le_period_us(void)
{
	int period_us;
	char buf[32];

	SAFE_FILE_SCANF(RT_PERIOD_US, "%i", &period_us);

	sprintf(buf, "%i", period_us+1);

	TST_EXP_FAIL(write(runtime_fd, buf, strlen(buf)), EINVAL,
		"echo rt_period_us+1 > "RT_RUNTIME_US);
}

static void verify_sched_proc(void)
{
	rr_timeslice_ms_reset();
	rt_period_us_einval();
	rt_runtime_us_einval();
	rt_runtime_us_le_period_us();
}

static void setup(void)
{
	period_fd = open(RT_PERIOD_US, O_RDWR);
	runtime_fd = open(RT_RUNTIME_US, O_RDWR);
}

static void cleanup(void)
{
	if (period_fd > 0)
		SAFE_CLOSE(period_fd);

	if (runtime_fd > 0)
		SAFE_CLOSE(runtime_fd);
}

static struct tst_test test = {
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_sched_proc,
	.tags = (struct tst_tag []) {
		{"linux-git", "c1fc6484e1fb"},
		{"linux-git", "079be8fc6309"},
		{}
	},
	.needs_kconfigs = (const char *[]) {
		"CONFIG_SYSCTL",
		NULL
	},
};
