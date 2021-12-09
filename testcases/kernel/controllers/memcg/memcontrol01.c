// SPDX-License-Identifier: GPL-2.0
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
#include "tst_cgroup.h"

static const struct tst_cgroup_group *cg_test;
static struct tst_cgroup_group *parent, *child;
static struct tst_cgroup_group *parent2, *child2;

static void test_memcg_subtree_control(void)
{
	parent = tst_cgroup_group_mk(cg_test, "memcg_test_0");
	child = tst_cgroup_group_mk(parent, "memcg_test_1");
	parent2 = tst_cgroup_group_mk(cg_test, "memcg_test_2");
	child2 = tst_cgroup_group_mk(parent2, "memcg_test_3");

	SAFE_CGROUP_PRINT(parent2, "cgroup.subtree_control", "-memory");

	TST_EXP_POSITIVE(
		SAFE_CGROUP_OCCURSIN(child, "cgroup.controllers", "memory"),
		"child should have memory controller");
	TST_EXP_POSITIVE(
		!SAFE_CGROUP_OCCURSIN(child2, "cgroup.controllers", "memory"),
		"child2 should not have memory controller");

	child2 = tst_cgroup_group_rm(child2);
	parent2 = tst_cgroup_group_rm(parent2);
	child = tst_cgroup_group_rm(child);
	parent = tst_cgroup_group_rm(parent);
}

static void setup(void)
{
	tst_cgroup_require("memory", NULL);
	cg_test = tst_cgroup_get_test_group();

	if (TST_CGROUP_VER(cg_test, "memory") == TST_CGROUP_V1)
		tst_brk(TCONF, "V1 controllers do not have subtree control");
}

static void cleanup(void)
{
	if (child2)
		child2 = tst_cgroup_group_rm(child2);
	if (parent2)
		parent2 = tst_cgroup_group_rm(parent2);
	if (child)
		child = tst_cgroup_group_rm(child);
	if (parent)
		parent = tst_cgroup_group_rm(parent);

	tst_cgroup_cleanup();
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = test_memcg_subtree_control,
};
