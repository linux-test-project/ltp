// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verify that :manpage:`sysinfo(2)` succeeds to get the system information and
 * fills the structure passed. We do sanity checks on the returned values,
 * either comparing it againts values from /proc/ files or by checking that the
 * values are in sane e.g. free RAM <= total RAM.
 */

#include <stdlib.h>
#include <math.h>
#include <sys/sysinfo.h>
#include "tst_test.h"

static struct sysinfo *sys_buf;

static void run(void)
{
	long uptime;
	float load1, load5, load15;
	float sys_load1, sys_load5, sys_load15;
	unsigned long totalswap, totalswap_kb;
	unsigned long totalram, totalram_kb;

	TST_EXP_PASS(sysinfo(sys_buf));

	if (!TST_PASS)
		return;

	SAFE_FILE_SCANF("/proc/uptime", "%ld", &uptime);
	SAFE_FILE_SCANF("/proc/loadavg", "%f %f %f", &load1, &load5, &load15);
	totalram = SAFE_READ_MEMINFO("MemTotal:");
	totalswap = SAFE_READ_MEMINFO("SwapTotal:");

	if (sys_buf->uptime < uptime || sys_buf->uptime - uptime > 2) {
		tst_res(TFAIL, "uptime: %ld, expected between %ld and %ld",
			sys_buf->uptime, uptime, uptime + 2);
	} else {
		tst_res(TPASS, "uptime: %ld (>= %ld)", sys_buf->uptime, uptime);
	}

	sys_load1 = (float)sys_buf->loads[0] / (1 << SI_LOAD_SHIFT);
	sys_load5 = (float)sys_buf->loads[1] / (1 << SI_LOAD_SHIFT);
	sys_load15 = (float)sys_buf->loads[2] / (1 << SI_LOAD_SHIFT);

	/* Compare loads with tolerance */
	if (fabs(sys_load1 - load1) > 0.1 || fabs(sys_load5 - load5) > 0.1 || fabs(sys_load15 - load15) > 0.1) {
		tst_res(TFAIL, "loadavg: %.2f %.2f %.2f, expected ~%.2f %.2f %.2f",
			sys_load1, sys_load5, sys_load15, load1, load5, load15);
	} else {
		tst_res(TPASS, "loadavg: %.2f %.2f %.2f", sys_load1, sys_load5, sys_load15);
	}

	totalram_kb = ((unsigned long long)sys_buf->totalram * sys_buf->mem_unit) / TST_KB;
	totalswap_kb = ((unsigned long long)sys_buf->totalswap * sys_buf->mem_unit) / TST_KB;

	TST_EXP_EQ_LU(totalram_kb, totalram);
	TST_EXP_EQ_LU(totalswap_kb, totalswap);

	TST_EXP_LE_LU(sys_buf->freeram, sys_buf->totalram);
	TST_EXP_LE_LU(sys_buf->sharedram, sys_buf->totalram);
	TST_EXP_LE_LU(sys_buf->bufferram, sys_buf->totalram);
	TST_EXP_LE_LU(sys_buf->freeswap, sys_buf->totalswap);
}

static struct tst_test test = {
	.test_all = run,
	.bufs = (struct tst_buffers[]) {
		{&sys_buf, .size = sizeof(*sys_buf)},
		{}
	}
};
