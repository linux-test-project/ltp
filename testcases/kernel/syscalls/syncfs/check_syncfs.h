// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linaro Limited. All rights reserved.
 * Author: Sumit Garg <sumit.garg@linaro.org>
 */

#ifndef CHECK_SYNCFS_H
#define CHECK_SYNCFS_H

void check_syncfs(void)
{
	int ret;

	ret = syncfs(-1);
	if (ret == -1 && errno == EINVAL)
		tst_brk(TCONF, "syncfs() not supported");
}

#endif /* CHECK_SYNCFS_H */
