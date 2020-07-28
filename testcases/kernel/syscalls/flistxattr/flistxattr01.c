// SPDX-License-Identifier: GPL-2.0-or-later
/*
*  Copyright (c) 2016 RT-RK Institute for Computer Based Systems
*  Author: Dejan Jovicevic <dejan.jovicevic@rt-rk.com>
*/

/*
* Test Name: verify_flistxattr01
*
* Description:
* The testcase checks the basic functionality of the flistxattr(2).
* flistxattr(2) retrieves the list of extended attribute names
* associated with the file itself in the filesystem.
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

#define SECURITY_KEY1	"security.ltptest1"
#define VALUE	"test"
#define VALUE_SIZE	(sizeof(VALUE) - 1)
#define KEY_SIZE    (sizeof(SECURITY_KEY1) - 1)
#define MNTPOINT        "mntpoint"
#define DIR_MODE        (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#define FILE_MODE       (S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID)
#define TESTFILE "mntpoint/flistxattr01testfile"
static const char *device = "/dev/vda";
static const char *fs_type = "ext4";

static int fd;

static int has_attribute(const char *list, int llen, const char *attr)
{
	int i;

	for (i = 0; i < llen; i += strlen(list + i) + 1) {
		if (!strcmp(list + i, attr))
			return 1;
	}
	return 0;
}

static void verify_flistxattr(void)
{
	char buf[64];

	TEST(flistxattr(fd, buf, sizeof(buf)));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "flistxattr() failed");
		return;
	}

	if (!has_attribute(buf, sizeof(buf), SECURITY_KEY1)) {
		tst_res(TFAIL, "missing attribute %s",
			 SECURITY_KEY1);
		return;
	}

	tst_res(TPASS, "flistxattr() succeeded");
}

static void setup(void)
{
	rmdir(MNTPOINT);
	SAFE_MKDIR(MNTPOINT, DIR_MODE);
	SAFE_MOUNT(device, MNTPOINT, fs_type, 0, "user_xattr");

	fd = SAFE_OPEN(TESTFILE, O_RDWR | O_CREAT, 0644);

	SAFE_FSETXATTR(fd, SECURITY_KEY1, VALUE, VALUE_SIZE, XATTR_CREATE);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
	remove(TESTFILE);
	SAFE_UMOUNT(MNTPOINT);
	SAFE_RMDIR(MNTPOINT);

}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.needs_root = 1,
	.test_all = verify_flistxattr,
	.setup = setup,
	.cleanup = cleanup,
};

#else
	TST_TEST_TCONF("<sys/xattr.h> does not exist.");
#endif /* HAVE_SYS_XATTR_H */
