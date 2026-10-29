// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 IBM
 *
 * Original shell test by Kai Zhao (ltcd3@cn.ibm.com)
 * Converted to C by Sachin Sant <sachinp@linux.ibm.com>
 */

/*\
 * Test ACL operations on symlinks using direct xattr manipulation.
 *
 * Verify that ACL operations on symlinks follow the symlink to the target
 * file. When setting or getting ACLs through a symlink path, the operation
 * should affect the target file, not the symlink itself.
 *
 * Note: Some filesystems may not support ACLs on the target file and will
 * return EOPNOTSUPP, which is treated as TCONF (test not applicable).
 *
 * This test uses direct xattr manipulation without creating actual users,
 * testing only the kernel ACL implementation.
 *
 * [Algorithm]
 *
 * - Create a regular file with mode 0600 (rw-------)
 * - Create a symlink pointing to the file
 * - Set a distinct ACL through the symlink path (rwxrw----)
 * - Verify the ACL was set on the target file by reading it directly
 * - Get ACL through the symlink path
 * - Verify both ACLs match and differ from the initial 0600 mode
 */

#include "acl_lib.h"

#ifdef HAVE_SYS_XATTR_H

static void run(void)
{
	struct acl *acl, *target_acl = NULL, *symlink_acl = NULL;
	struct acl_entry *user_obj, *group_obj, *other;
	struct acl_entry *t, *s;
	int fd = -1;
	int match, i;

	tst_res(TINFO, "Testing ACL operations on symlinks");
	reset_test_path();

	fd = SAFE_OPEN(TESTFILE, O_CREAT | O_WRONLY, 0600);
	SAFE_CLOSE(fd);

	SAFE_SYMLINK("testfile", TESTSYMLINK);

	acl = acl_init();

	acl_add_entry(acl, ACL_USER_OBJ,
		      ACL_READ | ACL_WRITE | ACL_EXECUTE, 0);
	acl_add_entry(acl, ACL_GROUP_OBJ, ACL_READ | ACL_WRITE, 0);
	acl_add_entry(acl, ACL_OTHER, 0, 0);

	SAFE_ACL_SET_FILE(TESTSYMLINK, ACL_TYPE_ACCESS, acl);

	acl_free(acl);

	/* Verify ACL was actually set on target file with expected values */
	target_acl = acl_get_file(TESTFILE, ACL_TYPE_ACCESS);
	if (!target_acl)
		tst_brk(TBROK | TERRNO, "acl_get_file on target file failed");

	/* Verify expected ACL entries: USER_OBJ=rwx, GROUP_OBJ=rw, OTHER=--- */
	if (target_acl->count != 3) {
		tst_res(TFAIL, "Expected 3 ACL entries, got %d",
			target_acl->count);
		return;
	}

	user_obj = acl_find_entry(target_acl, ACL_USER_OBJ, 0);
	group_obj = acl_find_entry(target_acl, ACL_GROUP_OBJ, 0);
	other = acl_find_entry(target_acl, ACL_OTHER, 0);

	if (!user_obj || !group_obj || !other) {
		tst_res(TFAIL, "Missing required ACL entries");
		return;
	}

	if (user_obj->perm != (ACL_READ | ACL_WRITE | ACL_EXECUTE)) {
		tst_res(TFAIL, "USER_OBJ has wrong permissions: %o (expected rwx)",
			user_obj->perm);
		return;
	}

	if (group_obj->perm != (ACL_READ | ACL_WRITE)) {
		tst_res(TFAIL, "GROUP_OBJ has wrong permissions: %o (expected rw-)",
			group_obj->perm);
		return;
	}

	if (other->perm != 0) {
		tst_res(TFAIL, "OTHER has wrong permissions: %o (expected ---)",
			other->perm);
		return;
	}

	/* Now verify that reading via symlink gives the same result */
	symlink_acl = acl_get_file(TESTSYMLINK, ACL_TYPE_ACCESS);
	if (!symlink_acl)
		tst_brk(TBROK | TERRNO, "acl_get_file on symlink failed");

	/* Compare ACLs */
	match = 1;
	if (target_acl->count != symlink_acl->count) {
		match = 0;
	} else {
		for (i = 0; i < target_acl->count; i++) {
			t = &target_acl->entries[i];
			s = &symlink_acl->entries[i];

			if (t->tag != s->tag || t->perm != s->perm ||
			    t->id != s->id) {
				match = 0;
				break;
			}
		}
	}

	acl_free(symlink_acl);
	acl_free(target_acl);

	if (!match) {
		tst_res(TFAIL,
			"ACL via symlink differs from ACL on target file");
		return;
	}

	tst_res(TPASS,
		"ACL set via symlink was applied to target file (rwxrw----)");
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
