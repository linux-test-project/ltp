// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2018 MediaTek Inc.  All Rights Reserved.
 * Author: Eddie Horng <eddie.horng@mediatek.com>
 *
 * Check if an unlinked executable can run in overlayfs mount.
 *
 * The regression is introduced from 8db6c34f1dbc ("Introduce v3
 * namespaced file capabilities"). in security/commoncap.c,
 * cap_inode_getsecurity() use d_find_alias() cause unhashed dentry
 * can't be found. The solution could use d_find_any_alias() instead
 * of d_find_alias().
 *
 * Starting with kernel 4.14, this case fails, execveat shall returns EINVAL.
 *
 * This has been fixed by:
 * 355139a8dba4 ("cap_inode_getsecurity: use d_find_any_alias()
 * instead of d_find_alias()")
 */

#define _GNU_SOURCE
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/mount.h>
#include <fcntl.h>
#include "tst_test.h"
#include "lapi/execveat.h"
#include "lapi/fcntl.h"
#include "execveat.h"

#define TEST_APP "execveat_child"
#define TEST_FILE_PATH OVL_MNT"/"TEST_APP

static const char mntpoint[] = OVL_BASE_MNTPOINT;

static void do_child(void)
{
	char *argv[2] = {TEST_FILE_PATH, NULL};
	int fd;

	SAFE_CP(TEST_APP, TEST_FILE_PATH);

	fd = SAFE_OPEN(TEST_FILE_PATH, O_PATH);
	SAFE_UNLINK(TEST_FILE_PATH);

	TEST(execveat(fd, "", argv, environ, AT_EMPTY_PATH));
	tst_res(TFAIL | TERRNO, "execveat() returned unexpected errno");
}

static void verify_execveat(void)
{
	pid_t pid;

	pid = SAFE_FORK();
	if (pid == 0)
		do_child();
}

static void setup(void)
{
	check_execveat();
}

static const char *const resource_files[] = {
	TEST_APP,
	NULL,
};

static struct tst_test test = {
	.needs_root = 1,
	.mount_device = 1,
	.needs_overlay = 1,
	.mntpoint = mntpoint,
	.forks_child = 1,
	.child_needs_reinit = 1,
	.setup = setup,
	.test_all = verify_execveat,
	.resource_files = resource_files,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "8db6c34f1dbc"},
		{"linux-git", "355139a8dba4"},
		{}
	}
};
