// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018, Linux Test Project
 */

#include <stdlib.h>
#include <unistd.h>
#include "tst_test.h"
#include "tst_sys_conf.h"

static void setup(void)
{
	SAFE_FILE_PRINTF(PATH_KERN_CORE_PATTERN, "changed");
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
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/nonexistent", NULL, TST_SR_SKIP},
		{PATH_KERN_NUMA_BALANCING, NULL, TST_SR_TBROK},
		{PATH_KERN_CORE_PATTERN, NULL, TST_SR_TCONF},
		{}
	},
};
