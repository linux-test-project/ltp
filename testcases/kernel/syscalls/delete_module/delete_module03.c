// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2018 Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*
 * AUTHOR: Madhu T L <madhu.tarikere@wipro.com>
 *
 * DESCRIPTION
 * Verify that, delete_module(2) returns -1 and sets errno to EWOULDBLOCK,
 * if tried to remove a module while other modules depend on this module.
 */

#include <stdlib.h>
#include <errno.h>
#include "tst_test.h"
#include "tst_module.h"
#include "lapi/syscalls.h"

#define DUMMY_MOD		"dummy_del_mod"
#define DUMMY_MOD_KO		"dummy_del_mod.ko"
#define DUMMY_MOD_DEP_KO	"dummy_del_mod_dep.ko"

static int dummy_mod_loaded;
static int dummy_mod_dep_loaded;

static void do_delete_module(void)
{
	TEST(tst_syscall(__NR_delete_module, DUMMY_MOD, 0));
	if (TST_RET < 0) {
		if (TST_ERR == EWOULDBLOCK) {
			tst_res(TPASS | TTERRNO,
				"delete_module() failed as expected");
		} else {
			tst_res(TFAIL | TTERRNO, "delete_module() failed "
			"unexpectedly; expected: %s",
			tst_strerrno(EWOULDBLOCK));
		}
	} else {
		tst_res(TFAIL, "delete_module() succeeded unexpectedly");
		dummy_mod_loaded = 0;
		/*
		 * insmod DUMMY_MOD_KO again in case running
		 * with -i option
		 */
		tst_module_load(DUMMY_MOD_KO, NULL);
		dummy_mod_loaded = 1;
	}
}

static void setup(void)
{
	tst_requires_module_signature_disabled();

	/* Load first kernel module */
	tst_module_load(DUMMY_MOD_KO, NULL);
	dummy_mod_loaded = 1;

	/* Load dependant kernel module */
	tst_module_load(DUMMY_MOD_DEP_KO, NULL);
	dummy_mod_dep_loaded = 1;
}

static void cleanup(void)
{
	/* Unload dependent kernel module */
	if (dummy_mod_dep_loaded == 1)
		tst_module_unload(DUMMY_MOD_DEP_KO);

	/* Unload first kernel module */
	if (dummy_mod_loaded == 1)
		tst_module_unload(DUMMY_MOD_KO);
}

static struct tst_test test = {
	.needs_root = 1,
	/* lockdown and SecureBoot requires signed modules */
	.skip_in_lockdown = 1,
	.skip_in_secureboot = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = do_delete_module,
};
