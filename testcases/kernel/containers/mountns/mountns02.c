// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2014 Red Hat, Inc.
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Tests a private mount: private mount does not forward or receive
 * propagation.
 *
 * [Algorithm]
 *
 * - Creates directories "A", "B" and files "A/A", "B/B"
 * - Unshares mount namespace and makes it private (so mounts/umounts have no
 *   effect on a real system)
 * - Bind mounts directory "A" to "A"
 * - Makes directory "A" private
 * - Clones a new child process with CLONE_NEWNS flag
 * - There are two test cases (where X is parent namespace and Y child
 *   namespace):
 *  1. First test case
 *   .. X: bind mounts "B" to "A"
 *   .. Y: must see "A/A" and must not see "A/B"
 *   .. X: umounts "A"
 *  2. Second test case
 *   .. Y: bind mounts "B" to "A"
 *   .. X: must see "A/A" and must not see "A/B"
 *   .. Y: umounts A
 */

#include <sys/wait.h>
#include <sys/mount.h>
#include "mountns.h"
#include "tst_test.h"

static int child_func(LTP_ATTRIBUTE_UNUSED void *arg)
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

	return 0;
}

static void run(void)
{
	int ret;

	SAFE_UNSHARE(CLONE_NEWNS);

	/* makes sure parent mounts/umounts have no effect on a real system */
	SAFE_MOUNT("none", "/", "none", MS_REC | MS_PRIVATE, NULL);

	SAFE_MOUNT(DIRA, DIRA, "none", MS_BIND, NULL);

	SAFE_MOUNT("none", DIRA, "none", MS_PRIVATE, NULL);

	ret = ltp_clone_quick(CLONE_NEWNS | SIGCHLD, child_func, NULL);
	if (ret < 0)
		tst_brk(TBROK, "clone failed");

	SAFE_MOUNT(DIRB, DIRA, "none", MS_BIND, NULL);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	SAFE_UMOUNT(DIRA);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	if ((access(DIRA "/A", F_OK) != 0) || (access(DIRA "/B", F_OK) == 0))
		tst_res(TFAIL, "private mount in child failed");
	else
		tst_res(TPASS, "private mount in child passed");

	TST_CHECKPOINT_WAKE(0);

	SAFE_UMOUNT(DIRA);
}

static void setup(void)
{
	check_newns();
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
	.needs_checkpoints = 1,
};
