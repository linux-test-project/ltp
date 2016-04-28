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
 *  Test for feature MNT_EXPIRE of umount2().
 *  "Mark the mount point as expired.If a mount point is not currently
 *   in use, then an initial call to umount2() with this flag fails with
 *   the error EAGAIN, but marks the mount point as expired. The mount
 *   point remains expired as long as it isn't accessed by any process.
 *   A second umount2() call specifying MNT_EXPIRE unmounts an expired
 *   mount point. This flag cannot be specified with either MNT_FORCE or
 *   MNT_DETACH. (fails with the error EINVAL)"
 */

#include <errno.h>
#include <sys/mount.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/mount.h"

#include "umount2.h"

#define DIR_MODE	(S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#define MNTPOINT	"mntpoint"

static void setup(void);
static void test_umount2(int i);
static void verify_failure(int i);
static void verify_success(int i);
static void cleanup(void);

static const char *device;
static const char *fs_type;

static int mount_flag;

static struct test_case_t {
	int flag;
	int exp_errno;
	int do_access;
	const char *desc;
} test_cases[] = {
	{MNT_EXPIRE | MNT_FORCE, EINVAL, 0,
		"umount2(2) with MNT_EXPIRE | MNT_FORCE expected EINVAL"},
	{MNT_EXPIRE | MNT_DETACH, EINVAL, 0,
		"umount2(2) with MNT_EXPIRE | MNT_DETACH expected EINVAL"},
	{MNT_EXPIRE, EAGAIN, 0,
		"initial call to umount2(2) with MNT_EXPIRE expected EAGAIN"},
	{MNT_EXPIRE, EAGAIN, 1,
		"umount2(2) with MNT_EXPIRE after access(2) expected EAGAIN"},
	{MNT_EXPIRE, 0, 0,
		"second call to umount2(2) with MNT_EXPIRE expected success"},
};

char *TCID = "umount2_02";
int TST_TOTAL = ARRAY_SIZE(test_cases);

int main(int ac, char **av)
{
	int lc;
	int tc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		SAFE_MOUNT(cleanup, device, MNTPOINT, fs_type, 0, NULL);
		mount_flag = 1;

		for (tc = 0; tc < TST_TOTAL; tc++)
			test_umount2(tc);

		if (mount_flag) {
			if (tst_umount(MNTPOINT))
				tst_brkm(TBROK, cleanup, "umount() failed");
			mount_flag = 0;
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_require_root();

	if ((tst_kvercmp(2, 6, 8)) < 0) {
		tst_brkm(TCONF, NULL, "This test can only run on kernels "
			 "that are 2.6.8 or higher");
	}

	tst_sig(NOFORK, DEF_HANDLER, NULL);

	tst_tmpdir();

	fs_type = tst_dev_fs_type();
	device = tst_acquire_device(cleanup);

	if (!device)
		tst_brkm(TCONF, cleanup, "Failed to obtain block device");

	tst_mkfs(cleanup, device, fs_type, NULL, NULL);

	SAFE_MKDIR(cleanup, MNTPOINT, DIR_MODE);

	TEST_PAUSE;
}

static void test_umount2(int i)
{
	/* a new access removes the expired mark of the mount point */
	if (test_cases[i].do_access) {
		if (access(MNTPOINT, F_OK) == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "access(2) failed");
	}

	TEST(umount2_retry(MNTPOINT, test_cases[i].flag));

	if (test_cases[i].exp_errno != 0)
		verify_failure(i);
	else
		verify_success(i);
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
