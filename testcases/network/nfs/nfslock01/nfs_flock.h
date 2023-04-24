// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2001-2009
 * Copyright (c) International Business Machines Corp., 2001
 */

#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

int		lock_reg(int, int, off_t, int, off_t, int);

#define read_lock(fd, offset, whence, len) \
			lock_reg(fd, F_RDLCK, offset, whence, len, F_SETLK)
#define write_lock(fd, offset, whence, len) \
			lock_reg(fd, F_WRLCK, offset, whence, len, F_SETLK)
#define un_lock(fd, offset, whence, len) \
			lock_reg(fd, F_UNLCK, offset, whence, len, F_SETLK)
#define readb_lock(fd, offset, whence, len) \
			lock_reg(fd, F_RDLCK, offset, whence, len, F_SETLKW)
#define writeb_lock(fd, offset, whence, len) \
			lock_reg(fd, F_WRLCK, offset, whence, len, F_SETLKW)
#define unb_lock(fd, offset, whence, len) \
			lock_reg(fd, F_UNLCK, offset, whence, len, F_SETLKW)
