/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2016 Fujitsu Ltd.
 * Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 *
 * Description:
 *  This is a kernel loadable module programme used by lssmod01.sh
 *  testcase which inserts this module for test of lsmod command.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

static int test_init(void)
{
	return 0;
}

static void test_exit(void)
{

}

MODULE_LICENSE("GPL");

module_init(test_init);
module_exit(test_exit);
