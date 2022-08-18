// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2018 Xiao Yang <yangx.jy@cn.fujitsu.com>
 */
/*
 * Description:
 * This is a kernel loadable module programme used by delete_module*
 * testcases which insert this module as part setup.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>

/* Dummy function called by dependent module */
int dummy_func_test(void)
{
	return 0;
}
EXPORT_SYMBOL(dummy_func_test);

static int __init dummy_init(void)
{
	struct proc_dir_entry *proc_dummy;

	proc_dummy = proc_mkdir("dummy", 0);
	return 0;
}

static void __exit dummy_exit(void)
{
	remove_proc_entry("dummy", 0);
}

module_init(dummy_init);
module_exit(dummy_exit);
MODULE_LICENSE("GPL");
