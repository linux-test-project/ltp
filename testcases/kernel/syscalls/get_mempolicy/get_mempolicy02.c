// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
 * Copyright (c) Linux Test Project, 2009-2020
 * Copyright (c) Crackerjack Project, 2007-2008, Hitachi, Ltd
 *
 * Authors:
 * Takahiro Yasui <takahiro.yasui.mp@hitachi.com>
 * Yumiko Sugita <yumiko.sugita.yf@hitachi.com>
 * Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>
 * Manas Kumar Nayak <maknayak@in.ibm.com> (original port to the legacy API)
 */

/*\
 * Verify that get_mempolicy() returns a proper return errno for failure cases.
 */

#include "config.h"
#include "tst_test.h"

#ifdef HAVE_NUMA_V2
#include <numa.h>
#include <numaif.h>
#include <errno.h>
#include "tse_numa.h"

#define PAGES_ALLOCATED 16u

#define POLICY_DESC_TEXT(x, y) .policy = x, .desc = "policy: "#x", "y

static struct tse_nodemap *node;
static struct bitmask *nodemask;

struct test_case {
	int policy;
	const char *desc;
	unsigned int flags;
	int err;
	char *addr;
};

static struct test_case tcase[] = {
	{
		POLICY_DESC_TEXT(MPOL_DEFAULT, "invalid address"),
		.addr = NULL,
		.err = EFAULT,
		.flags = MPOL_F_ADDR,
	},
	{
		POLICY_DESC_TEXT(MPOL_DEFAULT, "invalid flags, no target"),
		.err = EINVAL,
		.flags = -1,
	},
};

static void setup(void)
{
	node = tse_get_nodemap(TST_NUMA_MEM, PAGES_ALLOCATED * getpagesize() / 1024);
	if (node->cnt < 1)
		tst_brk(TCONF, "test requires at least one NUMA memory node");

	nodemask = numa_allocate_nodemask();
}

static void cleanup(void)
{
	numa_free_nodemask(nodemask);
	tse_nodemap_free(node);
}

static void do_test(unsigned int i)
{
	struct test_case *tc = &tcase[i];
	int policy;

	TST_EXP_FAIL(get_mempolicy(&policy, nodemask->maskp, nodemask->size,
				   tc->addr, tc->flags), tc->err, "%s", tc->desc);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcase),
	.test = do_test,
	.setup = setup,
	.cleanup = cleanup,
};

#else
TST_TEST_TCONF(NUMA_ERROR_MSG);
#endif /* HAVE_NUMA_V2 */
