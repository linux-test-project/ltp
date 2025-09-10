// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by Wayne Boyer
 *      11/2010 Rewritten by Cyril Hrubis chrubis@suse.cz
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Verify that lchown(2) fails with errno:
 *
 * - EPERM, if the effective user id of process does not match the owner of
 *   the file and the process is not super user.
 * - EACCES, if search permission is denied on a component of the path prefix.
 * - EFAULT, if pathname points outside user's accessible address space.
 * - ENAMETOOLONG, if the pathname component is too long.
 * - ENOTDIR, if the directory component in pathname is not a directory.
 * - ENOENT, if the specified file does not exists.
 */

#include <pwd.h>

#include "tst_test.h"
#include "compat_tst_16.h"

#define TEST_USER "nobody"
#define DIR_TEMP "testdir_1"
#define TFILE1 "tfile_1"
#define SFILE1 "sfile_1"
#define TFILE2 "testdir_1/tfile_2"
#define SFILE2 "testdir_1/sfile_2"
#define TFILE3 "t_file"
#define SFILE3 "t_file/sfile"
#define MAXPATH (PATH_MAX + 2)

static char *sfile1;
static char *sfile2;
static char *bad_addr;
static char *maxpath;
static char *sfile3;
static char *empty;
static struct passwd *ltpuser;

static struct test_case_t {
	char **pathname;
	char *desc;
	int exp_errno;
} test_cases[] = {
	{ &sfile1, "Process is not owner/root", EPERM },
	{ &sfile2, "Search permission denied", EACCES },
	{ &bad_addr, "Unaccessible address space", EFAULT },
	{ &maxpath, "Pathname too long", ENAMETOOLONG },
	{ &sfile3, "Path contains regular file", ENOTDIR },
	{ &empty, "Pathname is empty", ENOENT },
};

static void run(unsigned int i)
{
	struct test_case_t *tc = &test_cases[i];

	TST_EXP_FAIL(lchown(*tc->pathname, ltpuser->pw_uid, ltpuser->pw_gid),
		     tc->exp_errno, "%s", tc->desc);
}

static void setup(void)
{
	bad_addr = tst_get_bad_addr(NULL);

	memset(maxpath, 'a', MAXPATH - 1);
	maxpath[MAXPATH - 1] = 0;

	ltpuser = SAFE_GETPWNAM(TEST_USER);
	SAFE_SETGID(ltpuser->pw_uid);

	UID16_CHECK(ltpuser->pw_uid, "lchown");
	GID16_CHECK(ltpuser->pw_gid, "lchown");

	SAFE_TOUCH(TFILE1, 0666, NULL);
	SAFE_SETEUID(0);
	SAFE_SYMLINK(TFILE1, SFILE1);
	SAFE_SETEUID(ltpuser->pw_uid);

	SAFE_MKDIR(DIR_TEMP, 0777);
	SAFE_TOUCH(TFILE2, 0666, NULL);
	SAFE_SYMLINK(TFILE2, SFILE2);
	SAFE_CHMOD(DIR_TEMP, 0644);

	SAFE_TOUCH(TFILE3, 0777, NULL);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(test_cases),
	.test = run,
	.setup = setup,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.bufs = (struct tst_buffers []) {
		{&maxpath, .size = MAXPATH},
		{&sfile1, .str = SFILE1},
		{&sfile2, .str = SFILE2},
		{&sfile3, .str = SFILE3},
		{&empty, .str = ""},
		{}
	},
};
