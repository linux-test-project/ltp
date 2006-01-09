
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pm.h>
#include <linux/genhd.h>
#include <linux/version.h>
#include <linux/string.h>
#include <linux/config.h>
#include <linux/nls.h>

#ifdef CONFIG_KMOD
#include <linux/kmod.h>
#endif

#include <linux/errno.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>
#include "nlsTest.h"

MODULE_AUTHOR("David Cruz <cruzd@us.ibm.com>");
MODULE_DESCRIPTION(TEST_DRIVER_NAME);
MODULE_LICENSE("GPL");

static int test_ioctl (struct inode *, struct file *, unsigned int, unsigned long);
static int test_open (struct inode *, struct file *);
static int test_close (struct inode *, struct file *);
static void test_nls_base(void);

static struct block_device_operations bdops = {
        open: test_open,
        release: test_close,
        ioctl: test_ioctl,
};

static char genhd_flags = 0;
static struct gendisk * gd_ptr;
static struct pm_dev *ltp_pm_dev = NULL;

static int test_open(struct inode *ino, struct file *f) {
	printk("device open\n");
        return 0;
}

static int test_close(struct inode *ino, struct file *f) {
	printk("device closed\n");
        return 0;
}

static int test_ioctl(struct inode *ino, struct file *f, unsigned int cmd, unsigned long l) {

	int rc = 0;             //return code
        int arg;

        printk("Entered the ioctl call.\n");

        if (copy_from_user(&arg, (void *)l, sizeof(int)) ) {
                //bad address
                return(-EFAULT);
        }

	switch(cmd) {
                case OPTION1: option1(); break;
                default:
                        printk("Mismatching ioctl command\n");
                        break;
        }
        //0 by default
        return rc;
}

static void option1(void) {
        printk("Module option 1 chosen\n");
}

static int ltp_pm_callback(struct pm_dev *dev, pm_request_t rqst, void *data) {
        return 0;
}

static int test_init_module(void) {

	int rc;

	printk("starting module\n");

	ltp_pm_dev = pm_register(PM_UNKNOWN_DEV, 0, ltp_pm_callback);
	rc = register_blkdev(NLSMAJOR, DEVICE_NAME);

	printk("BLK INC - result =%d major %d\n",rc,NLSMAJOR);

        if(rc < 0) {
                printk("Failed to register device.\n");
                return rc;
        }

	gd_ptr = kmalloc(sizeof(struct gendisk *),GFP_KERNEL);
	if (!gd_ptr) {
		 printk(KERN_ALERT "ERROR getting memory !!!\n");
		 return 0;
	}

        printk("major = %d\n",NLSMAJOR);
	gd_ptr = alloc_disk(1);
	printk(KERN_ALERT "gd_ptr after alloc = %p \n",gd_ptr);
	gd_ptr->major = NLSMAJOR;
	gd_ptr->first_minor = 0;
	gd_ptr->fops = &bdops;
	gd_ptr->minor_shift= MINOR_SHIFT_BITS;
	gd_ptr->driverfs_dev = NULL;
	gd_ptr->capacity = MAX_NUM_DISKS;
	gd_ptr->disk_de = NULL;
	gd_ptr->flags = genhd_flags;

	sprintf(gd_ptr->disk_name, DEVICE_NAME);
	add_disk(gd_ptr);
	
	test_nls_base();
	
	return 0;
}


static void test_exit_module(void) {
        
	int rc;
	
        pm_unregister(ltp_pm_dev);
        put_disk(gd_ptr);
        del_gendisk(gd_ptr);
	
	rc = unregister_blkdev(NLSMAJOR, DEVICE_NAME);
		
        if(rc < 0) {
                printk("unregister failed %d\n",rc);
        }
        else {
                printk("unregister success\n");
        }
}

static void test_nls_base(void) {

	wchar_t p=0x20;
	__u8 s=0x01;
	int n=2;
	struct nls_table nls;
	char charset[20]="David";
	
	load_nls_default();
	register_nls(&nls);
	unload_nls(&nls);
	load_nls(charset);
	unregister_nls(&nls);
	utf8_mbtowc(&p, &s, n);
	utf8_mbstowcs(&p, &s, n);
	n=20;
	utf8_wctomb(&s, p, n);
	utf8_wcstombs(&s, &p, n);
}


module_init(test_init_module)
module_exit(test_exit_module)
