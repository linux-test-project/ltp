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
 */
/*
 * Test Description:
 *  Verify that,
 *   1. link() fails with -1 return value and sets errno to EPERM
 *      if oldpath is a directory.
 *   2. link() fails with -1 return value and sets errno to EXDEV
 *      if oldpath and newpath are not on the same mounted file system( Linux
 *      permits a file system to be mounted at multiple points, but link()
 *      does not work across different mount points, even if the same
 *      file system is mounted on both. ).
 *   3. link() fails with -1 return value and sets errno to EROFS
 *      if the file is on a read-only file system.
 *   4. link() fails with -1 return value and sets errno to ELOOP
 *      if too many symbolic links were encountered in resolving path.
 */
#include <errno.h>
#include "tst_test.h"

#define DIR_MODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP| \
			 S_IXGRP|S_IROTH|S_IXOTH)
#define MNT_POINT	"mntpoint"
#define TEST_FILE	"testfile"
#define TEST_FILE1	"testfile1"
#define TEST_FILE2	"mntpoint/file"
#define TEST_FILE3	"mntpoint/testfile4"

static char test_file4[PATH_MAX] = ".";
static void setup(void);

static struct tcase {
	char *oldpath;
	char *newpath;
	int exp_errno;
} tcases[] = {
	{TEST_FILE1, TEST_FILE, EPERM},
	{TEST_FILE2, TEST_FILE, EXDEV},
	{TEST_FILE2, TEST_FILE3, EROFS},
	{test_file4, TEST_FILE, ELOOP},
};

static void link_verify(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TEST(link(tc->oldpath, tc->newpath));

	if (TST_RET != -1) {
		tst_res(TFAIL, "link() succeeded unexpectedly (%li)",
		        TST_RET);
		return;
	}

	if (TST_ERR == tc->exp_errno) {
		tst_res(TPASS | TTERRNO, "link() failed as expected");
		return;
	}

	tst_res(TFAIL | TTERRNO,
		"link() failed unexpectedly; expected: %d - %s",
		tc->exp_errno, tst_strerrno(tc->exp_errno));
}

static void setup(void)
{
	int i;

	SAFE_MKDIR(TEST_FILE1, DIR_MODE);

	SAFE_MKDIR("test_eloop", DIR_MODE);
	SAFE_SYMLINK("../test_eloop", "test_eloop/test_eloop");
	for (i = 0; i < 43; i++)
		strcat(test_file4, "/test_eloop");
}

static struct tst_test test = {
	.setup = setup,
	.test = link_verify,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.needs_rofs = 1,
	.mntpoint = MNT_POINT,
};
