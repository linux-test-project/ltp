// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2014 Red Hat, Inc.
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Tests a private mount: private mount does not forward or receive
 * propagation.
 *
 * [Algorithm]
 *
 * - Creates directories DIR_A, DIR_B and files DIR_A/"A", DIR_B/"B"
 * - Unshares mount namespace and makes it private (so mounts/umounts have no
 *   effect on a real system)
 * - Bind mounts directory DIR_A to DIR_A
 * - Makes directory DIR_A private
 * - Clones a new child process with CLONE_NEWNS flag
 * - There are two test cases (where X is parent namespace and Y child
 *   namespace):
 *  1. First test case
 *   .. X: bind mounts DIR_B to DIR_A
 *   .. Y: must see DIR_A/"A" and must not see DIR_A/"B"
 *   .. X: umounts DIR_A
 *  2. Second test case
 *   .. Y: bind mounts DIR_B to DIR_A
 *   .. X: must see DIR_A/"A" and must not see DIR_A/"B"
 *   .. Y: umounts DIRA
 */

#include <sys/wait.h>
#include <sys/mount.h>
#include "mountns.h"
#include "tst_test.h"
#include "lapi/sched.h"

static void child_func(void)
{
	TST_CHECKPOINT_WAIT(0);

	if ((access(DIRA "/A", F_OK) != 0) || (access(DIRA "/B", F_OK) == 0))
		tst_res(TFAIL, "private mount in parent failed");
	else
		tst_res(TPASS, "private mount in parent passed");

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	SAFE_MOUNT(DIRB, DIRA, "none", MS_BIND, NULL);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	SAFE_UMOUNT(DIRA);
}

static void run(void)
{
	const struct tst_clone_args args = {
		.flags = CLONE_NEWNS,
		.exit_signal = SIGCHLD,
	};

	SAFE_UNSHARE(CLONE_NEWNS);

	/* makes sure parent mounts/umounts have no effect on a real system */
	SAFE_MOUNT("none", "/", "none", MS_REC | MS_PRIVATE, NULL);

	SAFE_MOUNT(DIRA, DIRA, "none", MS_BIND, NULL);

	SAFE_MOUNT("none", DIRA, "none", MS_PRIVATE, NULL);

	if (!SAFE_CLONE(&args)) {
		child_func();
		return;
	}

	SAFE_MOUNT(DIRB, DIRA, "none", MS_BIND, NULL);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	SAFE_UMOUNT(DIRA);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	if ((access(DIRA "/A", F_OK) != 0) || (access(DIRA "/B", F_OK) == 0))
		tst_res(TFAIL, "private mount in child failed");
	else
		tst_res(TPASS, "private mount in child passed");

	TST_CHECKPOINT_WAKE(0);

	SAFE_WAIT(NULL);

	SAFE_UMOUNT(DIRA);
}

static void setup(void)
{
	create_folders();
}

static void cleanup(void)
{
	umount_folders();
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.needs_root = 1,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USER_NS",
		NULL,
	},
};
