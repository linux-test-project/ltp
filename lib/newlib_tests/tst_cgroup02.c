// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) 2021 SUSE LLC */

#include <stdlib.h>
#include <stdio.h>

#include "tst_test.h"
#include "tst_cgroup.h"

static char *only_mount_v1;
static char *no_cleanup;
static struct tst_option opts[] = {
	{"v", &only_mount_v1, "-v\tOnly try to mount CGroups V1"},
	{"n", &no_cleanup, "-n\tLeave CGroups created by test"},
	{NULL, NULL, NULL},
};
static struct tst_cgroup_opts cgopts;
static const struct tst_cgroup_group *cg;
static const struct tst_cgroup_group *cg_drain;
static struct tst_cgroup_group *cg_child;

static void do_test(void)
{
	char buf[BUFSIZ];
	size_t mem;

	if (!TST_CGROUP_VER_IS_V1(cg, "memory"))
		SAFE_CGROUP_PRINT(cg, "cgroup.subtree_control", "+memory");
	if (!TST_CGROUP_VER_IS_V1(cg, "cpuset"))
		SAFE_CGROUP_PRINT(cg, "cgroup.subtree_control", "+cpuset");

	cg_child = tst_cgroup_group_mk(cg, "child");
	if (!SAFE_FORK()) {
		SAFE_CGROUP_PRINTF(cg_child, "cgroup.procs", "%d", getpid());

		SAFE_CGROUP_SCANF(cg_child, "memory.current", "%zu", &mem);
		tst_res(TPASS, "child/memory.current = %zu", mem);
		SAFE_CGROUP_PRINTF(cg_child, "memory.max",
				   "%zu", (1UL << 24) - 1);
		SAFE_CGROUP_PRINTF(cg_child, "memory.swap.max",
				   "%zu", 1UL << 31);

		SAFE_CGROUP_READ(cg_child, "cpuset.mems", buf, sizeof(buf));
		tst_res(TPASS, "child/cpuset.mems = %s", buf);
		SAFE_CGROUP_PRINT(cg_child, "cpuset.mems", buf);

		exit(0);
	}

	SAFE_CGROUP_PRINTF(cg, "memory.max", "%zu", (1UL << 24) - 1);
	SAFE_CGROUP_PRINTF(cg_child, "cgroup.procs", "%d", getpid());
	SAFE_CGROUP_SCANF(cg, "memory.current", "%zu", &mem);
	tst_res(TPASS, "memory.current = %zu", mem);

	tst_reap_children();
	SAFE_CGROUP_PRINTF(cg_drain, "cgroup.procs", "%d", getpid());
	cg_child = tst_cgroup_group_rm(cg_child);
}

static void setup(void)
{
	cgopts.only_mount_v1 = !!only_mount_v1,

	tst_cgroup_scan();
	tst_cgroup_print_config();

	tst_cgroup_require("memory", &cgopts);
	tst_cgroup_require("cpuset", &cgopts);

	cg = tst_cgroup_get_test_group();
	cg_drain = tst_cgroup_get_drain_group();
}

static void cleanup(void)
{
	if (cg_child) {
		SAFE_CGROUP_PRINTF(cg_drain, "cgroup.procs", "%d", getpid());
		cg_child = tst_cgroup_group_rm(cg_child);
	}
	if (!no_cleanup)
		tst_cgroup_cleanup();
}

static struct tst_test test = {
	.test_all = do_test,
	.setup = setup,
	.cleanup = cleanup,
	.options = opts,
	.forks_child = 1,
};
