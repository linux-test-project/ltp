// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test verifies that listmount() is properly recognizing a mounted
 * root directory using LSMT_ROOT flag.
 *
 * [Algorithm]
 *
 * - move into a new unshared namespace
 * - mount() a root inside temporary folder and get its mount ID
 * - get list of mounted IDs using listmount(LSMT_ROOT, ..)
 * - verify that the root mount ID is the only mount ID present inside the list
 */

#define _GNU_SOURCE

#include "listmount.h"
#include "lapi/stat.h"
#include "lapi/sched.h"

#define MNTPOINT "mntpoint"
#define LISTSIZE 32

static uint64_t root_id;

static void run(void)
{
	uint64_t list[LISTSIZE];

	TST_EXP_POSITIVE(listmount(LSMT_ROOT, 0, list, LISTSIZE, 0));
	if (!TST_PASS)
		return;

	TST_EXP_EQ_LI(TST_RET, 1);
	TST_EXP_EQ_LI(list[0], root_id);
}

static void setup(void)
{
	struct ltp_statx sx;

	SAFE_UNSHARE(CLONE_NEWNS);

	SAFE_CHROOT(MNTPOINT);
	SAFE_MOUNT("", "/", NULL, MS_REC | MS_PRIVATE, NULL);
	SAFE_STATX(AT_FDCWD, "/", 0, STATX_MNT_ID_UNIQUE, &sx);

	root_id = sx.data.stx_mnt_id;
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.forks_child = 1,
	.min_kver = "6.8",
	.mount_device = 1,
	.mntpoint = MNTPOINT,
};
