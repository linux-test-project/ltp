// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Author: Wayne Boyer
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */
/*
 * Test that fchmod() fails and sets the proper errno values.
 */

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>
#include "tst_test.h"

#define MNT_POINT "mntpoint"

static int fd1;
static int fd2;
static int fd3;

static struct tcase {
	int *fd;
	int mode;
	int exp_errno;
} tcases[] = {
	{&fd1, 0644, EPERM},
	{&fd2, 0644, EBADF},
	{&fd3, 0644, EROFS},
};

static void verify_fchmod(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TEST(fchmod(*tc->fd, tc->mode));

	if (TST_RET != -1) {
		tst_res(TFAIL, "fchmod() passed unexpectedly (%li)",
			TST_RET);
		return;
	}

	if (TST_ERR == tcases[i].exp_errno) {
		tst_res(TPASS | TTERRNO, "fchmod() failed expectedly");
		return;
	}

	tst_res(TFAIL | TTERRNO,
	        "fchmod() failed unexpectedly, expected %i - %s",
		TST_ERR, tst_strerrno(TST_ERR));
}

static void setup(void)
{
	struct passwd *ltpuser = SAFE_GETPWNAM("nobody");

	fd3 = SAFE_OPEN(MNT_POINT"/file", O_RDONLY);
	fd1 = SAFE_OPEN("tfile_1", O_RDWR | O_CREAT, 0666);
	fd2 = SAFE_OPEN("tfile_2", O_RDWR | O_CREAT, 0666);
	SAFE_CLOSE(fd2);

	SAFE_SETEUID(ltpuser->pw_uid);
}

static void cleanup(void)
{
	if (fd1 > 0)
		SAFE_CLOSE(fd1);

	if (fd3 > 0)
		SAFE_CLOSE(fd3);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_fchmod,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.needs_rofs = 1,
	.mntpoint = MNT_POINT,
};
