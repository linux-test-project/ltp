// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linaro Limited. All rights reserved.
 * Author: Sumit Garg <sumit.garg@linaro.org>
 */

#ifndef CHECK_SYNC_FILE_RANGE_H
#define CHECK_SYNC_FILE_RANGE_H

int check_sync_file_range(void)
{
	int ret;

	ret = sync_file_range(-1, 0, 0, 0);
	if (ret == -1 && errno == EINVAL)
		return 0;

	return 1;
}

#endif /* CHECK_SYNC_FILE_RANGE_H */
