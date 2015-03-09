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
 *   1. path is NULL, EFAULT would return.
 *   2. Too many symbolic links were encountered in translating path,
 *	ELOOP would return.
 *   3. path is too long, ENAMETOOLONG would return.
 *   4. The file referred to by path does not exist, ENOENT would return.
 *   5. A component of the path prefix of path is not a directory,
 *	ENOENT would return.
 */

#include <errno.h>
#include <sys/statvfs.h>
#include <sys/mman.h>

#include "test.h"
#include "safe_macros.h"

#define TEST_SYMLINK	"statvfs_symlink"
#define TEST_FILE	"statvfs_file"

char *TCID = "statvfs02";

static struct statvfs buf;
static char nametoolong[PATH_MAX+2];
static void setup(void);
static void cleanup(void);

static struct test_case_t {
	char *path;
	struct statvfs *buf;
	int exp_errno;
} test_cases[] = {
	{NULL, &buf, EFAULT},
	{TEST_SYMLINK, &buf, ELOOP},
	{nametoolong, &buf, ENAMETOOLONG},
	{"filenoexist", &buf, ENOENT},
	{"statvfs_file/test", &buf, ENOTDIR},
};

int TST_TOTAL = ARRAY_SIZE(test_cases);
static void statvfs_verify(const struct test_case_t *);

int main(int argc, char **argv)
{
	int i, lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			statvfs_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	SAFE_SYMLINK(cleanup, TEST_SYMLINK, "statfs_symlink_2");
	SAFE_SYMLINK(cleanup, "statfs_symlink_2", TEST_SYMLINK);

	memset(nametoolong, 'a', PATH_MAX+1);

	SAFE_TOUCH(cleanup, TEST_FILE, 0644, NULL);

	test_cases[0].path = SAFE_MMAP(cleanup, NULL, 1, PROT_NONE,
	                               MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}

static void statvfs_verify(const struct test_case_t *test)
{
	TEST(statvfs(test->path, test->buf));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "statvfs() succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO == test->exp_errno) {
		tst_resm(TPASS | TTERRNO, "statvfs() failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "statvfs() failed unexpectedly; expected: %d - %s",
			 test->exp_errno, strerror(test->exp_errno));
	}
}

static void cleanup(void)
{
	tst_rmdir();
}
