// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *     07/2001 Ported by Wayne Boyer
 * Copyright (c) 2019 SUSE LLC <mdoucha@suse.cz>
 */

/*
 * Check that the chdir() syscall returns correct value and error code
 * in various situations when called with root privileges
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>

#include "tst_test.h"

#define MNTPOINT "mntpoint"

#define FILE_NAME "testfile"
#define DIR_NAME "subdir"
#define BLOCKED_NAME "keep_out"
#define LINK_NAME1 "symloop"
#define LINK_NAME2 "symloop2"
#define TESTUSER "nobody"

static char *workdir;
static int skip_symlinks, skip_blocked;
static struct passwd *ltpuser;

static char *file_name;
static char *blocked_name;
static char *dir_name;
static char *cwd_name;
static char *parent_name;
static char *root_name;
static char *missing_name;
static char *link_name;

static struct test_case {
	char **name;
	int root_ret, root_err, nobody_ret, nobody_err;
} testcase_list[] = {
	{&file_name, -1, ENOTDIR, -1, ENOTDIR},
	{&blocked_name, 0, 0, -1, EACCES},
	{&dir_name, 0, 0, 0, 0},
	{&cwd_name, 0, 0, 0, 0},
	{&parent_name, 0, 0, 0, 0},
	{&root_name, 0, 0, 0, 0},
	{&missing_name, -1, ENOENT, -1, ENOENT},
	{&link_name, -1, ELOOP, -1, ELOOP},
};

static void setup(void)
{
	char *cwd;
	int fd;
	struct stat statbuf;

	umask(0);

	cwd = SAFE_GETCWD(NULL, 0);
	workdir = SAFE_MALLOC(strlen(cwd) + strlen(MNTPOINT) + 2);
	sprintf(workdir, "%s/%s", cwd, MNTPOINT);
	free(cwd);
	SAFE_CHDIR(workdir);

	SAFE_MKDIR(DIR_NAME, 0755);
	SAFE_MKDIR(BLOCKED_NAME, 0644);

	/* FAT and NTFS override file and directory permissions */
	SAFE_STAT(BLOCKED_NAME, &statbuf);
	skip_blocked = statbuf.st_mode & 0111;
	skip_symlinks = 0;
	TEST(symlink(LINK_NAME1, LINK_NAME2));

	if (!TST_RET)
		SAFE_SYMLINK(LINK_NAME2, LINK_NAME1);
	else if (TST_RET == -1 && (TST_ERR == EPERM || TST_ERR == ENOSYS))
		skip_symlinks = 1;
	else
		tst_brk(TBROK | TTERRNO, "Cannot create symlinks");

	fd = SAFE_CREAT(FILE_NAME, 0644);
	SAFE_CLOSE(fd);

	if (!ltpuser)
		ltpuser = SAFE_GETPWNAM(TESTUSER);
}

static void check_result(const char *user, const char *name, int retval,
	int experr)
{
	if (TST_RET != retval) {
		tst_res(TFAIL | TTERRNO,
			"%s: chdir(\"%s\") returned unexpected value %ld",
			user, name, TST_RET);
		return;
	}

	if (TST_RET != 0 && TST_ERR != experr) {
		tst_res(TFAIL | TTERRNO,
			"%s: chdir(\"%s\") returned unexpected error", user,
			name);
		return;
	}

	tst_res(TPASS | TTERRNO, "%s: chdir(\"%s\") returned correct value",
		user, name);
}

static void run(unsigned int n)
{
	struct test_case *tc = testcase_list + n;

	tst_res(TINFO, "Testing '%s'", *tc->name);

	if (tc->root_err == ELOOP && skip_symlinks) {
		tst_res(TCONF, "Skipping symlink loop test, not supported");
		return;
	}

	/* Reset current directory to mountpoint */
	SAFE_CHDIR(workdir);

	TEST(chdir(*tc->name));
	check_result("root", *tc->name, tc->root_ret, tc->root_err);

	if (tc->nobody_err == EACCES && skip_blocked) {
		tst_res(TCONF, "Skipping unprivileged permission test, "
			"FS mangles dir mode");
		return;
	}

	SAFE_CHDIR(workdir);
	SAFE_SETEUID(ltpuser->pw_uid);
	TEST(chdir(*tc->name));
	SAFE_SETEUID(0);
	check_result(TESTUSER, *tc->name, tc->nobody_ret, tc->nobody_err);
}

static void cleanup(void)
{
	SAFE_CHDIR("..");
	free(workdir);
}

static struct tst_test test = {
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.test = run,
	.tcnt = ARRAY_SIZE(testcase_list),
	.setup = setup,
	.cleanup = cleanup,
	.bufs = (struct tst_buffers []) {
		{&file_name, .str = FILE_NAME},
		{&blocked_name, .str = BLOCKED_NAME},
		{&dir_name, .str = DIR_NAME},
		{&cwd_name, .str = "."},
		{&parent_name, .str = ".."},
		{&root_name, .str = "/"},
		{&missing_name, .str = "does_not_exist"},
		{&link_name, .str = LINK_NAME1},
		{}
	}
};
