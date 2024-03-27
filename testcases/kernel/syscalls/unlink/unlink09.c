// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 FUJITSU LIMITED. All Rights Reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * [Description]
 *
 * Verify that unlink(2) fails with
 *
 * - EPERM when target file is marked as immutable or append-only
 * - EROFS when target file is on a read-only filesystem.
 */

#include <sys/ioctl.h>
#include "tst_test.h"
#include "lapi/fs.h"

#define TEST_EPERM_IMMUTABLE "test_eperm_immutable"
#define TEST_EPERM_APPEND_ONLY "test_eperm_append_only"
#define DIR_EROFS "erofs"
#define TEST_EROFS "erofs/test_erofs"

static int fd_immutable;
static int fd_append_only;

static struct test_case_t {
	char *filename;
	int *fd;
	int flag;
	int expected_errno;
	char *desc;
} tcases[] = {
	{TEST_EPERM_IMMUTABLE, &fd_immutable, FS_IMMUTABLE_FL, EPERM,
		"target file is immutable"},
	{TEST_EPERM_APPEND_ONLY, &fd_append_only, FS_APPEND_FL, EPERM,
		"target file is append-only"},
	{TEST_EROFS, NULL, 0, EROFS, "target file in read-only filesystem"},
};

static void setup(void)
{
	int attr;

	fd_immutable = SAFE_OPEN(TEST_EPERM_IMMUTABLE, O_CREAT, 0600);
	SAFE_IOCTL(fd_immutable, FS_IOC_GETFLAGS, &attr);
	attr |= FS_IMMUTABLE_FL;
	SAFE_IOCTL(fd_immutable, FS_IOC_SETFLAGS, &attr);

	fd_append_only = SAFE_OPEN(TEST_EPERM_APPEND_ONLY, O_CREAT, 0600);
	SAFE_IOCTL(fd_append_only, FS_IOC_GETFLAGS, &attr);
	attr |= FS_APPEND_FL;
	SAFE_IOCTL(fd_append_only, FS_IOC_SETFLAGS, &attr);
}

static void cleanup(void)
{
	int attr;

	SAFE_IOCTL(fd_immutable, FS_IOC_GETFLAGS, &attr);
	attr &= ~FS_IMMUTABLE_FL;
	SAFE_IOCTL(fd_immutable, FS_IOC_SETFLAGS, &attr);
	SAFE_CLOSE(fd_immutable);

	SAFE_IOCTL(fd_append_only, FS_IOC_GETFLAGS, &attr);
	attr &= ~FS_APPEND_FL;
	SAFE_IOCTL(fd_append_only, FS_IOC_SETFLAGS, &attr);
	SAFE_CLOSE(fd_append_only);
}

static void verify_unlink(unsigned int i)
{
	struct test_case_t *tc = &tcases[i];
	int attr;

	TST_EXP_FAIL(unlink(tc->filename), tc->expected_errno, "%s", tc->desc);

	/* If unlink() succeeded unexpectedly, test file should be restored. */
	if (!TST_RET) {
		if (tc->fd) {
			*(tc->fd) = SAFE_OPEN(tc->filename, O_CREAT, 0600);
			if (tc->flag) {
				SAFE_IOCTL(*(tc->fd), FS_IOC_GETFLAGS, &attr);
				attr |= tc->flag;
				SAFE_IOCTL(*(tc->fd), FS_IOC_SETFLAGS, &attr);
			}
		} else {
			SAFE_TOUCH(tc->filename, 0600, 0);
		}
	}
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.cleanup = cleanup,
	.test = verify_unlink,
	.needs_rofs = 1,
	.mntpoint = DIR_EROFS,
	.needs_root = 1,
};
