// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test reproducer for a bug introduced in 8b0ba61df5a1 ("fs/xattr.c: fix
 * simple_xattr_list to always include security.* xattrs") and fixed in
 * 800d0b9b6a8b (fs/xattr.c: fix simple_xattr_list()).
 *
 * Bug can be reproduced when SELinux and ACL are activated on inodes as
 * following:
 *
 *     $ touch testfile
 *     $ setfacl -m u:myuser:rwx testfile
 *     $ getfattr -dm. /tmp/testfile
 *     Segmentation fault (core dumped)
 *
 * The reason why this happens is that simple_xattr_list() always includes
 * security.* xattrs without resetting error flag after
 * security_inode_listsecurity(). This results into an incorrect length of the
 * returned xattr name if POSIX ACL is also applied on the inode.
 */

#include "config.h"
#include "tst_test.h"

#if defined(HAVE_SYS_XATTR_H) && defined(HAVE_LIBACL)

#include <sys/acl.h>
#include <sys/xattr.h>

#define ACL_PERM        "u::rw-,u:root:rwx,g::r--,o::r--,m::rwx"
#define TEST_FILE       "test.bin"

static acl_t acl;

static void verify_xattr(const int size)
{
	char buf[size];

	memset(buf, 0, sizeof(buf));

	if (listxattr(TEST_FILE, buf, size) == -1) {
		if (errno != ERANGE)
			tst_brk(TBROK | TERRNO, "listxattr() error");

		tst_res(TFAIL, "listxattr() failed to read attributes length: ERANGE");
		return;
	}

	tst_res(TPASS, "listxattr() correctly read attributes length");
}

static void run(void)
{
	int size;

	size = listxattr(TEST_FILE, NULL, 0);
	if (size == -1)
		tst_brk(TBROK | TERRNO, "listxattr() error");

	verify_xattr(size);
}

static void setup(void)
{
	int res;

	if (!tst_lsm_enabled("selinux") && !tst_lsm_enabled("smack"))
		tst_brk(TCONF, "There are no LSM(s) implementing xattr");

	SAFE_TOUCH(TEST_FILE, 0644, NULL);

	acl = acl_from_text(ACL_PERM);
	if (!acl)
		tst_brk(TBROK | TERRNO, "acl_from_text() failed");

	res = acl_set_file(TEST_FILE, ACL_TYPE_ACCESS, acl);
	if (res == -1) {
		if (errno == EOPNOTSUPP)
			tst_brk(TCONF | TERRNO, "acl_set_file()");

		tst_brk(TBROK | TERRNO, "acl_set_file(%s) failed", TEST_FILE);
	}
}

static void cleanup(void)
{
	if (acl)
		acl_free(acl);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "800d0b9b6a8b"},
		{}
	}
};

#else /* HAVE_SYS_XATTR_H && HAVE_LIBACL */
	TST_TEST_TCONF("<sys/xattr.h> or <sys/acl.h> does not exist");
#endif
