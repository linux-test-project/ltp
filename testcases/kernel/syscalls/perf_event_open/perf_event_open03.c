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
#include "lapi/syscalls.h"

#include "perf_event_open.h"

#define INTEL_PT_PATH "/sys/bus/event_source/devices/intel_pt/type"

static int fd = -1;

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
}

static void run(void)
{
	long diff;
	int i;

	diff = SAFE_READ_MEMINFO("MemAvailable:");

	/* leak about 100MB of RAM */
	for (i = 0; i < 12000000; i++)
		ioctl(fd, PERF_EVENT_IOC_SET_FILTER, "filter,0/0@abcd");

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
