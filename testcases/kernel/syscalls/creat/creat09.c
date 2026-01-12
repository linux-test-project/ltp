// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 SUSE LLC <mdoucha@suse.cz>
 */
/*\
 * CVE-2018-13405
 *
 * Check for possible privilege escalation through creating files with setgid
 * bit set inside a setgid directory owned by a group which the user does not
 * belong to.
 *
 * Fixed in:
 *
 * - 0fa3ecd87848 ("Fix up non-directory creation in SGID directories") # v4.18
 * - 01ea173e103e ("xfs: fix up non-directory creation in SGID directories") # v5.12
 *
 * When use acl or umask, it still has bug. Fixed in:
 *
 * - 1639a49ccdce ("fs: move S_ISGID stripping into the vfs_*() helpers") # v6.0
 */

#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include "tst_test.h"
#include "tst_uid.h"

#define MODE_RWX        0777
#define MODE_SGID       (S_ISGID|0777)

#define MNTPOINT	"mntpoint"
#define WORKDIR		MNTPOINT "/testdir"
#define CREAT_FILE	WORKDIR "/creat.tmp"
#define OPEN_FILE	WORKDIR "/open.tmp"

static gid_t free_gid;
static int fd = -1;

static struct tcase {
	const char *msg;
	int mask;
} tcases[] = {
	{"umask(0)", 0},
	{"umask(S_IXGRP)", S_IXGRP}
};

static void setup(void)
{
	struct stat buf;
	struct passwd *ltpuser = SAFE_GETPWNAM("nobody");

	tst_res(TINFO, "User nobody: uid = %d, gid = %d", (int)ltpuser->pw_uid,
		(int)ltpuser->pw_gid);
	free_gid = tst_get_free_gid(ltpuser->pw_gid);

	/* Create directories and set permissions */
	SAFE_MKDIR(WORKDIR, MODE_RWX);
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

static void file_test(const char *name)
{
	struct stat buf;

	SAFE_STAT(name, &buf);

	if (buf.st_gid != free_gid) {
		tst_res(TFAIL, "%s: Incorrect group, %u != %u", name,
			buf.st_gid, free_gid);
	} else {
		tst_res(TPASS, "%s: Owned by correct group", name);
	}

	if (buf.st_mode & S_ISGID)
		tst_res(TFAIL, "%s: Setgid bit is set", name);
	else
		tst_res(TPASS, "%s: Setgid bit not set", name);
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	umask(tc->mask);
	tst_res(TINFO, "File created with %s", tc->msg);

	fd = SAFE_CREAT(CREAT_FILE, MODE_SGID);
	SAFE_CLOSE(fd);
	file_test(CREAT_FILE);

	fd = SAFE_OPEN(OPEN_FILE, O_CREAT | O_EXCL | O_RDWR, MODE_SGID);
	file_test(OPEN_FILE);
	SAFE_CLOSE(fd);

	/* Cleanup between loops */
	tst_purge_dir(WORKDIR);
}

static void cleanup(void)
{
	if (fd >= 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.timeout = 1,
	.test = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.all_filesystems = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.tcnt = ARRAY_SIZE(tcases),
	.skip_filesystems = (const char*[]) {
		"exfat",
		"ntfs",
		"vfat",
		NULL
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "0fa3ecd87848"},
		{"CVE", "2018-13405"},
		{"CVE", "2021-4037"},
		{"linux-git", "01ea173e103e"},
		{"linux-git", "1639a49ccdce"},
		{"linux-git", "426b4ca2d6a5"},
		{}
	},
};
