/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2021 SUSE LLC <mdoucha@suse.cz>
 *
 * Common definitions for perf_event_open tests
 */

#ifndef PERF_EVENT_OPEN_H
#define PERF_EVENT_OPEN_H

#include <linux/types.h>
#include <linux/perf_event.h>
#include <inttypes.h>

static int perf_event_open(struct perf_event_attr *event, pid_t pid,
	int cpu, int group_fd, unsigned long flags)
{
	int ret;

	ret = tst_syscall(__NR_perf_event_open, event, pid, cpu,
		group_fd, flags);

	if (ret != -1)
		return ret;

	tst_res(TINFO, "%s event.type: %"PRIu32
		", event.config: %"PRIu64, __func__, (uint32_t)event->type,
		(uint64_t)event->config);
	if (errno == ENOENT || errno == ENODEV) {
		tst_brk(TCONF | TERRNO, "%s type/config not supported",
			__func__);
	}
	tst_brk(TBROK | TERRNO, "%s failed", __func__);

	/* unreachable */
	return -1;
}

#endif /* PERF_EVENT_OPEN_H */
