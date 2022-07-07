// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2014 Red Hat, Inc.
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Tests a slave mount: slave mount is like a shared mount except that
 * mount and umount events only propagate towards it.
 *
 * [Algorithm]
 *
 * - Creates directories DIRA, DIRB and files DIRA/"A", DIRB/"B"
 * - Unshares mount namespace and makes it private (so mounts/umounts have no
 *   effect on a real system)
 * - Bind mounts directory DIRA to itself
 * - Makes directory DIRA shared
 * - Clones a new child process with CLONE_NEWNS flag and makes "A" a slave
 *   mount
 * - There are two testcases (where X is parent namespace and Y child
 *   namespace):
 *  1. First test case
 *   .. X: bind mounts DIRB to DIRA
 *   .. Y: must see the file DIRA/"B"
 *   .. X: umounts DIRA
 *  2. Second test case
 *   .. Y: bind mounts DIRB to DIRA
 *   .. X: must see only the DIRA/"A" and must not see DIRA/"B" (as slave mount does
 *         not forward propagation)
 *   .. Y: umounts DIRA
 */

#include <sys/wait.h>
#include <sys/mount.h>
#include "mountns.h"
#include "tst_test.h"

static int child_func(LTP_ATTRIBUTE_UNUSED void *arg)
{
	/*
	 * makes mount DIRA a slave of DIRA (all slave mounts have
	 * a master mount which is a shared mount)
	 */
	SAFE_MOUNT("none", DIRA, "none", MS_SLAVE, NULL);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	if (access(DIRA "/B", F_OK) == 0)
		tst_res(TPASS, "propagation to slave mount passed");
	else
		tst_res(TFAIL, "propagation to slave mount failed");

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

	SAFE_MOUNT("none", DIRA, "none", MS_SHARED, NULL);

	ret = ltp_clone_quick(CLONE_NEWNS | SIGCHLD, child_func, NULL);
	if (ret < 0)
		tst_brk(TBROK, "clone failed");

	TST_CHECKPOINT_WAIT(0);

	SAFE_MOUNT(DIRB, DIRA, "none", MS_BIND, NULL);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	SAFE_UMOUNT(DIRA);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	if ((access(DIRA "/A", F_OK) == 0) && (access(DIRA "/B", F_OK) == -1))
		tst_res(TPASS, "propagation from slave mount passed");
	else
		tst_res(TFAIL, "propagation form slave mount failed");

	TST_CHECKPOINT_WAKE(0);

	SAFE_WAIT(NULL);

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
