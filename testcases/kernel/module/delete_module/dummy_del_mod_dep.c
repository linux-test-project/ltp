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
 *		of setup. This module has dependency on dummy_del_mod module
 *		(calls function of dummy_del_mod during initialization).
 *************************************************************************/

#ifndef MODULE
#define MODULE
#endif
/* #define __KERNEL__    Commented this line out b/c it causes errors with
 *                       module.h when it calls /usr/include/linux/version.h
 *                       -11/22/02 Robbie Williamson <robbiew@us.ibm.com>
 */

#include <asm/atomic.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>

extern int dummy_func_test(void);

static int __init dummy_init(void)
{
	struct proc_dir_entry *proc_dummy;

	proc_dummy = proc_mkdir("dummy_dep", 0);
	dummy_func_test();
	return 0;
}

static void __exit dummy_exit(void)
{
	remove_proc_entry("dummy_dep", 0);
}

module_init(dummy_init);
module_exit(dummy_exit);
MODULE_LICENSE("GPL");
