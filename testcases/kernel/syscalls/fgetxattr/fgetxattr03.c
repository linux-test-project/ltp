// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Linaro Limited. All rights reserved.
 * Author: Rafael David Tinoco <rafael.tinoco@linaro.org>
 */

/*
 * An empty buffer of size zero can be passed into fgetxattr(2) to return
 * the current size of the named extended attribute.
 */

#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif
#include "tst_test.h"

#ifdef HAVE_SYS_XATTR_H
#define XATTR_TEST_KEY "user.testkey"
#define XATTR_TEST_VALUE "this is a test value"
#define XATTR_TEST_VALUE_SIZE 20
#define FILENAME "mntpoint/fgetxattr03testfile"
#define MNTPOINT       "mntpoint"
#define DIR_MODE        (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
const char *device = "/dev/vda";
static const char *fs_type = "ext4";

static int fd = -1;

static void verify_fgetxattr(void)
{
	TEST(fgetxattr(fd, XATTR_TEST_KEY, NULL, 0));

	if (TST_RET == XATTR_TEST_VALUE_SIZE) {
		tst_res(TPASS, "fgetxattr(2) returned correct value");
		return;
	}

	tst_res(TFAIL | TTERRNO, "fgetxattr(2) failed with %li", TST_RET);
}

static void setup(void)
{
	rmdir(MNTPOINT);
	SAFE_MKDIR(MNTPOINT, DIR_MODE);
	SAFE_MOUNT(device, MNTPOINT, fs_type, 0, "user_xattr");

	SAFE_TOUCH(FILENAME, 0644, NULL);
	fd = SAFE_OPEN(FILENAME, O_RDONLY, NULL);

	SAFE_FSETXATTR(fd, XATTR_TEST_KEY, XATTR_TEST_VALUE,
			XATTR_TEST_VALUE_SIZE, XATTR_CREATE);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
	remove(FILENAME);
	SAFE_UMOUNT(MNTPOINT);
	SAFE_RMDIR(MNTPOINT);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = verify_fgetxattr,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};

#else /* HAVE_SYS_XATTR_H */
TST_TEST_TCONF("<sys/xattr.h> does not exist");
#endif

