// SPDX-License-Identifier: GPL-2.0-only
/*\
 *
 * [Description]
 *
 * Conversion of the first kself test in cgroup/test_memcontrol.c.
 * This test creates two nested cgroups with and without enabling the
 * memory controller.
 *
 * The LTP API automatically adds controllers to subtree_control when
 * a child cgroup is added. So unlike the kselftest we remove the
 * controller after it being added automatically.
 */
#define _GNU_SOURCE

#include <stdio.h>

#include "tst_test.h"

static struct tst_cg_group *parent, *child;
static struct tst_cg_group *parent2, *child2;

static void test_memcg_subtree_control(void)
{
	parent = tst_cg_group_mk(tst_cg, "memcg_test_0");
	child = tst_cg_group_mk(parent, "memcg_test_1");
	parent2 = tst_cg_group_mk(tst_cg, "memcg_test_2");
	child2 = tst_cg_group_mk(parent2, "memcg_test_3");

	SAFE_CG_PRINT(parent2, "cgroup.subtree_control", "-memory");

	TST_EXP_POSITIVE(
		SAFE_CG_OCCURSIN(child, "cgroup.controllers", "memory"),
		"child should have memory controller");
	TST_EXP_POSITIVE(
		!SAFE_CG_OCCURSIN(child2, "cgroup.controllers", "memory"),
		"child2 should not have memory controller");

	child2 = tst_cg_group_rm(child2);
	parent2 = tst_cg_group_rm(parent2);
	child = tst_cg_group_rm(child);
	parent = tst_cg_group_rm(parent);
}

static void cleanup(void)
{
	if (child2)
		child2 = tst_cg_group_rm(child2);
	if (parent2)
		parent2 = tst_cg_group_rm(parent2);
	if (child)
		child = tst_cg_group_rm(child);
	if (parent)
		parent = tst_cg_group_rm(parent);
}

static struct tst_test test = {
	.cleanup = cleanup,
	.test_all = test_memcg_subtree_control,
	.needs_cgroup_ver = TST_CG_V2,
	.needs_cgroup_ctrls = (const char *const []){ "memory", NULL },
};
