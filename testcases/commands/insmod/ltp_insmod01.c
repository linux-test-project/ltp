/*
 * Copyright (c) 2016 Fujitsu Ltd.
 * Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 *
 * Description:
 *  This is a kernel loadable module programme used by insmod01.sh
 *  testcase which inserts this module for test of insmod command.
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
