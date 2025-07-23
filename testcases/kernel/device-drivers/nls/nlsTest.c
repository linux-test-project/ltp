
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
 */

/*
 * Legacy Power Management (PM) was removed from the kernel, removed it from
 * here also. ( http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commit;h=6afe1a1fe8ff83f6ac2726b04665e76ba7b14f3e )
 *
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pm.h>
#ifdef HAVE_LINUX_GENHD_H
# include <linux/genhd.h>
#endif
#include <linux/version.h>
#include <linux/string.h>
#include <linux/autoconf.h>
#include <linux/nls.h>
#include <linux/blkdev.h>

#ifdef CONFIG_KMOD
#include <linux/kmod.h>
#endif

#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>

#include "nlsTest.h"

MODULE_AUTHOR("David Cruz <cruzd@us.ibm.com>");
MODULE_AUTHOR("Márton Németh <nm127@freemail.hu>");
MODULE_DESCRIPTION(TEST_DRIVER_NAME);
MODULE_LICENSE("GPL");

/* Struct block_device_operations changed:
 * http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commitdiff;h=d4430d62fa77208824a37fe6f85ab2831d274769
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 28)
static int test_open(struct inode *, struct file *);
static int test_release(struct inode *, struct file *);
static int test_ioctl(struct inode *, struct file *,
		      unsigned int cmd, unsigned long l);
#else
static int test_open(struct block_device *bdev, fmode_t mode);
static int test_release(struct gendisk *disk, fmode_t mode);
static int test_ioctl(struct block_device *bdev, fmode_t mode,
		      unsigned int cmd, unsigned long l);
#endif

static void test_nls_base(void);
static void option1(void);

struct test_block_device {
	spinlock_t queue_lock;
};

static struct block_device_operations bdops = {
	.open = test_open,
	.release = test_release,
	.ioctl = test_ioctl,
};

static struct gendisk *gd_ptr;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 28)
static int test_open(struct inode *inode, struct file *f)
#else
static int test_open(struct block_device *bdev, fmode_t mode)
#endif
{
	printk(KERN_DEBUG "device opened\n");
	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 28)
static int test_release(struct inode *ino, struct file *f)
#else
static int test_release(struct gendisk *disk, fmode_t mode)
#endif
{
	printk(KERN_DEBUG "device released\n");
	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 28)
static int test_ioctl(struct inode *ino, struct file *f,
		      unsigned int cmd, unsigned long l)
#else
static int test_ioctl(struct block_device *bdev, fmode_t mode,
		      unsigned int cmd, unsigned long l)
#endif
{
	int rc = 0;		/* return code */
	int arg;

	printk(KERN_DEBUG "Entered the ioctl call.\n");

	if (copy_from_user(&arg, (void *)l, sizeof(int))) {
		/* bad address */
		return -EFAULT;
	}

	switch (cmd) {
	case OPTION1:
		option1();
		break;
	default:
		printk(KERN_ERR "Mismatching ioctl command\n");
	}

	return rc;
}

static void option1(void)
{
	printk(KERN_DEBUG "Module option 1 chosen\n");
}

static void test_request(struct request_queue *q)
{
	printk(KERN_DEBUG "test_request() called\n");
};

static int test_init_module(void)
{
	struct test_block_device *dev;
	struct request_queue *queue;
	int rc;

	printk(KERN_DEBUG "starting module\n");

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	rc = register_blkdev(NLSMAJOR, DEVICE_NAME);

	printk(KERN_DEBUG "BLK INC - result=%d, major=%d\n", rc, NLSMAJOR);

	if (rc < 0) {
		printk(KERN_ERR "Failed to register device.\n");
		kfree(dev);
		return rc;
	}

	spin_lock_init(&dev->queue_lock);

	gd_ptr = alloc_disk(1);
	if (!gd_ptr) {
		unregister_blkdev(NLSMAJOR, DEVICE_NAME);
		kfree(dev);
		return -ENOMEM;
	}

	printk(KERN_ALERT "gd_ptr after alloc=%p\n", gd_ptr);

	queue = blk_init_queue(test_request, &dev->queue_lock);
	if (!queue) {
		del_gendisk(gd_ptr);
		unregister_blkdev(NLSMAJOR, DEVICE_NAME);
		kfree(dev);
		return -ENOMEM;
	}

	gd_ptr->major = NLSMAJOR;
	gd_ptr->first_minor = 0;
	gd_ptr->fops = &bdops;
	gd_ptr->queue = queue;
	gd_ptr->private_data = dev;
	snprintf(gd_ptr->disk_name, sizeof(gd_ptr->disk_name), DEVICE_NAME);
	add_disk(gd_ptr);

	printk(KERN_DEBUG "block device %s added\n", DEVICE_NAME);

	test_nls_base();

	return 0;
}

static void test_exit_module(void)
{
	printk(KERN_DEBUG "unloading module\n");

	del_gendisk(gd_ptr);
	unregister_blkdev(NLSMAJOR, DEVICE_NAME);
}

static void test_nls_base(void)
{
	wchar_t p = 0x20;
	__u8 s = 0x01;
	int n = 2;
	struct nls_table nls;
	struct nls_table *nls_ptr;
	int ret;
	char charset[20] = "David";

	memset(&nls, 0, sizeof(nls));

	printk(KERN_DEBUG "Calling load_nls_default()\n");
	nls_ptr = load_nls_default();
	printk(KERN_DEBUG "load_nls_default() returns %p\n", nls_ptr);

	printk(KERN_DEBUG "Calling register_nls(%p)\n", &nls);
	ret = register_nls(&nls);
	printk(KERN_DEBUG "register_nls() returns %i\n", ret);

	printk(KERN_DEBUG "Calling unload_nls(%p)\n", &nls);
	unload_nls(&nls);

	printk(KERN_DEBUG "Calling load_nls(\"%s\")\n", charset);
	nls_ptr = load_nls(charset);
	printk(KERN_DEBUG "load_nls() returns %p\n", nls_ptr);

	printk(KERN_DEBUG "Calling unregister_nls(%p)\n", &nls);
	unregister_nls(&nls);

	printk(KERN_DEBUG "Calling utf8_mbtowc(%p, %p, %i);\n", &p, &s, n);
	ret = utf8_mbtowc(&p, &s, n);
	printk(KERN_DEBUG "utf8_mbtowc() returns %i\n", ret);

	printk(KERN_DEBUG "Calling utf8_mbstowcs(%p, %p, %i);\n", &p, &s, n);
	ret = utf8_mbstowcs(&p, &s, n);
	printk(KERN_DEBUG "utf8_mbstowcs() returns %i\n", ret);

	n = 20;

	printk(KERN_DEBUG "Calling utf8_wctomb(%p, 0x%X, %i);\n", &s, p, n);
	ret = utf8_wctomb(&s, p, n);
	printk(KERN_DEBUG "utf8_wctomb() returns %i\n", ret);

	printk(KERN_DEBUG "Calling utf8_wcstombs(%p, %p, %i);\n", &s, &p, n);
	ret = utf8_wcstombs(&s, &p, n);
	printk(KERN_DEBUG "utf8_wcstombs() returns %i\n", ret);

}

module_init(test_init_module);
module_exit(test_exit_module);
