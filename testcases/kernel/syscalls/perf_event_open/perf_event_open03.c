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
#include "tst_timer.h"
#include "lapi/syscalls.h"

#include "perf_event_open.h"

#define INTEL_PT_PATH "/sys/bus/event_source/devices/intel_pt/type"

const int iterations = 12000000;
static int fd = -1;
static int runtime;

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

	runtime = tst_remaining_runtime();
}

/*
 * Check how fast we can do the iterations after 5 seconds of runtime.
 * If the rate is too small to complete for current runtime then
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
		tst_res(TINFO, "needed rate for current test runtime: %f iters/ms",
			(float) iterations / (runtime * 1000));

		if (iter_per_ms * 1000 * (runtime - 1) < iterations)
			tst_brk(TCONF, "System too slow to complete test in specified runtime");
	}
}

static void run(void)
{
	long diff, diff_total, mem_avail, mem_avail_prev;
	int i, sample;

	sample = 0;
	diff_total = 0;

	mem_avail_prev = SAFE_READ_MEMINFO("MemAvailable:");
	tst_timer_start(CLOCK_MONOTONIC);

	/* leak about 100MB of RAM */
	for (i = 0; i < iterations; i++) {
		ioctl(fd, PERF_EVENT_IOC_SET_FILTER, "filter,0/0@abcd");
		check_progress(i);

		/*
		 * Every 1200000 iterations, calculate the difference in memory
		 * availability. If the difference is greater than 20 * 1024 (20MB),
		 * increment the sample counter and log the event.
		 */
		if ((i % 1200000) == 0) {
			mem_avail = SAFE_READ_MEMINFO("MemAvailable:");
			diff = mem_avail_prev - mem_avail;
			diff_total += diff;

			if (diff > 20 * 1024) {
				sample++;
				tst_res(TINFO, "MemAvailable decreased by %ld kB at iteration %d", diff, i);
			}

			mem_avail_prev = mem_avail;
		}
	}

	if ((sample > 5) || (diff_total > 100 * 1024))
		tst_res(TFAIL, "Likely kernel memory leak detected, total decrease: %ld kB", diff_total);
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
	.runtime = 300,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "7bdb157cdebb"},
		{"CVE", "2020-25704"},
		{}
	}
};
