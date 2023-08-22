// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 FUJITSU LIMITED. All rights reserved.
 * Copyright (c) Linux Test Project, 2003-2023
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * [Description]
 *
 * Check the basic functionality of faccessat2().
 *
 * Minimum Linux version required is v5.8.
 */

#include <stdlib.h>

#include "tst_test.h"
#include "lapi/syscalls.h"
#include "lapi/faccessat.h"

#define TESTDIR         "faccessat2dir"
#define TESTFILE        "faccessat2file"
#define RELPATH         "faccessat2dir/faccessat2file"
#define TESTSYMLINK     "faccessat2symlink"

static int dir_fd, bad_fd = -1;
static int atcwd_fd = AT_FDCWD;
static char *testfile;
static char *abs_path;
static char *rel_path;
static char *sym_path;

static struct tcase {
	int *fd;
	char **filename;
	int flags;
} tcases[] = {
	{&dir_fd, &testfile, 0},
	{&bad_fd, &abs_path, 0},
	{&atcwd_fd, &rel_path, 0},
	{&dir_fd, &testfile, AT_EACCESS},
	{&bad_fd, &abs_path, AT_EACCESS},
	{&atcwd_fd, &rel_path, AT_EACCESS},
	{&atcwd_fd, &sym_path, AT_SYMLINK_NOFOLLOW},
};

static void verify_faccessat2(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TST_EXP_PASS(faccessat2(*tc->fd, *tc->filename, R_OK, tc->flags),
		     "faccessat2(%d, %s, R_OK, %d)",
		     *tc->fd, *tc->filename, tc->flags);
}

static void setup(void)
{
	char *tmpdir_path = tst_get_tmpdir();

	abs_path = tst_aprintf("%s/%s", tmpdir_path, RELPATH);
	free(tmpdir_path);

	SAFE_MKDIR(TESTDIR, 0777);
	dir_fd = SAFE_OPEN(TESTDIR, O_DIRECTORY);
	SAFE_TOUCH(abs_path, 0444, NULL);
	SAFE_SYMLINK(abs_path, TESTSYMLINK);
}

static void cleanup(void)
{
	if (dir_fd > -1)
		SAFE_CLOSE(dir_fd);
}

static struct tst_test test = {
	.test = verify_faccessat2,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.bufs = (struct tst_buffers []) {
		{&testfile, .str = TESTFILE},
		{&rel_path, .str = RELPATH},
		{&sym_path, .str = TESTSYMLINK},
		{},
	},
	.needs_tmpdir = 1,
};
