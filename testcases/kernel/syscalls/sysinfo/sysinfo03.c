// SPDX-License-Identifier: GPL-2.0-or-later
/*
  Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>
 */
/*

  Test if CLOCK_BOOTTIME namespace offset is applied to sysinfo uptime and that
  it's consistent with /proc/uptime as well.

  After a call to unshare(CLONE_NEWTIME) a new timer namespace is created, the
  process that has called the unshare() can adjust offsets for CLOCK_MONOTONIC
  and CLOCK_BOOTTIME for its children by writing to the '/proc/self/timens_offsets'.

 */

#include <sys/sysinfo.h>
#include "lapi/namespaces_constants.h"
#include "lapi/posix_clocks.h"
#include "tst_test.h"

static int offsets[] = {
	10,
	-10,
	3600,
};

static long read_proc_uptime(void)
{
	long sec, sec_rem;

	SAFE_FILE_SCANF("/proc/uptime", "%li.%li", &sec, &sec_rem);

	return sec + (sec_rem ? 1 : 0);
}

static void verify_sysinfo(unsigned int n)
{
	struct sysinfo si;
	long uptime;
	int off = offsets[n];

	SAFE_UNSHARE(CLONE_NEWTIME);

        SAFE_FILE_PRINTF("/proc/self/timens_offsets", "%d %d 0",
	                 CLOCK_BOOTTIME, off);

	sysinfo(&si);

	uptime = si.uptime;

	if (!SAFE_FORK()) {
		sysinfo(&si);
		long proc_uptime = read_proc_uptime();

		long diff = si.uptime - uptime;

		if (diff < off || diff > off + 1)
			tst_res(TFAIL, "Wrong sysinfo uptime offset %li", diff);
		else
			tst_res(TPASS, "Correct sysinfo uptime offset %i", off);

		if (si.uptime < proc_uptime || si.uptime > proc_uptime + 1) {
			tst_res(TFAIL, "/proc/uptime %li differs from sysinfo %li",
			        proc_uptime, si.uptime);
		} else {
			tst_res(TPASS, "/proc/uptime is consistent with sysinfo");
		}
	}
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(offsets),
	.test = verify_sysinfo,
	.needs_root = 1,
	.forks_child = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_TIME_NS=y",
		NULL
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "ecc421e05bab"},
		{}
	}
};
