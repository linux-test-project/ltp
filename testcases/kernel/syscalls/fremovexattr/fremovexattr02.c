// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Linaro Limited. All rights reserved.
 * Author: Rafael David Tinoco <rafael.tinoco@linaro.org>
 */

/*
 * Test Name: fremovexattr02
 *
 * Test cases::
 * 1) fremovexattr(2) fails if the named attribute does not exist.
 * 2) fremovexattr(2) fails if file descriptor is not valid.
 * 3) fremovexattr(2) fails if named attribute has an invalid address.
 *
 * Expected Results:
 * fremovexattr(2) should return -1 and set errno to ENODATA.
 * fremovexattr(2) should return -1 and set errno to EBADF.
 * fremovexattr(2) should return -1 and set errno to EFAULT.
 */

#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif

#include "tst_test.h"

#ifdef HAVE_SYS_XATTR_H

#define XATTR_TEST_KEY "user.testkey"

#define MNTPOINT "mntpoint"
#define FNAME MNTPOINT"/fremovexattr02testfile"

static int fd = -1;

struct test_case {
	int fd;
	char *key;
	int exp_err;
};

struct test_case tc[] = {
	{				/* case 1: attribute does not exist */
	 .key = XATTR_TEST_KEY,
	 .exp_err = ENODATA,
	 },
	{				/* case 2: file descriptor is invalid */
	 .fd = -1,
	 .key = XATTR_TEST_KEY,
	 .exp_err = EBADF,
	 },
	{				/* case 3: bad name attribute */
	 .exp_err = EFAULT,
	},
};

static void verify_fremovexattr(unsigned int i)
{
	TEST(fremovexattr(tc[i].fd, tc[i].key));

	if (TST_RET == -1 && TST_ERR == EOPNOTSUPP)
		tst_brk(TCONF, "fremovexattr(2) not supported");

	if (TST_RET == -1) {
		if (tc[i].exp_err == TST_ERR) {
			tst_res(TPASS | TTERRNO,
				"fremovexattr(2) failed expectedly");
		} else {
			tst_res(TFAIL | TTERRNO,
				"fremovexattr(2) should fail with %s",
				tst_strerrno(tc[i].exp_err));
		}
		return;
	}

	tst_res(TFAIL, "fremovexattr(2) returned %li", TST_RET);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static void setup(void)
{
	size_t i = 0;

	fd = SAFE_OPEN(FNAME, O_RDWR | O_CREAT, 0644);

	for (i = 0; i < ARRAY_SIZE(tc); i++) {

		if (tc[i].fd != -1)
			tc[i].fd = fd;

		if (!tc[i].key && tc[i].exp_err == EFAULT)
			tc[i].key = tst_get_bad_addr(cleanup);
	}
}

static struct tst_test test = {
	.timeout = 10,
	.setup = setup,
	.test = verify_fremovexattr,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tc),
	.mntpoint = MNTPOINT,
	.mount_device = 1,
	.all_filesystems = 1,
	.needs_root = 1,
};

#else /* HAVE_SYS_XATTR_H */
TST_TEST_TCONF("<sys/xattr.h> does not exist");
#endif
