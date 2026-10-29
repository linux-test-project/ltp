// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2002
 *
 * HISTORY
 *	06/2002 Written by Paul Larson
 */

/*\
 * Test mlock with various valid addresses and lengths.
 */

#include <stdlib.h>
#include "tst_test.h"

static void *addr;

static struct tcase {
	char *msg;
	int len;
} tcases[] = {
	{"mlock 1 byte", 1},
	{"mlock 1024 bytes", 1024},
	{"mlock 1024 * 1024 bytes", 1024 * 1024},
	{"mlock 1024 * 1024 * 10 bytes", 1024 * 1024 * 10}
};

static void do_mlock(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	tst_res(TINFO, "%s", tc->msg);
	addr = SAFE_MALLOC(tc->len);
	TST_EXP_PASS(mlock(addr, tc->len), "mlock(%p, %d)", addr, tc->len);
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
	.test = do_mlock,
	.tcnt = ARRAY_SIZE(tcases),
	.cleanup = cleanup,
};
