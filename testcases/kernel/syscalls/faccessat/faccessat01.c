// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2006
 * Copyright (c) Linux Test Project, 2003-2023
 * Author: Yi Yang <yyangcdl@cn.ibm.com>
 */

/*\
 * Check the basic functionality of the faccessat() system call.
 *
 * - faccessat() passes if dir_fd is file descriptor to the directory
 *   where the file is located and pathname is relative path of the file.
 *
 * - faccessat() passes if dir_fd is a bad file descriptor and pathname is
 *   absolute path of the file.
 *
 * - faccessat() passes if dir_fd is AT_FDCWD and pathname is interpreted
 *   relative to the current working directory of the calling process.
 */

#include <stdlib.h>
#include <stdio.h>
#include "tst_test.h"

#define TESTDIR         "faccessatdir"
#define TESTFILE        "faccessatfile"
#define FILEPATH        "faccessatdir/faccessatfile"

static int dir_fd, file_fd;
static int atcwd_fd = AT_FDCWD;
static char *abs_path;
static char *test_file;
static char *file_path;

static struct tcase {
	int *fd;
	char **filename;
	int exp_errno;
} tcases[] = {
	{&dir_fd, &test_file, 0},
	{&dir_fd, &abs_path, 0},
	{&atcwd_fd, &file_path, 0},
};

static void verify_faccessat(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TST_EXP_PASS(faccessat(*tc->fd, *tc->filename, R_OK, 0),
		     "faccessat(%d, %s, R_OK, 0)",
		     *tc->fd, *tc->filename);
}

static void setup(void)
{
	abs_path = tst_tmpdir_genpath(FILEPATH);

	SAFE_MKDIR(TESTDIR, 0700);
	dir_fd = SAFE_OPEN(TESTDIR, O_DIRECTORY);
	file_fd = SAFE_OPEN(FILEPATH, O_CREAT | O_RDWR, 0600);
}

static void cleanup(void)
{
	if (dir_fd > -1)
		SAFE_CLOSE(dir_fd);

	if (file_fd > -1)
		SAFE_CLOSE(file_fd);
}

static struct tst_test test = {
	.test = verify_faccessat,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.bufs = (struct tst_buffers []) {
		{&test_file, .str = TESTFILE},
		{&file_path, .str = FILEPATH},
		{},
	},
	.needs_tmpdir = 1,
};
