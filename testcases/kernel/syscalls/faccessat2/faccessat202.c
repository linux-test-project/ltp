// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 FUJITSU LIMITED. All rights reserved.
 * Copyright (c) Linux Test Project, 2003-2023
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * [Description]
 *
 * Test basic error handling of faccessat2 syscall:
 *
 * - faccessat2() fails with EFAULT if pathname is a bad pathname point.
 * - faccessat2() fails with EINVAL if flags is -1.
 * - faccessat2() fails with EINVAL if mode is -1.
 * - faccessat2() fails with EBADF if dirfd is -1.
 * - faccessat2() fails with ENOTDIR if pathname is relative path to a
 *   file and dir_fd is file descriptor for this file.
 * - faccessat2() fails with EACCES if flags is AT_EACCESS and not using
 *   the effective user and group IDs.
 *
 * Minimum Linux version required is v5.8.
 */

#include <pwd.h>

#include "tst_test.h"
#include "lapi/syscalls.h"
#include "lapi/faccessat.h"

#define TESTUSER        "nobody"
#define TESTDIR         "faccessat2dir"
#define RELPATH         "faccessat2dir/faccessat2file"

static int fd;
static int bad_fd = -1;
static int atcwd_fd = AT_FDCWD;
static char *bad_path;
static char *rel_path;

static struct passwd *ltpuser;

static struct tcase {
	int *fd;
	char **filename;
	int mode;
	int flags;
	int exp_errno;
	const char *desc;
} tcases[] = {
	{&atcwd_fd, &bad_path, R_OK, 0, EFAULT, "invalid address"},
	{&atcwd_fd, &rel_path, R_OK, -1, EINVAL, "invalid flags"},
	{&atcwd_fd, &rel_path, -1, 0, EINVAL, "invalid mode"},
	{&bad_fd, &rel_path, R_OK, 0, EBADF, "invalid fd"},
	{&fd, &rel_path, R_OK, 0, ENOTDIR, "fd pointing to file"},
	{&atcwd_fd, &rel_path, R_OK, AT_EACCESS, EACCES,
         "AT_EACCESS and unprivileged EUID"},
};

static void verify_faccessat2(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	if (tc->exp_errno == EACCES)
		SAFE_SETEUID(ltpuser->pw_uid);

	TST_EXP_FAIL(faccessat2(*tc->fd, *tc->filename, tc->mode, tc->flags),
		     tc->exp_errno, "faccessat2() with %s", tc->desc);

	if (tc->exp_errno == EACCES)
		SAFE_SETEUID(0);
}

static void setup(void)
{
	SAFE_MKDIR(TESTDIR, 0666);
	SAFE_TOUCH(RELPATH, 0444, NULL);

	fd = SAFE_OPEN(RELPATH, O_RDONLY);
	bad_path = tst_get_bad_addr(NULL);

	ltpuser = SAFE_GETPWNAM(TESTUSER);
}

static void cleanup(void)
{
	if (fd > -1)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test = verify_faccessat2,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.bufs = (struct tst_buffers []) {
		{&rel_path, .str = RELPATH},
		{},
	},
	.needs_tmpdir = 1,
	.needs_root = 1,
};
