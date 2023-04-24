// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2001-2012
 * Copyright (c) International Business Machines Corp., 2001
 */

#include <stdlib.h>
#include <unistd.h>

#include "nfs_flock.h"

int lock_reg(int fd, int type, off_t offset, int whence, off_t len, int cmd)
{
	struct flock lock;

	lock.l_type = type;
	lock.l_start = offset;
	lock.l_whence = whence;
	lock.l_len = len;

	return (fcntl(fd, cmd, &lock));
}

int lock_test(int fd, int type, off_t offset, int whence, int len)
{
	struct flock lock;

	lock.l_type = type;
	lock.l_start = offset;
	lock.l_whence = whence;
	lock.l_len = len;

	if (fcntl(fd, F_GETLK, &lock) < 0) {
		perror("F_GETLK");
		exit(2);
	}

	if (lock.l_type == F_UNLCK)
		return (0);

	return (lock.l_pid);
}
