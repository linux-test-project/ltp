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

 * This test is meant to hit
 * many of the inline functions
 * in various include files.
 *
 * Author: David Cruz <cruzd@us.ibm.com>
 * Module: includeTest
 *
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pm.h>
#include <linux/genhd.h>
#include <linux/in.h>
#include <asm/types.h>
#include <linux/lockd/bind.h>
#include <acpi/acpi_drivers.h>
#include <linux/nfsd/nfsfh.h>
#include <linux/sunrpc/auth.h>
#include <linux/sunrpc/cache.h>
#include <linux/sunrpc/svc.h>
#include <linux/sunrpc/xdr.h>
#include <linux/sunrpc/timer.h>
#include <video/vga.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/lockd/lockd.h>
#include <linux/lockd/nlm.h>
#include <linux/nfsd/export.h>
#include <asm/uaccess.h>
#include "includeTest.h"

MODULE_AUTHOR("David Cruz <cruzd@us.ibm.com>");
MODULE_DESCRIPTION(TEST_DRIVER_NAME);
MODULE_LICENSE("GPL");

static int test_ioctl(struct inode *, struct file *, unsigned int,
		      unsigned long);
static int test_open(struct inode *, struct file *);
static int test_close(struct inode *, struct file *);
static void test_acpi(void);
static void test_vga(void);
static void test_sunrpc_auth(void);
static void test_nfsfh(void);
static void test_lockd(void);
static void test_sunrpc_cache(void);
static void test_sunrpc_svc(void);
static void test_sunrpc_timer(void);

static int Major = 0;

static struct block_device_operations bdops = {
open:	test_open,
release:test_close,
ioctl:	test_ioctl,
};

static char genhd_flags = 0;
static struct gendisk *gd_ptr;
static struct pm_dev *ltp_pm_dev = NULL;

static int test_open(struct inode *ino, struct file *f)
{
	printk("device open\n");
	return 0;
}

static int test_close(struct inode *ino, struct file *f)
{
	printk("device closed\n");
	return 0;
}

static int test_ioctl(struct inode *ino, struct file *f, unsigned int cmd,
		      unsigned long l)
{

	int rc = 0;		//return code
	int arg;

	printk("Entered the ioctl call.\n");

	if (copy_from_user(&arg, (void *)l, sizeof(int))) {
		//bad address
		return (-EFAULT);
	}

	switch (cmd) {

	case OPTION1:
		option1();
		break;
	default:
		printk("Mismatching ioctl command\n");
		break;
	}

	//0 by default
	return rc;
}

static void option1(void)
{
	printk("Module option 1 chosen\n");
}

static int ltp_pm_callback(struct pm_dev *dev, pm_request_t rqst, void *data)
{
	return 0;
}

static int test_init_module(void)
{

	int rc;

	printk("starting module\n");

	ltp_pm_dev = pm_register(PM_UNKNOWN_DEV, 0, ltp_pm_callback);
	rc = register_blkdev(INCLUDEMAJOR, DEVICE_NAME);

	printk("BLK INC - result =%d major %d\n", rc, INCLUDEMAJOR);

	if (rc < 0) {
		printk("Failed to register device.\n");
		return rc;
	}

	gd_ptr = kmalloc(sizeof(struct gendisk *), GFP_KERNEL);
	if (!gd_ptr) {
		printk(KERN_ALERT "ERROR getting memory !!!\n");
		return 0;
	}

	printk("major = %d\n", Major);
	gd_ptr = alloc_disk(1);
	printk(KERN_ALERT "gd_ptr after alloc = %p \n", gd_ptr);
	gd_ptr->major = INCLUDEMAJOR;
	gd_ptr->first_minor = 0;
	gd_ptr->fops = &bdops;
//      gd_ptr->minor_shift= MINOR_SHIFT_BITS;
	gd_ptr->driverfs_dev = NULL;
	gd_ptr->capacity = MAX_NUM_DISKS;
//      gd_ptr->disk_de = NULL;
	gd_ptr->flags = genhd_flags;

	sprintf(gd_ptr->disk_name, DEVICE_NAME);
	add_disk(gd_ptr);

	printk("major = %d\n", Major);

	test_acpi();
	test_vga();
	test_lockd();
	test_sunrpc_auth();
	test_nfsfh();
	test_sunrpc_cache();
	test_sunrpc_svc();
	test_sunrpc_timer();
	printk("finished module\n");

	return 0;
}

static void test_exit_module(void)
{

	int rc;

	pm_unregister(ltp_pm_dev);
	put_disk(gd_ptr);
	del_gendisk(gd_ptr);

	rc = unregister_blkdev(INCLUDEMAJOR, DEVICE_NAME);

	if (rc < 0) {
		printk("unregister failed %d\n", rc);
	} else {
		printk("unregister success\n");
	}
}

static void test_acpi(void)
{
	u32 flag;

	for (flag = 0; flag <= 4; flag++)
		acpi_set_debug(flag);

	printk("finished acpi test\n");
}

static void test_sunrpc_auth(void)
{
	struct rpc_cred cred;

	atomic_set(&(cred.cr_count), 0);
	get_rpccred(&cred);
	printk("finished auth test\n");
}

static void test_vga(void)
{

	unsigned short vgaS = 0;
	int i = 0;
	vga_r((caddr_t) i, vgaS);
	printk("finished vga test\n");
}

static void test_nfsfh(void)
{
	dev_t dev = 0;
	u32 unfs = 0, u32ptr[2];
	ino_t ino = 0;
	struct svc_fh A1;
	int i = 20;

	u32_to_dev_t((__u32) unfs);
	ino_t_to_u32(ino);
	u32_to_ino_t((__u32) unfs);
	mk_fsid_v0(u32ptr, dev, ino);
	mk_fsid_v1(u32ptr, unfs);
	SVCFH_fmt(&A1);
	fh_init(&A1, i);
	fh_lock(&A1);
	fh_unlock(&A1);
	printk("finished nfsfh test\n");
}

static void test_lockd(void)
{

	struct nlm_file file;
	struct sockaddr_in sin1, sin2;
	struct file_lock fl1, fl2;

	nlm_compare_locks(&fl1, &fl2);
	nlm_cmp_addr(&sin1, &sin2);
	nlmsvc_file_inode(&file);
	printk("finished lockd test\n");
}

static void test_sunrpc_cache(void)
{
	struct cache_head head;
	struct cache_detail detail;

	cache_get(&head);
	cache_put(&head, &detail);
	printk("finished cache test\n");
}

static void test_sunrpc_svc(void)
{
	u32 val;
	struct svc_rqst rqstp;
	char name[50];
	struct iovec iov;
	int bits = 0, bits2 = 0;
	rqstp.rq_resused = 1;

	svc_getu32(&iov);
//      svc_putu32(&iov, val);

	xdr_argsize_check(&rqstp, &val);
	xdr_ressize_check(&rqstp, &val);
	svc_take_page(&rqstp);
	svc_pushback_allpages(&rqstp);
	svc_pushback_unused_pages(&rqstp);
	svc_free_allpages(&rqstp);
	hash_str(name, bits);
	hash_mem(name, bits, bits2);
	printk("finished svc test\n");

}

static void test_sunrpc_timer()
{
	struct rpc_rtt rt;

	rpc_inc_timeo(&rt);
	rpc_clear_timeo(&rt);
	rpc_ntimeo(&rt);
	printk("finished timer test\n");
}

module_init(test_init_module)
    module_exit(test_exit_module)
