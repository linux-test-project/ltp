// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2006
 * Copyright (c) Linux Test Project, 2003-2023
 * 08/28/2006 AUTHOR: Yi Yang <yyangcdl@cn.ibm.com>
 */

/*\
 * Check the basic functionality of the fchmodat() system call.
 *
 * - fchmodat() passes if dir_fd is file descriptor to the directory
 *   where the file is located and pathname is relative path of the file.
 * - fchmodat() passes if pathname is absolute, then dirfd is ignored.
 * - fchmodat() passes if dir_fd is AT_FDCWD and pathname is interpreted
 *   relative to the current working directory of the calling process.
 */

#include <stdlib.h>
#include <stdio.h>
#include "tst_test.h"

#define TESTDIR         "fchmodatdir"
#define TESTFILE        "fchmodatfile"
#define FILEPATH        "fchmodatdir/fchmodatfile"

static int dir_fd, file_fd;
static int atcwd_fd = AT_FDCWD;
static char *abs_path;
static char *test_file;
static char *file_path;

static struct tcase {
	int *fd;
	char **filenames;
	char **full_path;
} tcases[] = {
	{&dir_fd, &test_file, &file_path},
	{&file_fd, &abs_path, &abs_path},
	{&atcwd_fd, &file_path, &file_path},
};

static void verify_fchmodat(unsigned int i)
{
	struct tcase *tc = &tcases[i];
	struct stat st;

	TST_EXP_PASS(fchmodat(*tc->fd, *tc->filenames, 0600, 0),
		     "fchmodat(%d, %s, 0600, 0)",
		     *tc->fd, *tc->filenames);

	SAFE_LSTAT(*tc->full_path, &st);

	if ((st.st_mode & ~S_IFREG) == 0600)
		tst_res(TPASS, "File permission changed correctly");
	else
		tst_res(TFAIL, "File permission not changed correctly");
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
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_fchmodat,
	.setup = setup,
	.cleanup = cleanup,
	.bufs = (struct tst_buffers []) {
		{&test_file, .str = TESTFILE},
		{&file_path, .str = FILEPATH},
		{},
	},
	.needs_tmpdir = 1,
};
