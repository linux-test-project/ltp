/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */
/*
 * Test Description:
 *  Verify that,
 *   1. bufsiz is 0, EINVAL should be returned.
 *   2. The named file is not a symbolic link, EINVAL should be returned.
 *   3. The component of the path prefix is not a directory, ENOTDIR should be
 *	returned.
 *   4. pathname is relative and dirfd is a file descriptor referring to a file
 *	other than a directory, ENOTDIR should be returned.
 */

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/readlinkat.h"
#include "lapi/syscalls.h"

#define TEST_FILE	"test_file"
#define SYMLINK_FILE	"symlink_file"
#define BUFF_SIZE	256

static int file_fd, dir_fd;

static struct test_case_t {
	int *dirfd;
	const char *pathname;
	size_t bufsiz;
	int exp_errno;
} test_cases[] = {
	{&dir_fd, SYMLINK_FILE, 0, EINVAL},
	{&dir_fd, TEST_FILE, BUFF_SIZE, EINVAL},
	{&file_fd, SYMLINK_FILE, BUFF_SIZE, ENOTDIR},
	{&dir_fd, "test_file/test_file", BUFF_SIZE, ENOTDIR},
};

char *TCID = "readlinkat02";
int TST_TOTAL = ARRAY_SIZE(test_cases);
static void setup(void);
static void cleanup(void);
static void readlinkat_verify(const struct test_case_t *);

int main(int argc, char **argv)
{
	int i, lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			readlinkat_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	dir_fd = SAFE_OPEN(cleanup, "./", O_RDONLY);

	file_fd = SAFE_OPEN(cleanup, TEST_FILE, O_RDWR | O_CREAT, 0644);

	SAFE_SYMLINK(cleanup, TEST_FILE, SYMLINK_FILE);
}

static void readlinkat_verify(const struct test_case_t *test)
{
	char buf[BUFF_SIZE];
	TEST(readlinkat(*test->dirfd, test->pathname, buf, test->bufsiz));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "readlinkat succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO == test->exp_errno) {
		tst_resm(TPASS | TTERRNO, "readlinkat failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "readlinkat failed unexpectedly; expected: %d - %s",
			 test->exp_errno, strerror(test->exp_errno));
	}
}

static void cleanup(void)
{
	close(dir_fd);
	close(file_fd);

	tst_rmdir();
}
