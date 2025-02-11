// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */

/*\
 * Verify that,
 *
 * - pathconf() fails with ENOTDIR if a component used as a directory
 *   in path is not in fact a directory.
 * - pathconf() fails with ENOENT if path is an empty string.
 * - pathconf() fails with ENAMETOOLONG if path is too long.
 * - pathconf() fails with EINVA if name is invalid.
 * - pathconf() fails with EACCES if search permission is denied for
 *   one of the directories in the path prefix of path.
 * - pathconf() fails with ELOOP if too many symbolic links were
 *   encountered while resolving path.
 */

#define FILEPATH "testfile/testfile_1"
#define TESTELOOP "test_eloop1"
#define PATH_LEN (PATH_MAX + 2)

#include <stdlib.h>
#include <pwd.h>
#include "tst_test.h"

static char *fpath;
static char *emptypath;
static char path[PATH_LEN];
static char *long_path = path;
static char *abs_path;
static char *testeloop;
static struct passwd *user;

static struct tcase {
	char **path;
	int name;
	int exp_errno;
	char *desc;
} tcases[] = {
	{&fpath, 0, ENOTDIR, "path prefix is not a directory"},
	{&emptypath, 0, ENOENT, "path is an empty string"},
	{&long_path, 0, ENAMETOOLONG, "path is too long"},
	{&abs_path, -1, EINVAL, "name is invalid"},
	{&abs_path, 0, EACCES, "without full permissions of the path prefix"},
	{&testeloop, 0, ELOOP, "too many symbolic links"},
};

static void verify_fpathconf(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	if (tc->exp_errno == EACCES)
		SAFE_SETEUID(user->pw_uid);

	TST_EXP_FAIL(pathconf(*tc->path, tc->name), tc->exp_errno,
		     "pathconf() fail with %s", tc->desc);

	if (tc->exp_errno == EACCES)
		SAFE_SETEUID(0);
}

static void setup(void)
{
	user = SAFE_GETPWNAM("nobody");

	SAFE_TOUCH("testfile", 0777, NULL);

	abs_path = tst_tmpdir_genpath(FILEPATH);

	SAFE_CHMOD(tst_tmpdir_path(), 0);

	memset(path, 'a', PATH_LEN);

	SAFE_SYMLINK("test_eloop1", "test_eloop2");
	SAFE_SYMLINK("test_eloop2", "test_eloop1");
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_fpathconf,
	.setup = setup,
	.needs_tmpdir = 1,
	.bufs = (struct tst_buffers []) {
		{&fpath, .str = FILEPATH},
		{&emptypath, .str = ""},
		{&testeloop, .str = TESTELOOP},
		{},
	},
	.needs_root = 1,
};
