// SPDX-License-Identifier: GPL-2.0-or-later
/*
* Copyright (c) 2016 Fujitsu Ltd.
* Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
*/

/*
* Test Name: llistxattr03
*
* Description:
* llistxattr is identical to listxattr. an empty buffer of size zero
* can return the current size of the list of extended attribute names,
* which can be used to estimate a suitable buffer.
*/

#include "config.h"
#include <errno.h>
#include <sys/types.h>

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#include "tst_test.h"

#ifdef HAVE_SYS_XATTR_H

#define SECURITY_KEY	"security.ltptest"
#define VALUE           "test"
#define VALUE_SIZE      (sizeof(VALUE) - 1)
#define MNTPOINT       "mntpoint"

static const char *filename[] = {"mntpoint/testfile1", "mntpoint/testfile2"};

static int check_suitable_buf(const char *name, long size)
{
	int n;
	char buf[size];

	n = llistxattr(name, buf, sizeof(buf));

	return n != -1;
}

static void verify_llistxattr(unsigned int n)
{
	const char *name = filename[n];

	TEST(llistxattr(name, NULL, 0));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "llistxattr() failed");
		return;
	}

	if (check_suitable_buf(name, TST_RET))
		tst_res(TPASS, "llistxattr() succeed with suitable buffer");
	else
		tst_res(TFAIL, "llistxattr() failed with small buffer");
}

static void setup(void)
{
       SAFE_MKDIR(MNTPOINT, 0644);
       SAFE_MOUNT("/dev/vda", MNTPOINT, "ext4", 0, "user_xattr");

	SAFE_TOUCH(filename[0], 0644, NULL);

	SAFE_TOUCH(filename[1], 0644, NULL);

	SAFE_LSETXATTR(filename[1], SECURITY_KEY, VALUE, VALUE_SIZE, XATTR_CREATE);
}

static void cleanup(void)
{
       remove(filename[0]);
       remove(filename[1]);
       SAFE_UMOUNT(MNTPOINT);
       SAFE_RMDIR(MNTPOINT);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.needs_root = 1,
	.test = verify_llistxattr,
	.tcnt = ARRAY_SIZE(filename),
	.setup = setup,
       .cleanup = cleanup,
};

#else /* HAVE_SYS_XATTR_H */
	TST_TEST_TCONF("<sys/xattr.h> does not exist.");
#endif
