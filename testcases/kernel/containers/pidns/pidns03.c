// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2014 Red Hat, Inc. All rights reserved.
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Clone a process with CLONE_NEWPID flag and check if procfs mounted folder
 * belongs to the new pid namespace by looking at /proc/self .
 */

#include <sys/mount.h>
#include "tst_test.h"
#include "lapi/sched.h"

#define PROCDIR "proc"

static void child_func(void)
{
	char proc_self[10];

	SAFE_MOUNT("none", PROCDIR, "proc", MS_RDONLY, NULL);

	SAFE_READLINK(PROCDIR"/self", proc_self, sizeof(proc_self) - 1);

	SAFE_UMOUNT(PROCDIR);

	TST_EXP_PASS(strcmp(proc_self, "1"), PROCDIR"/self contains 1:");
}

static void setup(void)
{
	SAFE_MKDIR(PROCDIR, 0555);
}

static void cleanup(void)
{
	if (tst_is_mounted_at_tmpdir(PROCDIR))
		SAFE_UMOUNT(PROCDIR);
}

static void run(void)
{
	const struct tst_clone_args args = {
		.flags = CLONE_NEWPID,
		.exit_signal = SIGCHLD,
	};

	if (!SAFE_CLONE(&args)) {
		child_func();
		return;
	}
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.forks_child = 1,
	.needs_tmpdir = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_PID_NS",
		NULL,
	},
};
