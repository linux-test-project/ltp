// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * AUTHOR: Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 */

/*\
 * [Description]
 *
 * Test munlock with various valid addresses and lengths.
 */

#include <stdlib.h>
#include "tst_test.h"

static void *addr;

static struct tcase {
	char *msg;
	int len;
} tcases[] = {
	{"munlock 1 byte", 1},
	{"munlock 1024 bytes", 1024},
	{"munlock 1024 * 1024 bytes", 1024 * 1024},
	{"munlock 1024 * 1024 * 10 bytes", 1024 * 1024 * 10}
};

static void verify_munlock(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	tst_res(TINFO, "%s", tc->msg);
	addr = SAFE_MALLOC(tc->len);
	SAFE_MLOCK(addr, tc->len);
	TST_EXP_PASS(munlock(addr, tc->len), "munlock(%p, %d)", addr, tc->len);
	free(addr);
	addr = NULL;
}

static void cleanup(void)
{
	if (addr)
		free(addr);
}

static struct tst_test test = {
	.needs_root = 1,
	.test = verify_munlock,
	.tcnt = ARRAY_SIZE(tcases),
	.cleanup = cleanup,
};
