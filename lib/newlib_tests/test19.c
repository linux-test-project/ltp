// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018, Linux Test Project
 */

#include <stdlib.h>
#include <unistd.h>
#include "tst_test.h"
#include "tst_sys_conf.h"

static const char * const save_restore[] = {
	"?/proc/nonexistent",
	"!/proc/sys/kernel/numa_balancing",
	"/proc/sys/kernel/core_pattern",
	NULL,
};

static void setup(void)
{
	SAFE_FILE_PRINTF("/proc/sys/kernel/core_pattern", "changed");
	tst_sys_conf_dump();
}

static void run(void)
{
	tst_res(TPASS, "OK");
}

static struct tst_test test = {
	.needs_root = 1,
	.test_all = run,
	.setup = setup,
	.save_restore = save_restore,
};
