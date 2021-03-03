// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Authors: Jinhui huang <huangjh.jy@cn.fujitsu.com>
 */

/* Test Description:
 *  Check various errnos for execveat(2):
 *    1) execveat() fails and returns EBADF if dirfd is a invalid file
 *	 descriptor.
 *    2) execveat() fails and returns EINVAL if flag specified is invalid.
 *    3) execveat() fails and returns ELOOP if the file identified by dirfd and
 *       pathname is a symbolic link and flag includes AT_SYMLINK_NOFOLLOW.
 *    4) execveat() fails and returns ENOTDIR if pathname is relative and dirfd
 *       is a file descriptor referring to a file other than a directory.
 */

#define _GNU_SOURCE
#include "config.h"

#include <stdio.h>
#include <errno.h>

#include "tst_test.h"
#include "lapi/execveat.h"
#include "lapi/fcntl.h"
#include "execveat.h"

#define TESTDIR "testdir"
#define TEST_APP "execveat_errno"
#define TEST_SYMLINK "execveat_symlink"
#define TEST_REL_APP TESTDIR"/"TEST_APP
#define TEST_ERL_SYMLINK TESTDIR"/"TEST_SYMLINK

static int bad_fd = -1, fd;
static char app_abs_path[512], app_sym_path[512];

static struct tcase {
	int *fd;
	char *pathname;
	int flag;
	int exp_err;
} tcases[] = {
	{&bad_fd, "", AT_EMPTY_PATH, EBADF},
	{&fd, app_abs_path, -1, EINVAL},
	{&fd, app_sym_path, AT_SYMLINK_NOFOLLOW, ELOOP},
	{&fd, TEST_REL_APP, 0, ENOTDIR},
};

static void verify_execveat(unsigned int i)
{
	struct tcase *tc = &tcases[i];
	char *argv[2] = {TEST_APP, NULL};
	pid_t pid;

	pid = SAFE_FORK();
	if (pid == 0) {
		TEST(execveat(*tc->fd, tc->pathname, argv, environ, tc->flag));
		if (tc->exp_err != TST_ERR) {
			tst_res(TFAIL | TTERRNO,
				"execveat() fails unexpectedly, expected: %s",
				tst_strerrno(tc->exp_err));
		} else {
			tst_res(TPASS | TTERRNO,
				"execveat() fails as expected");
		}
	}
}

static void setup(void)
{
	char cur_dir_path[512];

	check_execveat();

	SAFE_MKDIR(TESTDIR, 0777);

	SAFE_CP(TEST_APP, TEST_REL_APP);

	SAFE_GETCWD(cur_dir_path, sizeof(cur_dir_path));
	sprintf(app_abs_path, "%s/%s", cur_dir_path, TEST_REL_APP);
	sprintf(app_sym_path, "%s/%s", cur_dir_path, TEST_ERL_SYMLINK);

	SAFE_SYMLINK(TEST_REL_APP, TEST_ERL_SYMLINK);

	fd = SAFE_OPEN(TEST_REL_APP, O_PATH);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.resource_files = (const char *const []) {
		TEST_APP,
		NULL
	},
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_execveat,
	.child_needs_reinit = 1,
	.forks_child = 1,
	.cleanup = cleanup,
	.setup = setup,
};
