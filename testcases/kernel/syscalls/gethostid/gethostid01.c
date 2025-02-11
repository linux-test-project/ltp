// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2021 Xie Ziyao <xieziyao@huawei.com>
 *
 * AUTHOR: William Roske
 * CO-PILOT: Dave Fenner
 *
 *   12/2002 Paul Larson: Add functional test to compare output from hostid
 *   command and gethostid().
 *
 *   01/2003 Robbie Williamson: Add code to handle distros that add "0x" to
 *   beginning of `hostid` output.
 *
 *   01/2006  Marty Ridgeway: Correct 64 bit check so the second 64 bit check
 *   doesn't clobber the first 64 bit check.
 *
 *   07/2021 Xie Ziyao: Rewrite with newlib and use/test sethostid.
 */

/*\
 * Test the basic functionality of the sethostid() and gethostid() system call.
 */

#include "tst_test.h"
#include "config.h"

#ifdef HAVE_SETHOSTID

static long origin;
static long tc[] = {0x00000000, 0x0000ffff};

static void run(unsigned int i)
{
	TST_EXP_PASS(sethostid(tc[i]), "set hostid to %ld", tc[i]);
	TEST(gethostid());

	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO, "gethostid failed");

	if (tc[i] == TST_RET)
		tst_res(TPASS, "hostid is %ld, expected %ld", TST_RET, tc[i]);
	else
		tst_res(TFAIL, "hostid is %ld, expected %ld", TST_RET, tc[i]);
}

static void setup(void)
{
	TEST(gethostid());
	if (TST_RET == -1)
		tst_brk(TFAIL | TTERRNO, "gethostid failed");

	tst_res(TINFO, "get original hostid: %ld", origin = TST_RET);
}

static void cleanup(void)
{
	TST_EXP_PASS(sethostid(origin), "set hostid to %ld", origin);
}

static struct tst_test test = {
	.test = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.tcnt = ARRAY_SIZE(tc),
};

#else
TST_TEST_TCONF("sethostid is undefined.");
#endif
