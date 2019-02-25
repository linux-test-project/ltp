// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linaro Limited. All rights reserved.
 * Author: Sumit Garg <sumit.garg@linaro.org>
 */

#ifndef SYNCFS_H
#define SYNCFS_H

#include "config.h"
#include <sys/types.h>
#include "lapi/syscalls.h"

#if !defined(HAVE_SYNCFS)
int syncfs(int fd)
{
	return tst_syscall(__NR_syncfs, fd);
}
#endif

#endif /* SYNCFS_H */
