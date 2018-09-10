// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Zilogic Systems Pvt. Ltd., 2018
 * Email: code@zilogic.com
 */

/*
 * Test statx
 *
 * 1) STATX_ATTR_ENCRYPTED - A key is required for the file to be encrypted by
 *                          the filesystem.
 *
 * e4crypt is used to set the encrypt flag (currently supported only by ext4).
 *
 * Two directories are tested.
 * First directory has all flags set.
 * Second directory has no flags set.
 *
 * Minimum kernel version required is 4.11.
 */

#include "tst_test.h"
#include "lapi/fs.h"
#include <stdlib.h>
#include "lapi/stat.h"

#define MOUNT_POINT "mnt_point"
#define TESTDIR_FLAGGED MOUNT_POINT"/test_dir1"
#define TESTDIR_UNFLAGGED MOUNT_POINT"/test_dir2"

static void test_flagged(void)
{
	struct statx buf;

	TEST(statx(AT_FDCWD, TESTDIR_FLAGGED, 0, 0, &buf));
	if (TST_RET == 0)
		tst_res(TPASS,
			"sys_statx(AT_FDCWD, %s, 0, 0, &buf)", TESTDIR_FLAGGED);
	else
		tst_brk(TFAIL | TTERRNO,
			"sys_statx(AT_FDCWD, %s, 0, 0, &buf)", TESTDIR_FLAGGED);

	if (buf.stx_attributes & STATX_ATTR_ENCRYPTED)
		tst_res(TPASS, "STATX_ATTR_ENCRYPTED flag is set");
	else
		tst_res(TFAIL, "STATX_ATTR_ENCRYPTED flag is not set");
}

static void test_unflagged(void)
{
	struct statx buf;

	TEST(statx(AT_FDCWD, TESTDIR_UNFLAGGED, 0, 0, &buf));
	if (TST_RET == 0)
		tst_res(TPASS,
			"sys_statx(AT_FDCWD, %s, 0, 0, &buf)",
			TESTDIR_UNFLAGGED);
	else
		tst_brk(TFAIL | TTERRNO,
			"sys_statx(AT_FDCWD, %s, 0, 0, &buf)",
			TESTDIR_UNFLAGGED);

	if ((buf.stx_attributes & STATX_ATTR_ENCRYPTED) == 0)
		tst_res(TPASS, "STATX_ATTR_ENCRYPTED flag is not set");
	else
		tst_res(TFAIL, "STATX_ATTR_ENCRYPTED flag is set");
}

struct test_cases {
	void (*tfunc)(void);
} tcases[] = {
	{&test_flagged},
	{&test_unflagged},
};

static void run(unsigned int i)
{
	tcases[i].tfunc();
}

static void setup(void)
{
	SAFE_MKDIR(TESTDIR_FLAGGED, 0777);
	SAFE_MKDIR(TESTDIR_UNFLAGGED, 0777);

	TEST(tst_system("echo qwery | e4crypt add_key "TESTDIR_FLAGGED));

	if (WEXITSTATUS(TST_RET) == 127)
		tst_brk(TCONF, "e4crypt not installed!");

	if (WEXITSTATUS(TST_RET))
		tst_brk(TCONF, "e4crypt failed (CONFIG_EXT4_ENCRYPTION not set?)");
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.min_kver = "4.11",
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MOUNT_POINT,
	.dev_fs_type = "ext4",
	.dev_extra_opt = "-O encrypt",
	.dev_min_size = 512,
};
