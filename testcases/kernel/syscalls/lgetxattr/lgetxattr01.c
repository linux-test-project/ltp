// SPDX-License-Identifier: GPL-2.0-or-later
/*
* Copyright (c) 2016 Fujitsu Ltd.
* Author: Jinbao Huang <huangjb.jy@cn.fujitsu.com>
*/

/*
* Test Name: lgetxattr01
*
* Description:
* The testcase checks the basic functionality of the lgetxattr(2).
* In the case of a symbolic link, we only get the value of the
* extended attribute related to the link itself by name.
*
*/

#include <stdio.h>
#include "config.h"
#include <errno.h>
#include <sys/types.h>
#include <string.h>

#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif

#include "tst_test.h"

#ifdef HAVE_SYS_XATTR_H

#define SECURITY_KEY1   "security.ltptest1"
#define SECURITY_KEY2   "security.ltptest2"
#define VALUE1   "test1"
#define VALUE2   "test2"
#define MNTPOINT  "mntpoint"
#define FILENAME  "mntpoint/lgetxattr01testfile"
#define SYMLINK  "mntpoint/lgetxattr01symlink"
#define DIR_MODE  (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)

const char *device = "/dev/vda";
static const char *fs_type = "ext4";

static void set_xattr(char *path, char *key, void *value, size_t size)
{
	int res;

	res = lsetxattr(path, key, value, size, XATTR_CREATE);
	if (res == -1) {
		if (errno == ENOTSUP) {
			tst_brk(TCONF,
				"no xattr support in fs or mounted "
				"without user_xattr option");
		} else {
			tst_brk(TBROK | TERRNO, "lsetxattr(%s) failed", key);
		}
	}
}

static void setup(void)
{
	rmdir(MNTPOINT);
	SAFE_MKDIR(MNTPOINT, DIR_MODE);
	SAFE_MOUNT(device, MNTPOINT, fs_type, 0, "user_xattr");
	SAFE_TOUCH(FILENAME, 0644, NULL);
	SAFE_SYMLINK(FILENAME, SYMLINK);

	set_xattr(FILENAME, SECURITY_KEY1, VALUE1, strlen(VALUE1));
	set_xattr(SYMLINK, SECURITY_KEY2, VALUE2, strlen(VALUE2));
}

static void cleanup(void)
{
	remove(SYMLINK);
	remove(FILENAME);
	SAFE_UMOUNT(MNTPOINT);
	SAFE_RMDIR(MNTPOINT);
}
static void verify_lgetxattr(void)
{
	int size = 64;
	char buf[size];

	TEST(lgetxattr(SYMLINK, SECURITY_KEY2, buf, size));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "lgetxattr() failed");
		goto next;
	}

	if (TST_RET != strlen(VALUE2)) {
		tst_res(TFAIL, "lgetxattr() got unexpected value size");
		goto next;
	}

	if (!strncmp(buf, VALUE2, TST_RET))
		tst_res(TPASS, "lgetxattr() got expected value");
	else
		tst_res(TFAIL, "lgetxattr() got unexpected value");

next:
	TEST(lgetxattr(SYMLINK, SECURITY_KEY1, buf, size));

	if (TST_RET != -1) {
		tst_res(TFAIL, "lgetxattr() succeeded unexpectedly");
		return;
	}

	if (TST_ERR == ENODATA) {
		tst_res(TPASS | TTERRNO, "lgetxattr() failed as expected");
	} else {
		tst_res(TFAIL | TTERRNO, "lgetxattr() failed unexpectedly,"
			"expected %s", tst_strerrno(ENODATA));
	}
}

static struct tst_test test = {
	.needs_root = 1,
	.test_all = verify_lgetxattr,
	.cleanup = cleanup,
	.setup = setup
};

#else
	TST_TEST_TCONF("<sys/xattr.h> does not exist.");
#endif /* HAVE_SYS_XATTR_H */
