// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2018 Xiao Yang <yangx.jy@cn.fujitsu.com>
 */
/*
 * Description:
 * This is a kernel loadable module programme used by delete_module03
 * testcase which inserts this module as part of setup. This module
 * has dependency on dummy_del_mod module (calls function of dummy_del_mod
 * during initialization).
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>

#define DIRNAME "dummy_dep"

extern int dummy_func_test(void);

static int __init dummy_init(void)
{
	struct proc_dir_entry *proc_dummy;

	proc_dummy = proc_mkdir(DIRNAME, 0);
	dummy_func_test();
	return 0;
}

static void __exit dummy_exit(void)
{
	remove_proc_entry(DIRNAME, 0);
}

module_init(dummy_init);
module_exit(dummy_exit);
MODULE_LICENSE("GPL");
