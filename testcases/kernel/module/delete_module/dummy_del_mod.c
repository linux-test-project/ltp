/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
/*************************************************************************
 * Description: This is a kernel loadable module programme used by
 *		delete_module03 testcase which inserts this module as part
 *		setup.
 *************************************************************************/

#ifndef MODULE
#define MODULE
#endif

/* #define __KERNEL__    Commented this line out b/c it causes errors with
 *			 module.h when it calls /usr/include/linux/version.h
 *			 -11/22/02 Robbie Williamson <robbiew@us.ibm.com>
 */

#include <asm/atomic.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>

static int dummy_func_test(void);

/* Dummy function called by dependent module */

static int dummy_func_test()
{
	return 0;
}

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
EXPORT_SYMBOL(dummy_func_test);
MODULE_LICENSE("GPL");
