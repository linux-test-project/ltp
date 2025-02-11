// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Linaro Limited. All rights reserved.
 * Copyright (c) Linux Test Project, 2018-2023
 * Author: Rafael David Tinoco <rafael.tinoco@linaro.org>
 */

/*\
 * Verify basic fsetxattr(2) syscall functionality:
 *
 * - Set attribute to a regular file, fsetxattr(2) should succeed.
 * - Set attribute to a directory, fsetxattr(2) should succeed.
 * - Set attribute to a symlink which points to the regular file,
 *   fsetxattr(2) should return -1 and set errno to EEXIST.
 * - Set attribute to a FIFO, fsetxattr(2) should return -1 and set
 *   errno to EPERM.
 * - Set attribute to a char special file, fsetxattr(2) should
 *   return -1 and set errno to EPERM.
 * - Set attribute to a block special file, fsetxattr(2) should
 *   return -1 and set errno to EPERM.
 * - Set attribute to a UNIX domain socket, fsetxattr(2) should
 *   return -1 and set errno to EPERM.
 */

/*
 * In the user.* namespace, only regular files and directories can
 * have extended attributes. Otherwise fsetxattr(2) will return -1
 * and set errno to EPERM.
 */

#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif
#include "tst_test.h"

#ifdef HAVE_SYS_XATTR_H
#define XATTR_TEST_KEY "user.testkey"
#define XATTR_TEST_VALUE "this is a test value"
#define XATTR_TEST_VALUE_SIZE 20

#define MNTPOINT "mntpoint"
#define OFFSET    11
#define FILENAME "fsetxattr02testfile"
#define DIRNAME  "fsetxattr02testdir"
#define SYMLINK  "fsetxattr02symlink"
#define FIFO     MNTPOINT"/fsetxattr02fifo"
#define CHR      MNTPOINT"/fsetxattr02chr"
#define BLK      MNTPOINT"/fsetxattr02blk"
#define SOCK     "fsetxattr02sock"

struct test_case {
	char *fname;
	int fd;
	int fflags;
	char *key;
	char *value;
	size_t size;
	int flags;
	int exp_err;
	int issocket;
	int needskeyset;
};
static struct test_case tc[] = {
	{			/* case 00, set attr to reg */
	 .fname = FILENAME,
	 .fflags = O_RDONLY,
	 .key = XATTR_TEST_KEY,
	 .value = XATTR_TEST_VALUE,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = 0,
	 },
	{			/* case 01, set attr to dir */
	 .fname = DIRNAME,
	 .fflags = O_RDONLY,
	 .key = XATTR_TEST_KEY,
	 .value = XATTR_TEST_VALUE,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = 0,
	 },
	{			/* case 02, set attr to symlink */
	 .fname = SYMLINK,
	 .fflags = O_RDONLY,
	 .key = XATTR_TEST_KEY,
	 .value = XATTR_TEST_VALUE,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = EEXIST,
	 .needskeyset = 1,
	 },
	{			/* case 03, set attr to fifo */
	 .fname = FIFO,
	 .fflags = (O_RDONLY | O_NONBLOCK),
	 .key = XATTR_TEST_KEY,
	 .value = XATTR_TEST_VALUE,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = EPERM,
	 },
	{			/* case 04, set attr to character special */
	 .fname = CHR,
	 .fflags = O_RDONLY,
	 .key = XATTR_TEST_KEY,
	 .value = XATTR_TEST_VALUE,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = EPERM,
	 },
	{			/* case 05, set attr to block special */
	 .fname = BLK,
	 .fflags = O_RDONLY,
	 .key = XATTR_TEST_KEY,
	 .value = XATTR_TEST_VALUE,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = EPERM,
	 },
	{			/* case 06, set attr to socket */
	 .fname = SOCK,
	 .fflags = O_RDONLY,
	 .key = XATTR_TEST_KEY,
	 .value = XATTR_TEST_VALUE,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = EPERM,
	 .issocket = 1,
	 },
};

static void verify_fsetxattr(unsigned int i)
{
	const char *fname = strstr(tc[i].fname, "fsetxattr02") + OFFSET;

	/* some tests might require existing keys for each iteration */
	if (tc[i].needskeyset) {
		SAFE_FSETXATTR(tc[i].fd, tc[i].key, tc[i].value, tc[i].size,
				XATTR_CREATE);
	}

	TEST(fsetxattr(tc[i].fd, tc[i].key, tc[i].value, tc[i].size,
			tc[i].flags));

	if (TST_RET == -1 && TST_ERR == EOPNOTSUPP)
		tst_brk(TCONF, "fsetxattr(2) not supported");

	/* success */

	if (!tc[i].exp_err) {
		if (TST_RET) {
			tst_res(TFAIL | TTERRNO,
				"fsetxattr(2) on %s failed with %li",
				fname, TST_RET);
			return;
		}

		/* this is needed for subsequent iterations */
		SAFE_FREMOVEXATTR(tc[i].fd, tc[i].key);

		tst_res(TPASS, "fsetxattr(2) on %s passed", fname);
		return;
	}

	if (TST_RET == 0) {
		tst_res(TFAIL, "fsetxattr(2) on %s passed unexpectedly", fname);
		return;
	}

	/* fail */

	if (tc[i].exp_err != TST_ERR) {
		tst_res(TFAIL | TTERRNO,
				"fsetxattr(2) on %s should have failed with %s",
				fname, tst_strerrno(tc[i].exp_err));
		return;
	}

	/* key might have been added AND test might have failed, remove it */
	if (tc[i].needskeyset)
		SAFE_FREMOVEXATTR(tc[i].fd, tc[i].key);

	tst_res(TPASS | TTERRNO, "fsetxattr(2) on %s failed", fname);
}

static void setup(void)
{
	size_t i = 0;
	struct sockaddr_un sun;

	dev_t dev = makedev(1, 3);

	SAFE_TOUCH(FILENAME, 0644, NULL);
	SAFE_MKDIR(DIRNAME, 0644);
	SAFE_SYMLINK(FILENAME, SYMLINK);

	/* root: mknod(2) needs it to create something other than a file */
	SAFE_MKNOD(FIFO, S_IFIFO | 0777, 0);
	SAFE_MKNOD(CHR, S_IFCHR | 0777, dev);
	SAFE_MKNOD(BLK, S_IFBLK | 0777, dev);

	for (i = 0; i < ARRAY_SIZE(tc); i++) {

		if (!tc[i].issocket) {
			tc[i].fd = SAFE_OPEN(tc[i].fname, tc[i].fflags);
			continue;
		}

		/* differently than setxattr calls, when dealing with
		 * sockets, mknod() isn't enough to test fsetxattr(2).
		 * we have to get a real unix socket in order for open()
		 * to get a file desc.
		 */
		tc[i].fd = SAFE_SOCKET(AF_UNIX, SOCK_STREAM, 0);

		memset(&sun, 0, sizeof(struct sockaddr_un));
		sun.sun_family = AF_UNIX;
		strncpy(sun.sun_path, tc[i].fname, sizeof(sun.sun_path) - 1);

		SAFE_BIND(tc[i].fd, (const struct sockaddr *) &sun,
				sizeof(struct sockaddr_un));
	}
}

static void cleanup(void)
{
	size_t i = 0;

	for (i = 0; i < ARRAY_SIZE(tc); i++) {
		if (tc[i].fd > 0)
			SAFE_CLOSE(tc[i].fd);
	}
}

static struct tst_test test = {
	.setup = setup,
	.test = verify_fsetxattr,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tc),
	.needs_devfs = 1,
	.mntpoint = MNTPOINT,
	.needs_root = 1,
	.needs_drivers = (const char *const[]) {
		"brd",
		NULL,
	},
};

#else /* HAVE_SYS_XATTR_H */
TST_TEST_TCONF("<sys/xattr.h> does not exist");
#endif
