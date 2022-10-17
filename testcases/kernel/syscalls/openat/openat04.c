// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * [Description]
 *
 * Check setgid strip logic whether works correctly when creating tmpfile under
 * filesystem without POSIX ACL supported(by using noacl mount option). Test it
 * with umask S_IXGRP and also check file mode whether has filtered S_IXGRP.
 *
 * Fixed in:
 *
 *  commit ac6800e279a22b28f4fc21439843025a0d5bf03e
 *  Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 *  Date:   Thu July 14 14:11:26 2022 +0800
 *
 *  fs: Add missing umask strip in vfs_tmpfile
 *
 * The most code is pasted form creat09.c.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "tst_test.h"
#include "tst_uid.h"
#include "tst_safe_file_at.h"

#define MODE_RWX        0777
#define MODE_SGID       (S_ISGID|0777)
#define MNTPOINT	"mntpoint"
#define WORKDIR		MNTPOINT "/testdir"
#define OPEN_FILE	"open.tmp"

static gid_t free_gid;
static int tmpfile_fd = -1, dir_fd = -1, mount_flag;
static struct passwd *ltpuser;

static void do_mount(const char *source, const char *target,
	const char *filesystemtype, unsigned long mountflags,
	const void *data)
{
	TEST(mount(source, target, filesystemtype, mountflags, data));

	if (TST_RET == -1 && TST_ERR == EINVAL)
		tst_brk(TCONF, "Kernel does not support noacl feature");

	if (TST_RET == -1) {
		tst_brk(TBROK | TTERRNO, "mount(%s, %s, %s, %lu, %p) failed",
			source, target, filesystemtype, mountflags, data);
	}

	if (TST_RET)
		tst_brk(TBROK, "Invalid mount return value %ld", TST_RET);

	mount_flag = 1;
}

static void open_tmpfile_supported(int dirfd)
{
	TEST(openat(dirfd, ".", O_TMPFILE | O_RDWR, S_IXGRP | S_ISGID));

	if (TST_RET == -1) {
		if (errno == ENOTSUP)
			tst_brk(TCONF, "fs doesn't support O_TMPFILE");
		else
			tst_brk(TBROK | TTERRNO, "openat(%d, O_TMPFILE) failed", dirfd);
	}

	if (TST_RET < 0)
		tst_brk(TBROK, "Invalid openat return value %ld", TST_RET);

	SAFE_CLOSE(TST_RET);
}

static void setup(void)
{
	struct stat buf;

	ltpuser = SAFE_GETPWNAM("nobody");

	do_mount(tst_device->dev, MNTPOINT, tst_device->fs_type, 0, "noacl");

	tst_res(TINFO, "User nobody: uid = %d, gid = %d", (int)ltpuser->pw_uid,
		(int)ltpuser->pw_gid);
	free_gid = tst_get_free_gid(ltpuser->pw_gid);

	/* Create directories and set permissions */
	SAFE_MKDIR(WORKDIR, MODE_RWX);
	dir_fd = SAFE_OPEN(WORKDIR, O_RDONLY, O_DIRECTORY);
	open_tmpfile_supported(dir_fd);

	SAFE_CHOWN(WORKDIR, ltpuser->pw_uid, free_gid);
	SAFE_CHMOD(WORKDIR, MODE_SGID);
	SAFE_STAT(WORKDIR, &buf);

	if (!(buf.st_mode & S_ISGID))
		tst_brk(TBROK, "%s: Setgid bit not set", WORKDIR);

	if (buf.st_gid != free_gid) {
		tst_brk(TBROK, "%s: Incorrect group, %u != %u", WORKDIR,
			buf.st_gid, free_gid);
	}

	/* Switch user */
	SAFE_SETGID(ltpuser->pw_gid);
	SAFE_SETREUID(-1, ltpuser->pw_uid);
}

static void file_test(int dfd, const char *path, int flags)
{
	struct stat buf;

	SAFE_FSTATAT(dfd, path, &buf, flags);

	TST_EXP_EQ_LI(buf.st_gid, free_gid);

	if (buf.st_mode & S_ISGID)
		tst_res(TFAIL, "%s: Setgid bit is set", path);
	else
		tst_res(TPASS, "%s: Setgid bit not set", path);

	if (buf.st_mode & S_IXGRP)
		tst_res(TFAIL, "%s: S_IXGRP bit is set", path);
	else
		tst_res(TPASS, "%s: S_IXGRP bit is not set", path);
}

static void run(void)
{
	char path[PATH_MAX];

	umask(S_IXGRP);
	tmpfile_fd = SAFE_OPENAT(dir_fd, ".", O_TMPFILE | O_RDWR, MODE_SGID);
	snprintf(path, PATH_MAX, "/proc/self/fd/%d", tmpfile_fd);
	SAFE_LINKAT(AT_FDCWD, path, dir_fd, OPEN_FILE, AT_SYMLINK_FOLLOW);
	file_test(dir_fd, OPEN_FILE, 0);
	SAFE_CLOSE(tmpfile_fd);
	/* Cleanup between loops */
	tst_purge_dir(WORKDIR);
}

static void cleanup(void)
{
	SAFE_SETREUID(-1, 0);

	if (tmpfile_fd >= 0)
		SAFE_CLOSE(tmpfile_fd);
	if (dir_fd >= 0)
		SAFE_CLOSE(dir_fd);
	if (mount_flag && tst_umount(MNTPOINT))
		tst_res(TWARN | TERRNO, "umount(%s)", MNTPOINT);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.all_filesystems = 1,
	.format_device = 1,
	.mntpoint = MNTPOINT,
	.skip_filesystems = (const char*[]) {
		"exfat",
		"ntfs",
		"vfat",
		NULL
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "ac6800e279a2"},
		{"linux-git", "426b4ca2d6a5"},
		{}
	},
};
