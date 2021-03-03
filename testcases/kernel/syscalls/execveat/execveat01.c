// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Authors: Jinhui huang <huangjh.jy@cn.fujitsu.com>
 *          Xiao Yang <yangx.jy@cn.fujitsu.com>
 */
/* Test Description:
 *  This case checks the basic functionality of the execveat(2):
 *	1) When pathname is relative, it is relative to the directory where
 *	   the executed process is located and the dirfd is the descriptor of
 *	   the directory.
 *	2) When pathname is relative and dirfd is the special value AT_FDCWD,
 *	   the pathname is the relative to the current working directory of
 *	   the calling process.
 *	3) When pathname is absolute, dirfd can be ignored.
 *	4) When pathname is an empty string and the flag AT_EMPTY_PATH is
 *	   specified, dirfd specifies the file to be executed.
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
#define TEST_APP "execveat_child"
#define TEST_REL_APP TESTDIR"/"TEST_APP

static int fd1, fd4;
static int fd2 = AT_FDCWD, fd3 = -1;
static char app_abs_path[512];

static struct tcase {
	int *fd;
	char *pathname;
	int flag;
} tcases[] = {
	{&fd1, TEST_APP, 0},
	{&fd2, TEST_REL_APP, 0},
	{&fd3, app_abs_path, 0},
	{&fd4, "", AT_EMPTY_PATH},
};

static void verify_execveat(unsigned int i)
{
	struct tcase *tc = &tcases[i];
	char *argv[2] = {TEST_APP, NULL};
	pid_t pid;

	pid = SAFE_FORK();
	if (pid == 0) {
		TEST(execveat(*tc->fd, tc->pathname, argv, environ, tc->flag));
		tst_res(TFAIL | TTERRNO, "execveat() returns unexpected errno");
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

	fd1 = SAFE_OPEN(TESTDIR, O_DIRECTORY);
	fd4 = SAFE_OPEN(TEST_REL_APP, O_PATH);
}

static void cleanup(void)
{
	if (fd1 > 0)
		SAFE_CLOSE(fd1);

	if (fd4 > 0)
		SAFE_CLOSE(fd4);
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
