// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) 2021 SUSE LLC */

#include <stdio.h>

#include "tst_test.h"

static char *only_mount_v1;
static char *no_cleanup;
static struct tst_option opts[] = {
	{"v", &only_mount_v1, "-v\tOnly try to mount CGroups V1"},
	{"n", &no_cleanup, "-n\tLeave CGroups created by test"},
	{NULL, NULL, NULL},
};
struct tst_cgroup_opts cgopts;

static void do_test(void)
{
	tst_res(TPASS, "pass");
}

static void setup(void)
{
	cgopts.needs_ver = !!only_mount_v1 ? TST_CGROUP_V1 : 0;

	tst_cgroup_scan();
	tst_cgroup_print_config();

	tst_cgroup_require("memory", &cgopts);
	tst_cgroup_print_config();
	tst_cgroup_require("cpuset", &cgopts);
	tst_cgroup_print_config();
}

static void cleanup(void)
{
	if (no_cleanup) {
		tst_res(TINFO, "no cleanup");
	} else {
		tst_res(TINFO, "cleanup");
		tst_cgroup_cleanup();
	}
}

static struct tst_test test = {
	.test_all = do_test,
	.setup = setup,
	.cleanup = cleanup,
	.options = opts,
};
