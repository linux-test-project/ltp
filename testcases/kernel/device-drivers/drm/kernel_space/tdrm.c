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
 * author: Kai Zhao
 * date:   09/03/2003
 *
 * module: tdrm
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include "str_drm.h"

#include <linux/config.h>
#include "tdrm.h"
#include "drmP.h"

#define DRIVER_AUTHOR		"Kai Zhao"

#define DRIVER_DESC		"drm test mode"
#define DRIVER_DATE		"20030903"

static drm_pci_list_t DRM(idlist)[] =
{
	{
	PCI_ANY_ID, PCI_ANY_ID}, {
	0, 0}
};

#define DRIVER_CARD_LIST DRM(idlist)

#define DRIVER_FOPS						\
static struct file_operations	DRM(fops) = {			\
	.owner   		= THIS_MODULE,			\
	.open	 		= DRM(open),			\
	.flush	 		= DRM(flush),			\
	.release 		= DRM(release),			\
	.ioctl	 		= DRM(ioctl),			\
	.mmap	 		= DRM(mmap),			\
	.read	 		= DRM(read),			\
	.fasync	 		= DRM(fasync),			\
	.poll	 		= DRM(poll),			\
}

#include "drm_auth.h"
#include "drm_bufs.h"
#include "drm_context.h"
#include "drm_dma.h"
#include "drm_drawable.h"
#include "drm_drv.h"

static int minor = 0;
static unsigned long alloc_pages_address = 0;

int tdrm_test_stub_register(struct inode *inode, struct file *filp,
			    unsigned int cmd, unsigned long arg)
{
	drm_file_t *priv = filp->private_data;
	drm_device_t *dev = priv->dev;
	minor = DRM(stub_register) (DEVICE_NAME, &DRM(fops), dev);
	printk("tdrm stub register : minor = %d\n", minor);
	return 0;

}

int tdrm_test_stub_unregister(struct inode *inode, struct file *filp,
			      unsigned int cmd, unsigned long arg)
{
	DRM(stub_unregister) (minor);
	return 0;
}

int tdrm_test_uninit_agp(struct inode *inode, struct file *filp,
			 unsigned int cmd, unsigned long arg)
{
	DRM(agp_uninit) ();
	return 0;
}

int tdrm_test_init_agp(struct inode *inode, struct file *filp,
		       unsigned int cmd, unsigned long arg)
{
	DRM(agp_init) ();
	return 0;
}

int tdrm_test_add_magic(struct inode *inode, struct file *filp,
			unsigned int cmd, unsigned long arg)
{
	drm_file_t *priv = filp->private_data;
	drm_device_t *dev = priv->dev;
	int magic = 5;
	return (DRM(add_magic) (dev, priv, magic));
}

int tdrm_test_remove_magic(struct inode *inode, struct file *filp,
			   unsigned int cmd, unsigned long arg)
{
	drm_file_t *priv = filp->private_data;
	drm_device_t *dev = priv->dev;
	int magic = 5;
	return (DRM(remove_magic) (dev, magic));
}

int tdrm_test_ctxbitmap_init(struct inode *inode, struct file *filp,
			     unsigned int cmd, unsigned long arg)
{
	drm_file_t *priv = filp->private_data;
	drm_device_t *dev = priv->dev;
	return (DRM(ctxbitmap_init) (dev));
}

int tdrm_test_ctxbitmap_cleanup(struct inode *inode, struct file *filp,
				unsigned int cmd, unsigned long arg)
{
	drm_file_t *priv = filp->private_data;
	drm_device_t *dev = priv->dev;
	DRM(ctxbitmap_cleanup) (dev);
	return 0;
}

int tdrm_test_alloc_pages(struct inode *inode, struct file *filp,
			  unsigned int cmd, unsigned long arg)
{
	alloc_pages_address = DRM(alloc_pages) (1, 0);
//      printk("address = %ld\n",alloc_pages_address);
	return 0;
}

int tdrm_test_free_pages(struct inode *inode, struct file *filp,
			 unsigned int cmd, unsigned long arg)
{
	DRM(free_pages) (alloc_pages_address, 1, 0);
	return 0;
}

#ifndef MODULE

/* JH- We have to hand expand the string ourselves because of the cpp.  If
 * anyone can think of a way that we can fit into the __setup macro without
 * changing it, then please send the solution my way.
 */
static int __init tdrm_options(char *str)
{
	DRM(parse_options) (str);
	return 1;
}

__setup(DRIVER_NAME "=", tdrm_options);
#endif

#include "drm_fops.h"
#include "drm_init.h"
#include "drm_ioctl.h"
#include "drm_lock.h"
#include "drm_memory.h"
#include "drm_proc.h"
#include "drm_vm.h"
#include "drm_stub.h"
#include "drm_agpsupport.h"
