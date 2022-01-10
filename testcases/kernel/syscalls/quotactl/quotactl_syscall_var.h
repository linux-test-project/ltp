// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

#ifndef LTP_QUOTACTL_SYSCALL_VAR_H
#define LTP_QUOTACTL_SYSCALL_VAR_H

#include "lapi/quotactl.h"

#define QUOTACTL_SYSCALL_VARIANTS 2
#define MNTPOINT "mntpoint"

static int fd = -1;

static int do_quotactl(int fd, int cmd, const char *special, int id, caddr_t addr)
{
	if (tst_variant == 0)
		return quotactl(cmd, special, id, addr);
	return quotactl_fd(fd, cmd, id, addr);
}

static void quotactl_info(void)
{
	if (tst_variant == 0)
		tst_res(TINFO, "Test quotactl()");
	else
		tst_res(TINFO, "Test quotactl_fd()");
}

#endif /* LTP_QUOTACTL_SYSCALL_VAR_H */
