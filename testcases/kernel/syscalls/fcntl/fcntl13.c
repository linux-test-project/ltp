// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2021
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 */

/*\
 * Tests basic error handling of the fcntl syscall.
 *
 * - EFAULT when lock is outside your accessible address space
 * - EINVAL when cmd argument is not recognized by this kernel
 * - EINVAL when cmd argument is F_SETLK and flock.l_whence is not equal to
 *   SEET_CUR,SEEK_SET,SEEK_END
 * - EBADF when fd refers to an invalid file descriptor
 */

#include <fcntl.h>
#include "tst_test.h"

#define F_BADCMD 999

static struct flock flock;

static struct tcase {
	int fd;
	int cmd;
	struct flock *flock;
	char *desc;
	int exp_errno;
} tcases[] = {
	{1, F_SETLK, NULL, "F_SETLK", EFAULT},
	{1, F_BADCMD, &flock, "F_BADCMD", EINVAL},
	{1, F_SETLK, &flock,  "F_SETLK", EINVAL},
	{-1, F_GETLK, &flock, "F_GETLK", EBADF}
};

static void verify_fcntl(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	if (!tc->flock)
		tc->flock = tst_get_bad_addr(NULL);

	TST_EXP_FAIL2(fcntl(tc->fd, tc->cmd, tc->flock), tc->exp_errno,
		"fcntl(%d, %s, flock)", tc->fd, tc->desc);
}

static void setup(void)
{
	flock.l_whence = -1;
	flock.l_type = F_WRLCK;
	flock.l_start = 0L;
	flock.l_len = 0L;
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_fcntl,
};
