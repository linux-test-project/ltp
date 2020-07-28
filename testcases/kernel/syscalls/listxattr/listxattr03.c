// SPDX-License-Identifier: GPL-2.0-or-later
/*
*  Copyright (c) 2016 RT-RK Institute for Computer Based Systems
*  Author: Dejan Jovicevic <dejan.jovicevic@rt-rk.com>
*/

/*
* Test Name: listxattr03
*
* Description:
* An empty buffer of size zero can return the current size of the list
* of extended attribute names, which can be used to estimate a suitable buffer.
*/

/* Currently xattr is not enabled while mounting root file system. Patch is
 * to mount root file system with xattr enabled and then use it for the test.
*/

#include <stdio.h>
#include "config.h"
#include <errno.h>
#include <sys/types.h>

#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif

#include "tst_test.h"

#ifdef HAVE_SYS_XATTR_H

#define SECURITY_KEY	"security.ltptest"
#define VALUE	"test"
#define VALUE_SIZE	(sizeof(VALUE) - 1)

#define MNTPOINT        "mntpoint"
#define DIR_MODE        (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#define FILE_MODE       (S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID)
#define TESTFILE1 "mntpoint/listxattr03testfile1"
#define TESTFILE2 "mntpoint/listxattr03testfile2"
static const char *device = "/dev/vda";
static const char *fs_type = "ext4";

static const char * const filename[] = {TESTFILE1,TESTFILE2};

static int check_suitable_buf(const char *name, long size)
{
	int n;
	char buf[size];

	n = listxattr(name, buf, sizeof(buf));

	return n != -1;
}

static void verify_listxattr(unsigned int n)
{
	const char *name = filename[n];

	TEST(listxattr(name, NULL, 0));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "listxattr() failed");
		return;
	}

	if (check_suitable_buf(name, TST_RET))
		tst_res(TPASS, "listxattr() succeed with suitable buffer");
	else
		tst_res(TFAIL, "listxattr() failed with small buffer");
}

static void setup(void)
{
	rmdir(MNTPOINT);
	SAFE_MKDIR(MNTPOINT, DIR_MODE);
	SAFE_MOUNT(device, MNTPOINT, fs_type, 0, "user_xattr");

	SAFE_TOUCH(filename[0], 0644, NULL);

	SAFE_TOUCH(filename[1], 0644, NULL);

	SAFE_SETXATTR(filename[1], SECURITY_KEY, VALUE, VALUE_SIZE, XATTR_CREATE);
}

static void cleanup(void)
{
	remove(TESTFILE1);
	remove(TESTFILE2);
	SAFE_UMOUNT(MNTPOINT);
	SAFE_RMDIR(MNTPOINT);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.needs_root = 1,
	.test = verify_listxattr,
	.tcnt = ARRAY_SIZE(filename),
	.setup = setup,
	.cleanup = cleanup,
};

#else /* HAVE_SYS_XATTR_H */
	TST_TEST_TCONF("<sys/xattr.h> does not exist.");
#endif
