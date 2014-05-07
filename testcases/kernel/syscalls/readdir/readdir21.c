/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*
 * Test Description:
 *  Verify that,
 *   1. Creat a directory and open it, then delete the directory, ENOENT would
 *	return.
 *   2. File descriptor does not refer to a directory, ENOTDIR would return.
 *
 *  PS:
 *   This file is for readdir(2) and the other files(readdir01.c and
 *   readdir02.c) are for readdir(3).
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "test.h"
#include "usctest.h"
#include "safe_macros.h"
#include "linux_syscall_numbers.h"
#include "lapi/readdir.h"

char *TCID = "readdir21";

#define TEST_DIR	"test_dir"
#define TEST_FILE	"test_file"
#define DIR_MODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP| \
			 S_IXGRP|S_IROTH|S_IXOTH)

static unsigned int del_dir_fd, file_fd;
static struct old_linux_dirent dirp;
static void setup(void);
static void cleanup(void);

static struct test_case_t {
	unsigned int *fd;
	struct old_linux_dirent *dirp;
	unsigned int count;
	int exp_errno;
} test_cases[] = {
	{&del_dir_fd, &dirp, sizeof(struct old_linux_dirent), ENOENT},
	{&file_fd, &dirp, sizeof(struct old_linux_dirent), ENOTDIR},
};

int TST_TOTAL = ARRAY_SIZE(test_cases);
static int exp_enos[] = { ENOENT, ENOTDIR, 0 };
static void readdir_verify(const struct test_case_t *);

int main(int argc, char **argv)
{
	int i, lc;
	char *msg;

	msg = parse_opts(argc, argv, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			readdir_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;

	tst_tmpdir();

	SAFE_MKDIR(cleanup, TEST_DIR, DIR_MODE);
	del_dir_fd = SAFE_OPEN(cleanup, TEST_DIR, O_RDONLY | O_DIRECTORY);
	if (rmdir(TEST_DIR) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "rmdir failed");

	file_fd = SAFE_OPEN(cleanup, TEST_FILE, O_RDWR | O_CREAT, 0777);
}

static void readdir_verify(const struct test_case_t *test)
{
	TEST(ltp_syscall(__NR_readdir, *test->fd, test->dirp, test->count));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "readdir() succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO == test->exp_errno) {
		tst_resm(TPASS | TTERRNO, "readdir() failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "readdir() failed unexpectedly; expected: %d - %s",
			 test->exp_errno, strerror(test->exp_errno));
	}
}

static void cleanup(void)
{
	TEST_CLEANUP;

	if (del_dir_fd > 0)
		close(del_dir_fd);

	if (file_fd > 0)
		close(file_fd);

	tst_rmdir();
}
