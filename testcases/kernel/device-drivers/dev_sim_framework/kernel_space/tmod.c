/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *

 * This example module shows how a test driver
 * can be driven through various ioctl calls in
 * a user space program that has attained the
 * appropriate file descriptor for this device.
 *
 * author: Sean Ruyle
 * date:   06/11/2003
 *
 * module: tmod
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/init.h>
#include <asm/uaccess.h>

#include "tmod.h"
#include "str_mod.h"

MODULE_AUTHOR("Sean Ruyle <srruyle@us.ibm.com>");
MODULE_DESCRIPTION(TMOD_DRIVER_NAME);
MODULE_LICENSE("GPL");

static int tmod_ioctl(struct inode *, struct file *, unsigned int,
		      unsigned long);
static int tmod_open(struct inode *, struct file *);
static int tmod_close(struct inode *, struct file *);

static int test_option(void);

static int Major = TMOD_MAJOR;
//static ltpmod_user_t ltp_mod;

/*
 * File operations struct, to use operations find the
 * correct file descriptor
 */
static struct file_operations tmod_fops = {
open:	tmod_open,
release:tmod_close,
ioctl:	tmod_ioctl,
};

/*
 * open and close operations, just return 0 for
 * your test modules, need them for the file
 * operations structure
 */
static int tmod_open(struct inode *ino, struct file *f)
{
	return 0;
}

static int tmod_close(struct inode *ino, struct file *f)
{
	return 0;
}

/*
 * tmod_ioctl:
 *      a user space program can drive the test functions
 *      through a call to ioctl once the correct file
 *      descriptor has been attained
 *
 * 	in user space the file descriptor that you attain
 * 	will represent the inode and file pointers in
 * 	the kernel ioctl function, and only 3 variables
 *	will be passed in, linux/ioctl.h should be
 *	included
 *
 */
static int tmod_ioctl(struct inode *ino, struct file *f,
		      unsigned int cmd, unsigned long l)
{
	int rc;
	tmod_interface_t tif;
	caddr_t *inparms;
	caddr_t *outparms;

	printk("Enter tmod_ioctl\n");

	inparms = NULL;
	outparms = NULL;
	rc = 0;

	/*
	 * the following calls are used to setup the
	 * parameters that might need to be passed
	 * between user and kernel space, using the tif
	 * pointer that is passed in as the last
	 * parameter to the ioctl
	 *
	 */
	if (copy_from_user(&tif, (void *)l, sizeof(tif))) {
		/* Bad address */
		return (-EFAULT);
	}

	/*
	 * Setup inparms and outparms as needed
	 */
	if (tif.in_len > 0) {
		inparms = (caddr_t *) kmalloc(tif.in_len, GFP_KERNEL);
		if (!inparms) {
			return (-ENOMEM);
		}

		rc = copy_from_user(inparms, tif.in_data, tif.in_len);
		if (rc) {
			kfree(inparms);
			return (-EFAULT);
		}
	}
	if (tif.out_len > 0) {
		outparms = (caddr_t *) kmalloc(tif.out_len, GFP_KERNEL);
		if (!outparms) {
			kfree(inparms);
			return (-ENOMEM);
		}
	}

	/*
	 * Use a switch statement to determine which function
	 * to call, based on the cmd flag that is specified
	 * in user space. Pass in inparms or outparms as
	 * needed
	 *
	 */
	switch (cmd) {
	case LTP_OPTION1:
		rc = test_option();
		break;
	default:
		printk("Mismatching ioctl command\n");
		break;
	}

	/*
	 * copy in the test return code, the reason we
	 * this is so that in user space we can tell the
	 * difference between an error in one of our test
	 * calls or an error in the ioctl function
	 */
	tif.out_rc = rc;
	rc = 0;

	/*
	 * setup the rest of tif pointer for returning to
	 * to user space, using copy_to_user if needed
	 */

	/* if outparms then copy outparms into tif.out_data */
	if (outparms) {
		if (copy_to_user(tif.out_data, outparms, tif.out_len)) {
			printk("tpci: Unsuccessful copy_to_user of outparms\n");
			rc = -EFAULT;
		}
	}

	/* copy tif structure into l so that can be used by user program */
	if (copy_to_user((void *)l, &tif, sizeof(tif))) {
		printk("tpci: Unsuccessful copy_to_user of tif\n");
		rc = -EFAULT;
	}

	/*
	 * free inparms and outparms
	 */
	if (inparms) {
		kfree(inparms);
	}
	if (outparms) {
		kfree(outparms);
	}

	return rc;
}

/*
 * test functions can go here or in a seperate file,
 * remember that the makefile will have to be  modified
 * as well as the header file will need the function
 * prototypes if the test calls go in another file
 *
 * functions should be static so that they may not
 * be called by outside functions, in the kernel
 * if a function is non_static and the symbol is
 * exported using EXPORT_SYMBOL(function_name)
 * then other parts of the kernel such as modules
 * may use that function
 *
 */

static int test_option()
{

	/* setup test parameters and make the call here */

	printk("tmod: this is option1 example\n");

	/* remember that printk does not show up on the console,
	   check /var/log/messages to see your what is printed */

	return 0;
}

/*
 * tmod_init_module
 *      set the owner of tmod_fops, register the module
 *      as a char device, and perform any necessary
 *      initialization for pci devices
 */
static int tmod_init_module(void)
{
	int rc;

	SET_MODULE_OWNER(&tmod_fops);

	rc = register_chrdev(Major, DEVICE_NAME, &tmod_fops);
	if (rc < 0) {
		printk("tmod: Failed to register device.\n");
		return rc;
	}

	if (Major == 0)
		Major = rc;

	/* call any other init functions you might use here */

	printk("tmod: Registration success.\n");
	return 0;
}

/*
 * tmod_exit_module
 *      unregister the device and any necessary
 *      operations to close devices
 */
static void tmod_exit_module(void)
{
	int rc;

	/* free any pointers still allocated, using kfree */

	rc = unregister_chrdev(Major, DEVICE_NAME);
	if (rc < 0)
		printk("tmod: unregister failed\n");
	else
		printk("tmod: unregister success\n");

}

/* specify what that init is run when the module is first
loaded and that exit is run when it is removed */

module_init(tmod_init_module)
    module_exit(tmod_exit_module)
