// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2018 Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*
 * AUTHOR: Madhu T L <madhu.tarikere@wipro.com>
 *
 * DESCRIPTION:
 * Basic tests for delete_module(2)
 * 1) insmod dummy_del_mod.ko
 * 2) call delete_module(2) to remove dummy_del_mod.ko
 */

#include <errno.h>
#include "tst_test.h"
#include "tst_module.h"
#include "lapi/syscalls.h"

#define MODULE_NAME	"dummy_del_mod"
#define MODULE_NAME_KO	"dummy_del_mod.ko"

static int module_loaded;

static void do_delete_module(void)
{
	if (module_loaded == 0) {
		tst_module_load(MODULE_NAME_KO, NULL);
		module_loaded = 1;
	}

	TEST(tst_syscall(__NR_delete_module, MODULE_NAME, 0));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "delete_module() failed to "
			"remove module entry for %s ", MODULE_NAME);
		return;
	}

	tst_res(TPASS, "delete_module() successful");
	module_loaded = 0;
}

static void cleanup(void)
{
	if (module_loaded == 1)
		tst_module_unload(MODULE_NAME_KO);
}

static struct tst_test test = {
	.needs_root = 1,
	/* lockdown requires signed modules */
	.skip_in_lockdown = 1,
	.cleanup = cleanup,
	.test_all = do_delete_module,
};
