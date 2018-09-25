// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Authors: Jinhui huang <huangjh.jy@cn.fujitsu.com>
 *          Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

void check_execveat(void)
{
	int ret;

	ret = execveat(-1, "", NULL, NULL, AT_EMPTY_PATH);
	if (ret == -1 && errno == EINVAL)
		tst_brk(TCONF, "execveat() not supported");
}
