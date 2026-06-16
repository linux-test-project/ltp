// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 IBM
 *
 * Original shell test by Kai Zhao (ltcd3@cn.ibm.com)
 * Converted to C by Sachin Sant <sachinp@linux.ibm.com>
 */

/*\
 * Test :manpage:`chmod(2)` and :manpage:`chown(2)` interaction
 * with ACLs using direct xattr manipulation.
 *
 * Verify that standard file operations interact correctly
 * with ACLs:
 *
 * - :manpage:`chmod(2)` should update ACL_USER_OBJ, ACL_GROUP_OBJ,
 *   and ACL_OTHER entries
 * - :manpage:`chown(2)` should change file owner/group without
 *   affecting ACL entries
 * - ACL permissions should be preserved after ownership changes
 *
 * This test uses arbitrary UIDs without creating actual users, testing
 * only the kernel ACL implementation.
 *
 * [Algorithm]
 *
 * Test 1 - :manpage:`chmod(2)` interaction:
 *
 * - Create file with read-only ACL entries
 * - Use :manpage:`chmod(2)` to set permissions to 0777
 * - Verify ACL entries are updated to rwx for user, group, and other
 *
 * Test 2 - :manpage:`chown(2)` interaction:
 *
 * - Create file with specific ACL entries (rw for user, r for group/other)
 * - Use :manpage:`chown(2)` to change owner and group
 * - Verify ownership changed correctly
 * - Verify ACL entries preserved their permissions after
 *   :manpage:`chown(2)`
 */

#include "acl_lib.h"

#ifdef HAVE_SYS_XATTR_H

#define TEST_UID 1000
#define TEST_GID 1000
#define USER2_UID 2000
#define USER2_GID 2000

/*
 * Test chmod interaction with ACLs.
 * chmod should update ACL_USER_OBJ, ACL_GROUP_OBJ, and ACL_OTHER.
 */
static void test_chmod_acl(void)
{
	struct acl *acl;
	int fd = -1;
	int user_ok = 0, group_ok = 0, other_ok = 0;

	tst_res(TINFO, "Testing chmod interaction with ACLs");
	reset_test_path();

	fd = SAFE_OPEN(TESTFILE, O_CREAT | O_WRONLY, 0644);
	SAFE_CLOSE(fd);

	acl = acl_init();

	acl_add_entry(acl, ACL_USER_OBJ, ACL_READ, 0);
	acl_add_entry(acl, ACL_GROUP_OBJ, ACL_READ, 0);
	acl_add_entry(acl, ACL_OTHER, ACL_READ, 0);

	SAFE_ACL_SET_FILE(TESTFILE, ACL_TYPE_ACCESS, acl);

	acl_free(acl);

	SAFE_CHMOD(TESTFILE, 0777);

	acl = acl_get_file(TESTFILE, ACL_TYPE_ACCESS);
	if (!acl)
		tst_brk(TBROK | TERRNO, "acl_get_file failed");

	/* Check if all entries have rwx permissions */
	for (int i = 0; i < acl->count; i++) {
		struct acl_entry *entry = &acl->entries[i];

		if (entry->tag == ACL_USER_OBJ) {
			if (acl_entry_has_rwx(entry))
				user_ok = 1;
		} else if (entry->tag == ACL_GROUP_OBJ) {
			if (acl_entry_has_rwx(entry))
				group_ok = 1;
		} else if (entry->tag == ACL_OTHER) {
			if (acl_entry_has_rwx(entry))
				other_ok = 1;
		}
	}

	acl_free(acl);

	if (user_ok && group_ok && other_ok)
		tst_res(TPASS, "chmod correctly updated ACL entries");
	else
		tst_res(TFAIL, "chmod did not update ACL entries correctly");
}

/*
 * Test chown interaction with ACLs.
 * chown should change file owner and group without affecting ACL entries.
 */
static void test_chown_acl(void)
{
	struct acl *acl;
	struct stat st;
	int fd = -1;
	int found_user_obj = 0, found_group_obj = 0, found_other = 0;

	tst_res(TINFO, "Testing chown interaction with ACLs");
	reset_test_path();

	fd = SAFE_OPEN(TESTFILE, O_CREAT | O_WRONLY, 0644);
	SAFE_CLOSE(fd);

	acl = acl_init();

	acl_add_entry(acl, ACL_USER_OBJ, ACL_READ | ACL_WRITE, 0);
	acl_add_entry(acl, ACL_GROUP_OBJ, ACL_READ, 0);
	acl_add_entry(acl, ACL_OTHER, ACL_READ, 0);

	SAFE_ACL_SET_FILE(TESTFILE, ACL_TYPE_ACCESS, acl);

	acl_free(acl);

	SAFE_CHOWN(TESTFILE, USER2_UID, USER2_GID);

	SAFE_STAT(TESTFILE, &st);

	if (st.st_uid != USER2_UID || st.st_gid != USER2_GID) {
		tst_res(TFAIL, "chown did not change owner/group correctly");
		return;
	}

	/* Verify ACL entries are preserved after chown */
	acl = acl_get_file(TESTFILE, ACL_TYPE_ACCESS);
	if (!acl)
		tst_brk(TBROK | TERRNO, "acl_get_file failed");

	for (int i = 0; i < acl->count; i++) {
		struct acl_entry *entry = &acl->entries[i];

		if (entry->tag == ACL_USER_OBJ) {
			found_user_obj = 1;
			if (!acl_entry_has_perm(entry, ACL_READ) ||
			    !acl_entry_has_perm(entry, ACL_WRITE)) {
				acl_free(acl);
				tst_res(TFAIL,
					"ACL_USER_OBJ perms changed after chown");
				return;
			}
		} else if (entry->tag == ACL_GROUP_OBJ) {
			found_group_obj = 1;
			if (!acl_entry_has_perm(entry, ACL_READ) ||
			    acl_entry_has_perm(entry, ACL_WRITE)) {
				acl_free(acl);
				tst_res(TFAIL,
					"ACL_GROUP_OBJ perms changed after chown");
				return;
			}
		} else if (entry->tag == ACL_OTHER) {
			found_other = 1;
			if (!acl_entry_has_perm(entry, ACL_READ) ||
			    acl_entry_has_perm(entry, ACL_WRITE)) {
				acl_free(acl);
				tst_res(TFAIL,
					"ACL_OTHER perms changed after chown");
				return;
			}
		}
	}

	acl_free(acl);

	if (!found_user_obj || !found_group_obj || !found_other) {
		tst_res(TFAIL, "ACL entries missing after chown");
		return;
	}

	tst_res(TPASS, "chown preserved ACL entries correctly");
}

static void cleanup(void)
{
	cleanup_test_paths();
}

static void run(unsigned int n)
{
	switch (n) {
	case 0:
		test_chmod_acl();
		break;
	case 1:
		test_chown_acl();
		break;
	}
}

static struct tst_test test = {
	.test = run,
	.tcnt = 2,
	.cleanup = cleanup,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
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
