// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *  AUTHOR : William Roske, CO-PILOT : Dave Fenner
 * Copyright (c) 2023 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that sbrk() successfully increments or decrements the program's
 * data break.
 */

#include "tst_test.h"

static struct tcase {
	long increment;
} tcases[] = {
	{0},
	{8192},
	{-8192}
};

static void run(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TESTPTR(sbrk(tc->increment));

	if (TST_RET_PTR == (void *) -1)
		tst_res(TFAIL | TTERRNO, "sbrk(%ld) failed", tc->increment);
	else
		tst_res(TPASS, "sbrk(%ld) returned %p", tc->increment, TST_RET_PTR);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases)
};
