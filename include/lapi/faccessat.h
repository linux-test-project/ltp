// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 FUJITSU LIMITED. All rights reserved.
 * Copyright (c) Linux Test Project, 2003-2023
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

#ifndef FACCESSAT2_H
#define FACCESSAT2_H

#include "tst_test.h"
#include "config.h"
#include "lapi/syscalls.h"

#ifndef HAVE_FACCESSAT2
int faccessat2(int dirfd, const char *pathname, int mode, int flags)
{
	return tst_syscall(__NR_faccessat2, dirfd, pathname, mode, flags);
}
#endif

#endif /* FACCESSAT2_H */
