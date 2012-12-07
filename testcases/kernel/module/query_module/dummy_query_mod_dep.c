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
 *		testcases of query_module(2). This module has dependency
 *		on dummy_query_mod module (calls function of dummy_del_mod
 *		during initialization).
 *************************************************************************/

#define MODULE
/* #define __KERNEL__    Commented this line out b/c it causes errors with
 *                       module.h when it calls /usr/include/linux/version.h
 *                       -12/03/02 Robbie Williamson <robbiew@us.ibm.com>
 */

#include <linux/config.h>
#include <asm/atomic.h>
#include <linux/module.h>
#include <linux/kernel.h>

extern void dummy_func_test(void);

/* Initialization routine of module */
int init_module(void)
{
	/*
	 * Call function of other module, does nothing, used to create
	 * dependency with other module
	 */
	dummy_func_test();
	return 0;
}

/* Cleanup routine of module */
void cleanup_module(void)
{
	return;
}
