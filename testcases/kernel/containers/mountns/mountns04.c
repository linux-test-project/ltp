// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2014 Red Hat, Inc.
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Tests an unbindable mount: unbindable mount is an unbindable
 * private mount.
 *
 * - Creates directories "A", "B" and files "A/A", "B/B"
 * - Unshares mount namespace and makes it private (so mounts/umounts have no
 *   effect on a real system)
 * - Bind mounts directory "A" to "A"
 * - Makes directory "A" unbindable
 * - Check if bind mount unbindable "A" to "B" fails as expected
 */

#include <sys/wait.h>
#include <sys/mount.h>
#include "mountns.h"
#include "tst_test.h"

static void run(void)
{
	SAFE_UNSHARE(CLONE_NEWNS);

	/* makes sure mounts/umounts have no effect on a real system */
	SAFE_MOUNT("none", "/", "none", MS_REC | MS_PRIVATE, NULL);

	SAFE_MOUNT(DIRA, DIRA, "none", MS_BIND, NULL);

	SAFE_MOUNT("none", DIRA, "none", MS_UNBINDABLE, NULL);

	if (mount(DIRA, DIRB, "none", MS_BIND, NULL) == -1) {
		tst_res(TPASS, "unbindable mount passed");
	} else {
		SAFE_UMOUNT(DIRB);
		tst_res(TFAIL, "unbindable mount faled");
	}

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
