// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Authors: Jinhui huang <huangjh.jy@cn.fujitsu.com>
 */

/*
 * execveat_errno.c
 *  dummy program which is used by execveat02.c testcase.
 */

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

int main(void)
{
	tst_reinit();
	tst_res(TFAIL, "execveat() passes unexpectedly");
	return 0;
}
