/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Description:
 *  Verify that,
 *   1. rename() fails with -1 return value and sets errno to ELOOP, if too
 *      many symbolic links were encountered in resolving oldpath or newpath.
 *   2. rename() fails with -1 return value and sets errno to EROFS,
 *      if the file is on a read-only file system.
 *   3. rename() fails with -1 return value and sets errno to EMLINK,
 *	if the file named by old is a directory and the link count of
 *	the parent directory of new would exceed {LINK_MAX}.
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mount.h>

#include "test.h"
#include "tso_safe_macros.h"

char *TCID = "rename11";

#define MNTPOINT	"mntpoint"
#define TEST_EROFS	"mntpoint/test_erofs"
#define TEST_NEW_EROFS	"mntpoint/new_test_erofs"

#define TEST_EMLINK	"test_emlink"
#define TEST_NEW_EMLINK	"emlink_dir/testdir"

#define TEST_NEW_ELOOP	"new_test_eloop"
#define ELOPFILE	"/test_eloop"
static char elooppathname[sizeof(ELOPFILE) * 43] = ".";
static int max_subdirs;

static const char *device;
static const char *fs_type;
static int mount_flag;

static void cleanup(void);
static void setup(void);
static void test_eloop(void);
static void test_erofs(void);
static void test_emlink(void);

static void (*testfunc[])(void) = { test_eloop, test_erofs, test_emlink };

int TST_TOTAL = ARRAY_SIZE(testfunc);

int main(int ac, char **av)
{
	int lc, i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			(*testfunc[i])();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int i;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root();

	tst_tmpdir();

	TEST_PAUSE;

	fs_type = tst_dev_fs_type();
	device = tst_acquire_device(cleanup);

	if (!device)
		tst_brkm(TCONF, cleanup, "Failed to obtain block device");

	tst_mkfs(cleanup, device, fs_type, NULL, NULL);

	SAFE_MKDIR(cleanup, MNTPOINT, 0755);
	SAFE_MOUNT(cleanup, device, MNTPOINT, fs_type, 0, NULL);
	mount_flag = 1;
	SAFE_TOUCH(cleanup, TEST_EROFS, 0644, NULL);

	SAFE_MKDIR(cleanup, TEST_EMLINK, 0755);
	max_subdirs = tst_fs_fill_subdirs(cleanup, "emlink_dir");
	/*
	 * NOTE: the ELOOP test is written based on that the consecutive
	 * symlinks limits in kernel is hardwired to 40.
	 */
	SAFE_MKDIR(cleanup, "test_eloop", 0644);
	SAFE_SYMLINK(cleanup, "../test_eloop", "test_eloop/test_eloop");
	for (i = 0; i < 43; i++)
		strcat(elooppathname, ELOPFILE);
}

static void check_and_print(int expected_errno)
{
	if (TEST_RETURN == -1) {
		if (TEST_ERRNO == expected_errno) {
			tst_resm(TPASS | TTERRNO, "failed as expected");
		} else {
			tst_resm(TFAIL | TTERRNO,
				 "failed unexpectedly; expected - %d : %s",
				 expected_errno, strerror(expected_errno));
		}
	} else {
		tst_resm(TFAIL, "rename succeeded unexpectedly");
	}
}

static void test_eloop(void)
{
	TEST(rename(elooppathname, TEST_NEW_ELOOP));
	check_and_print(ELOOP);

	if (TEST_RETURN == 0)
		SAFE_UNLINK(cleanup, TEST_NEW_ELOOP);
}

static void test_erofs(void)
{
	SAFE_MOUNT(cleanup, device, MNTPOINT, fs_type, MS_REMOUNT | MS_RDONLY,
		   NULL);

	TEST(rename(TEST_EROFS, TEST_NEW_EROFS));
	check_and_print(EROFS);

	if (TEST_RETURN == 0)
		SAFE_UNLINK(cleanup, TEST_NEW_EROFS);

	SAFE_MOUNT(cleanup, device, MNTPOINT, fs_type, MS_REMOUNT, NULL);
}

static void test_emlink(void)
{
	if (max_subdirs == 0) {
		tst_resm(TCONF, "EMLINK test is not appropriate");
		return;
	}

	TEST(rename(TEST_EMLINK, TEST_NEW_EMLINK));
	check_and_print(EMLINK);

	if (TEST_RETURN == 0)
		SAFE_RMDIR(cleanup, TEST_NEW_EMLINK);
}

static void cleanup(void)
{
	if (mount_flag && tst_umount(MNTPOINT) < 0)
		tst_resm(TWARN | TERRNO, "umount device:%s failed", device);

	if (device)
		tst_release_device(device);

	tst_rmdir();
}
