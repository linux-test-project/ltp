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
 * Verify that get_mempolicy() returns a proper return value and errno for various cases.
 */

#include "config.h"
#include "tst_test.h"

#ifdef HAVE_NUMA_V2
#include <numa.h>
#include <numaif.h>
#include <errno.h>
#include "tst_numa.h"

#define MEM_LENGTH	(4 * 1024 * 1024)
#define PAGES_ALLOCATED 16u

#define POLICY_DESC(x) .policy = x, .desc = "policy: "#x
#define POLICY_DESC_NO_TARGET(x) .policy = x, .desc = "policy: "#x", no target"
#define POLICY_DESC_FLAGS(x, y) .policy = x, .flags = y, .desc = "policy: "#x", flags: "#y
#define POLICY_DESC_FLAGS_NO_TARGET(x, y) .policy = x, .flags = y, .desc = "policy: "#x", flags: "#y", no target"

static struct tst_nodemap *node;
static struct bitmask *nodemask, *getnodemask, *empty_nodemask;

struct test_case {
	int policy;
	const char *desc;
	unsigned int flags;
	char *addr;
	int (*pre_test)(struct test_case *tc);
	int (*alloc)(struct test_case *tc);
	struct bitmask **exp_nodemask;
};

static int test_set_mempolicy_default(struct test_case *tc);
static int test_set_mempolicy_none(struct test_case *tc);
static int test_mbind_none(struct test_case *tc);
static int test_mbind_default(struct test_case *tc);

static struct test_case tcase[] = {
	{
		POLICY_DESC_NO_TARGET(MPOL_DEFAULT),
		.alloc = test_set_mempolicy_none,
		.exp_nodemask = &nodemask,
	},
	{
		POLICY_DESC(MPOL_BIND),
		.alloc = test_set_mempolicy_default,
		.exp_nodemask = &nodemask,
	},
	{
		POLICY_DESC(MPOL_INTERLEAVE),
		.alloc = test_set_mempolicy_default,
		.exp_nodemask = &nodemask,
	},
	{
		POLICY_DESC_NO_TARGET(MPOL_PREFERRED),
		.alloc = test_set_mempolicy_none,
		.exp_nodemask = &nodemask,
	},
	{
		POLICY_DESC(MPOL_PREFERRED),
		.alloc = test_set_mempolicy_default,
		.exp_nodemask = &nodemask,
	},
	{
		POLICY_DESC_FLAGS_NO_TARGET(MPOL_DEFAULT, MPOL_F_ADDR),
		.pre_test = test_mbind_none,
		.alloc = test_set_mempolicy_none,
		.exp_nodemask = &empty_nodemask,
	},
	{
		POLICY_DESC_FLAGS(MPOL_BIND, MPOL_F_ADDR),
		.pre_test = test_mbind_default,
		.exp_nodemask = &nodemask,
	},
	{
		POLICY_DESC_FLAGS(MPOL_INTERLEAVE, MPOL_F_ADDR),
		.pre_test = test_mbind_default,
		.alloc = test_set_mempolicy_default,
		.exp_nodemask = &nodemask,
	},
	{
		POLICY_DESC_FLAGS_NO_TARGET(MPOL_PREFERRED, MPOL_F_ADDR),
		.pre_test = test_mbind_none,
		.alloc = test_set_mempolicy_none,
		.exp_nodemask = &empty_nodemask,
	},
	{
		POLICY_DESC_FLAGS(MPOL_PREFERRED, MPOL_F_ADDR),
		.pre_test = test_mbind_default,
		.alloc = test_set_mempolicy_default,
		.exp_nodemask = &nodemask,
	},
};

static int test_set_mempolicy_default(struct test_case *tc)
{
	TEST(set_mempolicy(tc->policy, nodemask->maskp, nodemask->size));
	return TST_RET;
}

static int test_set_mempolicy_none(struct test_case *tc)
{
	TEST(set_mempolicy(tc->policy, NULL, 0));
	return TST_RET;
}

static int test_mbind(struct test_case *tc, unsigned long *maskp, unsigned long size)
{
	tc->addr = SAFE_MMAP(NULL, MEM_LENGTH, PROT_READ | PROT_WRITE,
			     MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

	TEST(mbind(tc->addr, MEM_LENGTH, tc->policy, maskp, size, 0));
	return TST_RET;
}

static int test_mbind_none(struct test_case *tc)
{
	return test_mbind(tc, NULL, 0);
}

static int test_mbind_default(struct test_case *tc)
{
	return test_mbind(tc, nodemask->maskp, nodemask->size);
}

static void setup(void)
{
	unsigned int i;

	node = tst_get_nodemap(TST_NUMA_MEM, PAGES_ALLOCATED * getpagesize() / 1024);
	if (node->cnt < 1)
		tst_brk(TCONF, "test requires at least one NUMA memory node");

	nodemask = numa_allocate_nodemask();
	empty_nodemask = numa_allocate_nodemask();
	getnodemask = numa_allocate_nodemask();
	numa_bitmask_setbit(nodemask, node->map[0]);

	for (i = 0; i < ARRAY_SIZE(tcase); i++) {
		struct test_case *tc = &tcase[i];

		if (tc->pre_test && tc->pre_test(tc))
			tst_brk(TFAIL | TERRNO, "test #%d: mbind() failed", i+1);

		if (tc->alloc && tc->alloc(tc))
			tst_brk(TFAIL | TERRNO, "test #%d: set_mempolicy() failed", i+1);
	}
}

static void cleanup(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(tcase); i++) {
		struct test_case *tc = &tcase[i];

		if (tc->pre_test)
			SAFE_MUNMAP(tc->addr, MEM_LENGTH);
	}

	numa_free_nodemask(nodemask);
	numa_free_nodemask(getnodemask);
	tst_nodemap_free(node);
}

static void do_test(unsigned int i)
{
	struct test_case *tc = &tcase[i];
	int policy;

	tst_res(TINFO, "test #%d: %s", i+1, tc->desc);

	TST_EXP_PASS(get_mempolicy(&policy, getnodemask->maskp, getnodemask->size,
			   tc->addr, tc->flags), "%s", tc->desc);

		struct bitmask *exp_mask = *(tc->exp_nodemask);

		if (!numa_bitmask_equal(exp_mask, getnodemask)) {
			tst_res(TFAIL, "masks are not equal");
			tst_res_hexd(TINFO, exp_mask->maskp,
				     exp_mask->size / 8, "expected:");
			tst_res_hexd(TINFO, getnodemask->maskp,
				     getnodemask->size / 8, "returned:");
		}
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
