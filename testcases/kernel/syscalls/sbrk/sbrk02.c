// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 * Copyright (c) 2023 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * Verify that sbrk() on failure sets errno to ENOMEM.
 */


#include "tst_test.h"

#define INC (16*1024*1024)
static long increment = INC;

static void run(void)
{
	TST_EXP_FAIL_PTR_VOID(sbrk(increment), ENOMEM,
		"sbrk(%ld) returned %p", increment, TST_RET_PTR);
}

static void setup(void)
{
	void *ret = NULL;

	while (ret != (void *)-1 && increment > 0) {
		ret = sbrk(increment);
		increment += INC;
	}
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup
};
