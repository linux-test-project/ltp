// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *		07/2001 Ported by Wayne Boyer
 *   Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that rename() fails with EPERM or EACCES when the directory
 * containing oldpath has the sticky bit (S_ISVTX) set and the caller
 * is not privileged.
 */

#include <stdio.h>
#include <pwd.h>
#include "tst_test.h"

#define MNT_POINT "mntpoint"
#define TEMP_DIR "tempdir"
#define TEMP_FILE1 TEMP_DIR"/tmpfile1"
#define TEMP_FILE2 TEMP_DIR"/tmpfile2"

static uid_t nobody_uid;
static struct stat buf1;

static void setup(void)
{
	struct passwd *pw;

	pw = SAFE_GETPWNAM("nobody");
	nobody_uid = pw->pw_uid;

	SAFE_CHDIR(MNT_POINT);
	SAFE_MKDIR(TEMP_DIR, 0777);
	SAFE_STAT(TEMP_DIR, &buf1);
	SAFE_CHMOD(TEMP_DIR, buf1.st_mode | S_ISVTX);
	SAFE_TOUCH(TEMP_FILE1, 0700, NULL);
}

static void run(void)
{
	SAFE_SETEUID(nobody_uid);

	TEST(rename(TEMP_FILE1, TEMP_FILE2));
	if (TST_RET == -1 && (TST_ERR == EPERM || TST_ERR == EACCES))
		tst_res(TPASS | TTERRNO, "rename() failed as expected");
	else if (TST_RET == 0)
		tst_res(TFAIL, "rename() succeeded unexpectedly");
	else
		tst_res(TFAIL | TTERRNO, "rename() failed, but not with expected errno");
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.mntpoint = MNT_POINT,
	.mount_device = 1,
	.all_filesystems = 1,
	.skip_filesystems = (const char *const[]){
		"exfat",
		"vfat",
		"fuse",
		"ntfs",
		NULL
	},
};
