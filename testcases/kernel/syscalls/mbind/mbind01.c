// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007-2008, Hitachi, Ltd
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 *
 * Authors:
 * Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,
 * Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,
 * Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>
 */

#include <errno.h>
#if HAVE_NUMA_H
#include <numa.h>
#endif

#include "config.h"
#include "numa_helper.h"
#include "tst_test.h"

#ifdef HAVE_NUMA_V2

#define MEM_LENGTH (4 * 1024 * 1024)

#define UNKNOWN_POLICY -1

#define POLICY_DESC(x) .policy = x, .desc = #x
#define POLICY_DESC_TEXT(x, y) .policy = x, .desc = #x" ("y")"

static struct bitmask *nodemask, *getnodemask, *empty_nodemask;

static void test_default(unsigned int i, char *p);
static void test_none(unsigned int i, char *p);
static void test_invalid_nodemask(unsigned int i, char *p);

struct test_case {
	int policy;
	const char *desc;
	unsigned flags;
	int ret;
	int err;
	void (*test)(unsigned int, char *);
	struct bitmask **exp_nodemask;
};

static struct test_case tcase[] = {
	{
		POLICY_DESC(MPOL_DEFAULT),
		.ret = 0,
		.err = 0,
		.test = test_none,
		.exp_nodemask = &empty_nodemask,
	},
	{
		POLICY_DESC_TEXT(MPOL_DEFAULT, "target exists"),
		.ret = -1,
		.err = EINVAL,
		.test = test_default,
	},
	{
		POLICY_DESC_TEXT(MPOL_BIND, "no target"),
		.ret = -1,
		.err = EINVAL,
		.test = test_none,
	},
	{
		POLICY_DESC(MPOL_BIND),
		.ret = 0,
		.err = 0,
		.test = test_default,
		.exp_nodemask = &nodemask,
	},
	{
		POLICY_DESC_TEXT(MPOL_INTERLEAVE, "no target"),
		.ret = -1,
		.err = EINVAL,
		.test = test_none,
	},
	{
		POLICY_DESC(MPOL_INTERLEAVE),
		.ret = 0,
		.err = 0,
		.test = test_default,
		.exp_nodemask = &nodemask,
	},
	{
		POLICY_DESC_TEXT(MPOL_PREFERRED, "no target"),
		.ret = 0,
		.err = 0,
		.test = test_none,
	},
	{
		POLICY_DESC(MPOL_PREFERRED),
		.ret = 0,
		.err = 0,
		.test = test_default,
		.exp_nodemask = &nodemask,
	},
	{
		POLICY_DESC(UNKNOWN_POLICY),
		.ret = -1,
		.err = EINVAL,
		.test = test_none,
	},
	{
		POLICY_DESC_TEXT(MPOL_DEFAULT, "invalid flags"),
		.flags = -1,
		.ret = -1,
		.err = EINVAL,
		.test = test_none,
	},
	{
		POLICY_DESC_TEXT(MPOL_PREFERRED, "invalid nodemask"),
		.ret = -1,
		.err = EFAULT,
		.test = test_invalid_nodemask,
	},
};

static void test_default(unsigned int i, char *p)
{
	struct test_case *tc = &tcase[i];

	TEST(mbind(p, MEM_LENGTH, tc->policy, nodemask->maskp,
		   nodemask->size, tc->flags));
}

static void test_none(unsigned int i, char *p)
{
	struct test_case *tc = &tcase[i];

	TEST(mbind(p, MEM_LENGTH, tc->policy, NULL, 0, tc->flags));
}

static void test_invalid_nodemask(unsigned int i, char *p)
{
	struct test_case *tc = &tcase[i];

	/* use invalid nodemask (64 MiB after heap) */
	TEST(mbind(p, MEM_LENGTH, tc->policy, sbrk(0) + 64*1024*1024,
		   NUMA_NUM_NODES, tc->flags));
}

static void setup(void)
{
	if (!is_numa(NULL, NH_MEMS, 1))
		tst_brk(TCONF, "requires NUMA with at least 1 node");
	empty_nodemask = numa_allocate_nodemask();
}

static void setup_node(void)
{
	int test_node = -1;

	if (get_allowed_nodes(NH_MEMS, 1, &test_node) < 0)
		tst_brk(TBROK | TERRNO, "get_allowed_nodes failed");

	nodemask = numa_allocate_nodemask();
	getnodemask = numa_allocate_nodemask();
	numa_bitmask_setbit(nodemask, test_node);
}

static void do_test(unsigned int i)
{
	struct test_case *tc = &tcase[i];
	int policy, fail = 0;
	char *p = NULL;

	tst_res(TINFO, "case %s", tc->desc);

	setup_node();

	p = mmap(NULL, MEM_LENGTH, PROT_READ | PROT_WRITE, MAP_PRIVATE |
			 MAP_ANONYMOUS, 0, 0);
	if (p == MAP_FAILED)
		tst_brk(TBROK | TERRNO, "mmap");

	tc->test(i, p);

	if (TST_RET >= 0) {
		/* Check policy of the allocated memory */
		TEST(get_mempolicy(&policy, getnodemask->maskp,
				   getnodemask->size, p, MPOL_F_ADDR));
		if (TST_RET < 0) {
			tst_res(TFAIL | TTERRNO, "get_mempolicy failed");
			return;
		}
		if (tc->policy != policy) {
			tst_res(TFAIL, "Wrong policy: %d, expected: %d",
				tc->policy, policy);
			fail = 1;
		}
		if (tc->exp_nodemask) {
			struct bitmask *exp_mask = *(tc->exp_nodemask);

			if (!numa_bitmask_equal(exp_mask, getnodemask)) {
				tst_res(TFAIL, "masks are not equal");
				tst_res_hexd(TINFO, exp_mask->maskp,
					exp_mask->size / 8, "exp_mask: ");
				tst_res_hexd(TINFO, getnodemask->maskp,
					getnodemask->size / 8, "returned: ");
				fail = 1;
			}
		}
	}

	if (TST_RET != tc->ret) {
		tst_res(TFAIL, "wrong return code: %ld, expected: %d",
			TST_RET, tc->ret);
		fail = 1;
	}
	if (TST_RET == -1 && TST_ERR != tc->err) {
		tst_res(TFAIL | TTERRNO, "expected errno: %s, got",
			tst_strerrno(tc->err));
		fail = 1;
	}
	if (!fail)
		tst_res(TPASS, "Test passed");
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcase),
	.test = do_test,
	.setup = setup,
};

#else
	TST_TEST_TCONF(NUMA_ERROR_MSG);
#endif
