/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) 2015 Fujitsu Ltd.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 * Copyright (c) Linux Test Project, 2016-2023
 */

#ifndef PWRITEV_H
#define PWRITEV_H

#include <sys/types.h>
#include "config.h"
#include "lapi/syscalls.h"

#if !defined(HAVE_PWRITEV)
int pwritev(int fd, const struct iovec *iov, int iovcnt, off_t offset)
{
	return tst_syscall(__NR_pwritev, fd, iov, iovcnt, offset);
}
#endif

#endif /* PWRITEV_H */
