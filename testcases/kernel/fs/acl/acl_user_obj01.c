// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 IBM
 *
 * Original shell test by Kai Zhao (ltcd3@cn.ibm.com)
 * Converted to C by Sachin Sant <sachinp@linux.ibm.com>
 */

/*\
 * Test ACL_USER_OBJ permissions using direct xattr manipulation.
 *
 * Verify that owner permissions (ACL_USER_OBJ) correctly control access
 * to files and directories. The test validates that:
 *
 * - ACL_USER_OBJ permissions are applied directly as the owner bits
 * - Setting ACL_USER_OBJ=rwx via :manpage:`setxattr(2)` overrides
 *   a previous :manpage:`chmod(2)` restriction
 * - Owner permissions work independently of group and other permissions
 *
 * This test uses arbitrary UIDs without creating actual users, testing
 * only the kernel ACL implementation.
 */

#include "acl_lib.h"

#ifdef HAVE_SYS_XATTR_H

#define TEST_UID 1000
#define TEST_GID 1000

/*
 * Test permission bits deny access.
 * Owner should be denied file creation when directory mode is 0555.
 */
static void test_deny_by_mode(void)
{
	tst_res(TINFO, "Testing permission bits deny access");
	reset_test_path();

	SAFE_CHOWN(TESTDIR, TEST_UID, TEST_GID);
	SAFE_CHMOD(TESTDIR, 0555);

	try_create_as(TEST_UID, TEST_GID, 0644, EACCES);
}

/*
 * Test ACL_USER_OBJ grants access.
 * Setting ACL_USER_OBJ=rwx should restore owner write permission and
 * allow file creation after the restrictive mode baseline.
 */
static void test_grant_by_acl(void)
{
	struct acl *acl;

	tst_res(TINFO, "Testing ACL_USER_OBJ grants access");
	reset_test_path();

	SAFE_CHOWN(TESTDIR, TEST_UID, TEST_GID);
	SAFE_CHMOD(TESTDIR, 0555);

	acl = acl_init();

	acl_add_entry(acl, ACL_USER_OBJ,
		      ACL_READ | ACL_WRITE | ACL_EXECUTE, 0);
	acl_add_entry(acl, ACL_GROUP_OBJ, 0, 0);
	acl_add_entry(acl, ACL_OTHER, 0, 0);

	SAFE_ACL_SET_FILE(TESTDIR, ACL_TYPE_ACCESS, acl);

	acl_free(acl);

	try_create_as(TEST_UID, TEST_GID, 0644, 0);

	cleanup_testfile();
}

static void run(unsigned int n)
{
	switch (n) {
	case 0:
		test_deny_by_mode();
		break;
	case 1:
		test_grant_by_acl();
		break;
	}
}

static void cleanup(void)
{
	cleanup_test_paths();
}

static struct tst_test test = {
	.test = run,
	.tcnt = 2,
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
