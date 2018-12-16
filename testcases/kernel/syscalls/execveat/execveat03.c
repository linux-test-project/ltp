/*
 * Copyright (C) 2018 MediaTek Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 or any later of the GNU General Public License
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Started by Eddie Horng <eddie.horng@mediatek.com>
 *
 * DESCRIPTION
 *     Check if an unlinked executable can run in overlayfs mount.
 *     The regression is introduced from 8db6c34f1dbc ("Introduce v3
 *     namespaced file capabilities"). in security/commoncap.c,
 *     cap_inode_getsecurity() use d_find_alias() cause unhashed dentry
 *     can't be found. The solution could use d_find_any_alias() instead of
 *     d_find_alias().
 *
 *     Starting with kernel 4.14, this case fails, execveat shall
 *     returns EINVAL.
 *
 *     This has been fixed by:
 *       355139a8dba4 ("cap_inode_getsecurity: use d_find_any_alias()
 *                      instead of d_find_alias()")
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

#define OVL_MNT "ovl"
#define TEST_APP "execveat_child"
#define TEST_FILE_PATH OVL_MNT"/"TEST_APP

static int ovl_mounted;

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
	int ret;

	check_execveat();

	/* Setup an overlay mount with lower file */
	SAFE_MKDIR("lower", 0755);
	SAFE_MKDIR("upper", 0755);
	SAFE_MKDIR("work", 0755);
	SAFE_MKDIR(OVL_MNT, 0755);
	ret = mount("overlay", OVL_MNT, "overlay", 0,
		    "lowerdir=lower,upperdir=upper,workdir=work");
	if (ret < 0) {
		if (errno == ENODEV) {
			tst_brk(TCONF,
				"overlayfs is not configured in this kernel.");
		}
		tst_brk(TBROK | TERRNO, "overlayfs mount failed");
	}
	ovl_mounted = 1;
}

static void cleanup(void)
{
	if (ovl_mounted)
		SAFE_UMOUNT(OVL_MNT);
}

static const char *const resource_files[] = {
	TEST_APP,
	NULL,
};

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.forks_child = 1,
	.child_needs_reinit = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_execveat,
	.resource_files = resource_files,
};
