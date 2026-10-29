// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 IBM
 *
 * Original shell test by Kai Zhao (ltcd3@cn.ibm.com)
 * Converted to C by Sachin Sant <sachinp@linux.ibm.com>
 */

/*\
 * Test default ACL inheritance using direct xattr manipulation.
 *
 * Verify that files created in a directory with default ACLs inherit
 * those ACLs as their access ACLs. Default ACLs are only applicable
 * to directories and define the access ACLs that files and subdirectories
 * created within that directory will inherit.
 *
 * This test uses arbitrary UIDs without creating actual users, testing
 * only the kernel ACL implementation.
 *
 * [Algorithm]
 *
 * - Set default ACL on parent directory with read-only permissions
 * - Create a new file in that directory with umask 0
 * - Verify the file inherits the default ACL as its access ACL
 * - Check that file permissions match the inherited ACL (0444)
 */

#include "acl_lib.h"

#ifdef HAVE_SYS_XATTR_H

#define TEST_UID 1000
#define TEST_GID 1000

static void run(void)
{
	struct acl *acl;
	struct stat st;

	tst_res(TINFO, "Testing default ACL inheritance");
	reset_test_path();

	SAFE_CHOWN(TESTDIR, TEST_UID, TEST_GID);

	acl = acl_init();

	acl_add_entry(acl, ACL_USER_OBJ, ACL_READ, 0);
	acl_add_entry(acl, ACL_GROUP_OBJ, ACL_READ, 0);
	acl_add_entry(acl, ACL_OTHER, ACL_READ, 0);

	SAFE_ACL_SET_FILE(TESTDIR, ACL_TYPE_DEFAULT, acl);

	acl_free(acl);

	create_with_umask_as(TEST_UID, TEST_GID, 0666, 0, 0);

	SAFE_STAT(TESTFILE, &st);

	/*
	 * For a minimal ACL (containing only ACL_USER_OBJ, ACL_GROUP_OBJ,
	 * and ACL_OTHER), the mode bits are the canonical representation.
	 * Verifying the mode bits confirms the inherited ACL was applied.
	 */
	if ((st.st_mode & 0777) != 0444) {
		tst_res(TFAIL,
			"File permissions 0%o, expected 0444 from default ACL",
			st.st_mode & 0777);
		cleanup_testfile();
		return;
	}

	cleanup_testfile();
	tst_res(TPASS, "Default ACL inheritance works correctly");
}

static void cleanup(void)
{
	cleanup_test_paths();
}

static struct tst_test test = {
	.test_all = run,
	.cleanup = cleanup,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.forks_child = 1,
	.filesystems = (struct tst_fs[]) {
		{.type = "ext2", .mnt_data = "acl"},
		{.type = "ext3", .mnt_data = "acl"},
		{.type = "ext4", .mnt_data = "acl"},
		{.type = "xfs"},
		{.type = "btrfs"},
		{}
	}
};

#else
	TST_TEST_TCONF("sys/xattr.h is not available");
#endif
