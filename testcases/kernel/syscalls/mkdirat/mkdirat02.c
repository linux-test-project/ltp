// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 */
/*
 * DESCRIPTION
 *	check mkdirat() with various error conditions that should produce
 *	ELOOP and EROFS.
 */

#define _GNU_SOURCE
#include "tst_test.h"

#define MNT_POINT	"mntpoint"
#define TEST_DIR	"mntpoint/test_dir"
#define DIR_MODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP| \
			 S_IXGRP|S_IROTH|S_IXOTH)

static int dir_fd;
static int cur_fd = AT_FDCWD;
static char test_dir[PATH_MAX] = ".";

static struct tcase {
	int *dirfd;
	char *pathname;
	int exp_errno;
} tcases[] = {
	{&dir_fd, TEST_DIR, EROFS},
	{&cur_fd, TEST_DIR, EROFS},
	{&dir_fd, test_dir, ELOOP},
	{&cur_fd, test_dir, ELOOP},
};

static void setup(void)
{
	unsigned int i;

	dir_fd = SAFE_OPEN(".", O_DIRECTORY);

	SAFE_MKDIR("test_eloop", DIR_MODE);
	SAFE_SYMLINK("../test_eloop", "test_eloop/test_eloop");

	/*
	 * NOTE: the ELOOP test is written based on that the consecutive
	 * symlinks limits in kernel is hardwired to 40.
	 */
	for (i = 0; i < 43; i++)
		strcat(test_dir, "/test_eloop");
}

static void mkdirat_verify(unsigned int i)
{
	struct tcase *test = &tcases[i];

	TEST(mkdirat(*test->dirfd, test->pathname, 0777));

	if (TST_RET != -1) {
		tst_res(TFAIL, "mkdirat() succeeded unexpectedly (%li)",
			TST_RET);
		return;
	}

	if (TST_ERR == test->exp_errno) {
		tst_res(TPASS | TTERRNO, "mkdirat() failed as expected");
		return;
	}

	tst_res(TFAIL | TTERRNO,
		"mkdirat() failed unexpectedly; expected: %d - %s",
		test->exp_errno, tst_strerrno(test->exp_errno));
}

static struct tst_test test = {
	.setup = setup,
	.test = mkdirat_verify,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.needs_rofs = 1,
	.mntpoint = MNT_POINT,
};
