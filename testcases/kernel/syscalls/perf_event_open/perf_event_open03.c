// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 SUSE LLC <mdoucha@suse.cz>
 * Copyright (c) 2022 Linux Test Project
 *
 * CVE-2020-25704
 *
 * Check for memory leak in PERF_EVENT_IOC_SET_FILTER ioctl command. Fixed in:
 *
 *  commit 7bdb157cdebbf95a1cd94ed2e01b338714075d00
 *  Author: kiyin(尹亮) <kiyin@tencent.com>
 *  Date:   Wed Nov 4 08:23:22 2020 +0300
 *
 *  perf/core: Fix a memory leak in perf_event_parse_addr_filter()
 */

#include "config.h"
#include "tst_test.h"
#include "tst_timer_test.h"
#include "lapi/syscalls.h"

#include "perf_event_open.h"

#define INTEL_PT_PATH "/sys/bus/event_source/devices/intel_pt/type"

const int iterations = 12000000;
static int fd = -1;
static int timeout;

static void setup(void)
{
	struct perf_event_attr ev = {
		.size = sizeof(struct perf_event_attr),
		.exclude_kernel = 1,
		.exclude_hv = 1,
		.exclude_idle = 1
	};

	/* intel_pt is currently the only event source that supports filters */
	if (access(INTEL_PT_PATH, F_OK))
		tst_brk(TCONF, "intel_pt is not available");

	SAFE_FILE_SCANF(INTEL_PT_PATH, "%d", &ev.type);
	fd = perf_event_open(&ev, getpid(), -1, -1, 0);

	timeout = tst_timeout_remaining();
}

/*
 * Check how fast we can do the iterations after 5 seconds of runtime.
 * If the rate is too small to complete for current timeout then
 * stop the test.
 */
static void check_progress(int i)
{
	static float iter_per_ms;
	long long elapsed_ms;

	if (iter_per_ms)
		return;

	if (i % 1000 != 0)
		return;

	tst_timer_stop();
	elapsed_ms = tst_timer_elapsed_ms();
	if (elapsed_ms > 5000) {
		iter_per_ms = (float) i / elapsed_ms;
		tst_res(TINFO, "rate: %f iters/ms", iter_per_ms);
		tst_res(TINFO, "needed rate for current test timeout: %f iters/ms",
			(float) iterations / (timeout * 1000));

		if (iter_per_ms * 1000 * (timeout - 1) < iterations)
			tst_brk(TCONF, "System too slow to complete test in specified timeout");
	}
}

static void run(void)
{
	long diff;
	int i;

	diff = SAFE_READ_MEMINFO("MemAvailable:");
	tst_timer_start(CLOCK_MONOTONIC);

	/* leak about 100MB of RAM */
	for (i = 0; i < iterations; i++) {
		ioctl(fd, PERF_EVENT_IOC_SET_FILTER, "filter,0/0@abcd");
		check_progress(i);
	}

	diff -= SAFE_READ_MEMINFO("MemAvailable:");

	if (diff > 50 * 1024)
		tst_res(TFAIL, "Likely kernel memory leak detected");
	else
		tst_res(TPASS, "No memory leak found");
}

static void cleanup(void)
{
	if (fd >= 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "7bdb157cdebb"},
		{"CVE", "2020-25704"},
		{}
	}
};
