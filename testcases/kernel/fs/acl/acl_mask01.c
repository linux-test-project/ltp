// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 IBM
 *
 * Original shell test by Kai Zhao (ltcd3@cn.ibm.com)
 * Converted to C by Sachin Sant <sachinp@linux.ibm.com>
 */

/*\
 * Test ACL mask interaction with named users and groups using direct xattr
 * manipulation.
 *
 * Verify that ACL_MASK correctly restricts permissions for:
 * - ACL_USER (named user) entries
 * - ACL_GROUP (named group) entries
 * - ACL_GROUP_OBJ (group owner) entries
 *
 * The mask acts as an upper bound on permissions for these entry types.
 * Even if an entry grants full permissions, the mask can restrict them.
 * ACL_USER_OBJ and ACL_OTHER are not affected by the mask.
 *
 * This test uses arbitrary UIDs without creating actual users, testing
 * only the kernel ACL implementation.
 *
 * [Algorithm]
 *
 * The test uses a parametrized approach with three test cases, one for each
 * entry type (ACL_USER, ACL_GROUP, ACL_GROUP_OBJ). For each test case:
 *
 * - Set up ACL with full permissions (rwx) for the entry type
 * - Set mask to allow full permissions (rwx)
 * - Verify access is granted
 * - Clear mask permissions (---)
 * - Verify access is denied despite entry having full permissions
 */

#include "acl_lib.h"

#ifdef HAVE_SYS_XATTR_H

#define TEST_UID 1000
#define TEST_GID 1000
#define USER2_UID 2000
#define USER2_GID 2000
#define USER3_UID 3000
#define USER3_GID 3000

static struct tcase {
	uint16_t tag;
	uid_t uid;
	gid_t gid;
	uid_t owner_uid;
	gid_t owner_gid;
	const char *desc;
} tcases[] = {
	{ACL_USER, USER3_UID, USER3_GID, TEST_UID, TEST_GID,
	 "ACL_USER with mask"},
	{ACL_GROUP, USER2_UID, USER2_GID, TEST_UID, TEST_GID,
	 "ACL_GROUP with mask"},
	{ACL_GROUP_OBJ, USER2_UID, USER2_GID, TEST_UID, USER2_GID,
	 "ACL_GROUP_OBJ with mask"},
};

/*
 * Test ACL mask interaction with different entry types.
 * The mask acts as an upper bound on permissions for ACL_USER,
 * ACL_GROUP, and ACL_GROUP_OBJ entries.
 */
static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	struct acl *acl;

	tst_res(TINFO, "Testing %s", tc->desc);
	reset_test_path();

	SAFE_CHOWN(TESTDIR, tc->owner_uid, tc->owner_gid);
	SAFE_CHMOD(TESTDIR, 0550);

	acl = acl_init();

	/* Set up ACL with full permissions for the entry type */
	acl_add_entry(acl, ACL_USER_OBJ,
		      ACL_READ | ACL_WRITE | ACL_EXECUTE, 0);

	if (tc->tag == ACL_USER) {
		acl_add_entry(acl, ACL_USER,
			      ACL_READ | ACL_WRITE | ACL_EXECUTE, tc->uid);
		acl_add_entry(acl, ACL_GROUP_OBJ, 0, 0);
	} else if (tc->tag == ACL_GROUP) {
		acl_add_entry(acl, ACL_GROUP_OBJ, 0, 0);
		acl_add_entry(acl, ACL_GROUP,
			      ACL_READ | ACL_WRITE | ACL_EXECUTE, tc->gid);
	} else {
		acl_add_entry(acl, ACL_GROUP_OBJ,
			      ACL_READ | ACL_WRITE | ACL_EXECUTE, 0);
	}

	acl_add_entry(acl, ACL_MASK,
		      ACL_READ | ACL_WRITE | ACL_EXECUTE, 0);
	acl_add_entry(acl, ACL_OTHER, 0, 0);

	SAFE_ACL_SET_FILE(TESTDIR, ACL_TYPE_ACCESS, acl);

	/* Verify access is granted with mask=rwx */
	try_create_as(tc->uid, tc->gid, 0644, 0);

	cleanup_testfile();

	acl_set_mask_perms(acl, 0);

	SAFE_ACL_SET_FILE(TESTDIR, ACL_TYPE_ACCESS, acl);

	acl_free(acl);

	/* Verify access is denied with mask=--- */
	try_create_as(tc->uid, tc->gid, 0644, EACCES);

	/* Restore ownership for ACL_GROUP_OBJ case */
	if (tc->tag == ACL_GROUP_OBJ)
		SAFE_CHOWN(TESTDIR, TEST_UID, TEST_GID);
}

static void cleanup(void)
{
	cleanup_test_paths();
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
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
