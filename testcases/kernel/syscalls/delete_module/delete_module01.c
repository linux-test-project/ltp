// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2018 Xiao Yang <yangx.jy@cn.fujitsu.com>
 * Copyright (c) Linux Test Project, 2002-2023
 * Author: Madhu T L <madhu.tarikere@wipro.com>
 */

/*\
 * Basic test for delete_module(2).
 *
 * Install dummy_del_mod.ko and delete it with delete_module(2).
 */

#include <stdlib.h>
#include "tst_test.h"
#include "tst_module.h"
#include "lapi/syscalls.h"

#define MODULE_NAME	"dummy_del_mod"
#define MODULE_NAME_KO	MODULE_NAME ".ko"

static int module_loaded;

static void do_delete_module(void)
{
	tst_requires_module_signature_disabled();

	if (!module_loaded) {
		tst_module_load(MODULE_NAME_KO, NULL);
		module_loaded = 1;
	}

	TEST(tst_syscall(__NR_delete_module, MODULE_NAME, 0));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO,
			"delete_module() failed to remove module entry for %s",
			MODULE_NAME);
		return;
	}

	tst_res(TPASS, "delete_module() successful");
	module_loaded = 0;
}

static void cleanup(void)
{
	if (module_loaded)
		tst_module_unload(MODULE_NAME_KO);
}

static struct tst_test test = {
	.needs_root = 1,
	/* lockdown and SecureBoot requires signed modules */
	.skip_in_lockdown = 1,
	.skip_in_secureboot = 1,
	.cleanup = cleanup,
	.test_all = do_delete_module,
};
