/*
 * Copyright (c) 2015 Fujitsu Ltd.
 * Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License
 * alone with this program.
 */

/*
 * DESCRIPTION
 *  Test for feature UMOUNT_NOFOLLOW of umount2().
 *  "Don't dereference target if it is a symbolic link,
 *   and fails with the error EINVAL."
 */

#include <errno.h>
#include <sys/mount.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/mount.h"

#include "umount2.h"

#define DIR_MODE	(S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#define MNTPOINT	"mntpoint"
#define SYMLINK	"symlink"

static void setup(void);
static void test_umount2(int i);
static void verify_failure(int i);
static void verify_success(int i);
static void cleanup(void);

static const char *device;
static const char *fs_type;

static int mount_flag;

static struct test_case_t {
	const char *mntpoint;
	int exp_errno;
	const char *desc;
} test_cases[] = {
	{SYMLINK, EINVAL,
		"umount2('symlink', UMOUNT_NOFOLLOW) expected EINVAL"},
	{MNTPOINT, 0,
		"umount2('mntpoint', UMOUNT_NOFOLLOW) expected success"},
};

char *TCID = "umount2_03";
int TST_TOTAL = ARRAY_SIZE(test_cases);

int main(int ac, char **av)
{
	int lc;
	int tc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (tc = 0; tc < TST_TOTAL; tc++)
			test_umount2(tc);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_require_root();

	if ((tst_kvercmp(2, 6, 34)) < 0) {
		tst_brkm(TCONF, NULL, "This test can only run on kernels "
			 "that are 2.6.34 or higher");
	}

	tst_sig(NOFORK, DEF_HANDLER, NULL);

	tst_tmpdir();

	fs_type = tst_dev_fs_type();
	device = tst_acquire_device(cleanup);

	if (!device)
		tst_brkm(TCONF, cleanup, "Failed to obtain block device");

	tst_mkfs(cleanup, device, fs_type, NULL, NULL);

	SAFE_MKDIR(cleanup, MNTPOINT, DIR_MODE);

	SAFE_SYMLINK(cleanup, MNTPOINT, SYMLINK);

	TEST_PAUSE;
}

static void test_umount2(int i)
{
	SAFE_MOUNT(cleanup, device, MNTPOINT, fs_type, 0, NULL);
	mount_flag = 1;

	TEST(umount2_retry(test_cases[i].mntpoint, UMOUNT_NOFOLLOW));

	if (test_cases[i].exp_errno != 0)
		verify_failure(i);
	else
		verify_success(i);

	if (mount_flag) {
		if (tst_umount(MNTPOINT))
			tst_brkm(TBROK, cleanup, "umount() failed");
		mount_flag = 0;
	}
}

static void verify_failure(int i)
{
	if (TEST_RETURN == 0) {
		tst_resm(TFAIL, "%s passed unexpectedly", test_cases[i].desc);
		mount_flag = 0;
		return;
	}

	if (TEST_ERRNO != test_cases[i].exp_errno) {
		tst_resm(TFAIL | TTERRNO, "%s failed unexpectedly",
			 test_cases[i].desc);
		return;
	}

	tst_resm(TPASS | TTERRNO, "umount2(2) failed as expected");
}

static void verify_success(int i)
{
	if (TEST_RETURN != 0) {
		tst_resm(TFAIL | TTERRNO, "%s failed unexpectedly",
			 test_cases[i].desc);
		return;
	}

	tst_resm(TPASS, "umount2(2) succeeded as expected");
	mount_flag = 0;
}

static void cleanup(void)
{
	if (mount_flag && tst_umount(MNTPOINT))
		tst_resm(TWARN | TERRNO, "Failed to unmount");

	if (device)
		tst_release_device(device);

	tst_rmdir();
}
