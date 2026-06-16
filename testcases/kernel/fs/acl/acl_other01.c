// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 IBM
 *
 * Original shell test by Kai Zhao (ltcd3@cn.ibm.com)
 * Converted to C by Sachin Sant <sachinp@linux.ibm.com>
 */

/*\
 * Test ACL_OTHER permissions using direct xattr manipulation.
 *
 * Verify that ACL_OTHER permissions work correctly and are not affected
 * by ACL_MASK. The ACL_OTHER entry controls access for users who don't
 * match any other ACL entry (not the owner, not in any named user entry,
 * not in the owning group, and not in any named group entry).
 *
 * Unlike ACL_USER, ACL_GROUP, and ACL_GROUP_OBJ entries, ACL_OTHER
 * permissions are not restricted by the ACL_MASK.
 *
 * This test uses arbitrary UIDs without creating actual users, testing
 * only the kernel ACL implementation.
 *
 * [Algorithm]
 *
 * - Set up ACL with rwx permissions for ACL_OTHER
 * - Set ACL_MASK to --- (no permissions)
 * - Attempt file creation as a user matching ACL_OTHER
 * - Verify access is granted despite restrictive mask
 */

#include "acl_lib.h"

#ifdef HAVE_SYS_XATTR_H

#define TEST_UID 1000
#define TEST_GID 1000
#define OTHER_UID 2000
#define OTHER_GID 2000

static void run(void)
{
	struct acl *acl;

	tst_res(TINFO, "Testing ACL_OTHER permissions");
	reset_test_path();

	SAFE_CHOWN(TESTDIR, TEST_UID, TEST_GID);

	acl = acl_init();

	acl_add_entry(acl, ACL_USER_OBJ,
		      ACL_READ | ACL_WRITE | ACL_EXECUTE, 0);
	acl_add_entry(acl, ACL_GROUP_OBJ, 0, 0);
	acl_add_entry(acl, ACL_MASK, 0, 0);
	acl_add_entry(acl, ACL_OTHER,
		      ACL_READ | ACL_WRITE | ACL_EXECUTE, 0);

	SAFE_ACL_SET_FILE(TESTDIR, ACL_TYPE_ACCESS, acl);

	acl_free(acl);

	try_create_as(OTHER_UID, OTHER_GID, 0644, 0);

	cleanup_testfile();
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
