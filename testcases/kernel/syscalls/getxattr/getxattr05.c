// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*
 * Description:
 * 1) Witout a user namespace, getxattr(2) should get same data when
 *    acquiring the value of system.posix_acl_access twice.
 * 2) With/Without mapped root UID in a user namespaces, getxattr(2) should
 *    get same data when acquiring the value of system.posix_acl_access twice.
 *
 * This issue included by getxattr05 has been fixed in kernel:
 * '82c9a927bc5d ("getxattr: use correct xattr length")'
 */

#define _GNU_SOURCE
#include "config.h"
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif

#ifdef HAVE_LIBACL
# include <sys/acl.h>
#endif

#include "tst_test.h"
#include "lapi/sched.h"

#if defined(HAVE_SYS_XATTR_H) && defined(HAVE_LIBACL)

#define TEST_FILE	"testfile"
#define SELF_USERNS	"/proc/self/ns/user"
#define MAX_USERNS	"/proc/sys/user/max_user_namespaces"
#define UID_MAP	"/proc/self/uid_map"

static acl_t acl;
static int orig_max_userns = -1;
static int user_ns_supported = 1;

static struct tcase {
	/* 0: without userns, 1: with userns */
	int set_userns;
	/* 0: don't map root UID in userns, 1: map root UID in userns */
	int map_root;
} tcases[] = {
	{0, 0},
	{1, 0},
	{1, 1},
};

static void verify_getxattr(void)
{
	ssize_t i, res1, res2;
	char buf1[128], buf2[132];

	res1 = SAFE_GETXATTR(TEST_FILE, "system.posix_acl_access",
			     buf1, sizeof(buf1));
	res2 = SAFE_GETXATTR(TEST_FILE, "system.posix_acl_access",
			     buf2, sizeof(buf2));

	if (res1 != res2) {
		tst_res(TFAIL, "Return different sizes when acquiring "
			"the value of system.posix_acl_access twice");
		return;
	}

	for (i = 0; i < res1; i++) {
		if (buf1[i] != buf2[i])
			break;
	}

	if (i < res1) {
		tst_res(TFAIL, "Got different data(%02x != %02x) at %ld",
			buf1[i], buf2[i], i);
		return;
	}

	tst_res(TPASS, "Got same data when acquiring the value of "
		"system.posix_acl_access twice");
}

static void do_unshare(int map_root)
{
	int res;

	/* unshare() should support CLONE_NEWUSER flag since Linux 3.8 */
	res = unshare(CLONE_NEWUSER);
	if (res == -1)
		tst_brk(TFAIL | TERRNO, "unshare(CLONE_NEWUSER) failed");

	if (map_root) {
		/* uid_map file should exist since Linux 3.8 because
		 * it is available on Linux 3.5
		 */
		SAFE_ACCESS(UID_MAP, F_OK);

		SAFE_FILE_PRINTF(UID_MAP, "%d %d %d", 0, 0, 1);
	}
}

static void do_getxattr(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	pid_t pid;

	if (tc->set_userns && !user_ns_supported) {
		tst_res(TCONF, "user namespace not available");
		return;
	}

	pid = SAFE_FORK();
	if (!pid) {
		if (tc->set_userns)
			do_unshare(tc->map_root);

		verify_getxattr();
		exit(0);
	}

	tst_reap_children();
}

static void setup(void)
{
	const char *acl_text = "u::rw-,u:root:rwx,g::r--,o::r--,m::rwx";
	int res;

	SAFE_TOUCH(TEST_FILE, 0644, NULL);

	acl = acl_from_text(acl_text);
	if (!acl)
		tst_brk(TBROK | TERRNO, "acl_from_text() failed");

	res = acl_set_file(TEST_FILE, ACL_TYPE_ACCESS, acl);
	if (res == -1) {
		if (errno == EOPNOTSUPP)
			tst_brk(TCONF | TERRNO, "acl_set_file()");

		tst_brk(TBROK | TERRNO, "acl_set_file(%s) failed", TEST_FILE);
	}

	/* The default value of max_user_namespaces is set to 0 on some distros,
	 * We need to change the default value to call unshare().
	 */
	if (access(SELF_USERNS, F_OK) != 0) {
		user_ns_supported = 0;
	} else if (!access(MAX_USERNS, F_OK)) {
		SAFE_FILE_SCANF(MAX_USERNS, "%d", &orig_max_userns);
		SAFE_FILE_PRINTF(MAX_USERNS, "%d", 10);
	}

}

static void cleanup(void)
{
	if (orig_max_userns != -1)
		SAFE_FILE_PRINTF(MAX_USERNS, "%d", orig_max_userns);

	if (acl)
		acl_free(acl);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.needs_root = 1,
	.forks_child = 1,
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = do_getxattr,
	.min_kver = "3.8",
};

#else /* HAVE_SYS_XATTR_H && HAVE_LIBACL*/
	TST_TEST_TCONF("<sys/xattr.h> or <sys/acl.h> does not exist.");
#endif
