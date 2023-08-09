// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 * Copyright (c) 2023 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that sbrk() on failure sets errno to ENOMEM.
 */


#include "tst_test.h"

#define INC (16*1024*1024)
static long increment = INC;

static void run(void)
{
	TESTPTR(sbrk(increment));

	if (TST_RET_PTR != (void *)-1) {
		tst_res(TFAIL, "sbrk(%ld) unexpectedly passed and returned %p, "
						"expected (void *)-1 with errno=%d",
						increment, TST_RET_PTR, ENOMEM);
		return;
	}

	if (TST_ERR == ENOMEM)
		tst_res(TPASS | TTERRNO, "sbrk(%ld) failed as expected", increment);
	else
		tst_res(TFAIL | TTERRNO, "sbrk(%ld) failed but unexpected errno, "
								"expected errno=%d - %s",
								increment, ENOMEM, strerror(ENOMEM));
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
