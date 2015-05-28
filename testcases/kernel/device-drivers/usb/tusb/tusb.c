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

 * This is a kernel module for testing usb
 * kernel functions found in /usr/src/<linux_
 * dir>/drivers/usb. The module is registered
 * as a char device with the system so that
 * ioctl calls can be made in a user space
 * program that has attained the correct
 * file descriptor for this module. A usb
 * driver is registered with the system also
 * so that it may be used in testing usb
 * system calls.
 *
 * Reference: "Linux Device Drivers" by
 * Alessandro Rubini and Jonathan Corbet
 *
 * Module name:	tusb
 * Author:	Sean Ruyle (srruyle@us.ibm.com)
 * Date:	6/2/2003
 *
 * tusb.c
 */

#include <linux/autoconf.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/pci.h>
#include <linux/input.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <asm/uaccess.h>

#include "tusb.h"
#include "st_tusb.h"

MODULE_AUTHOR("Sean Ruyle <srruyle@us.ibm.com>");
MODULE_DESCRIPTION(TEST_USB_DRIVER_NAME);
MODULE_LICENSE("GPL");

static int tusb_ioctl(struct inode *, struct file *, unsigned int,
		      unsigned long);
static int tusb_open(struct inode *, struct file *);
static int tusb_close(struct inode *, struct file *);

static int test_find_usbdev(void);
static int test_find_hcd(void);
static int test_hcd_probe(void);
static int test_hcd_remove(void);
static int test_hcd_suspend(void);
static int test_hcd_resume(void);
/*
 * File operations stuff
 */
static int Major = TUSB_MAJOR;
static tusb_user_t ltp_usb;

static struct file_operations tusb_fops = {
open:	tusb_open,
release:tusb_close,
ioctl:	tusb_ioctl,
};

static int tusb_open(struct inode *ino, struct file *f)
{
	return 0;
}

static int tusb_close(struct inode *ino, struct file *f)
{
	return 0;
}

/*
 * usb stuff
 */
struct tusb_device {
	char name[128];
	char phys[64];
	struct usb_device *usbdev;
	struct input_dev dev;
	struct urb *irq;
	int open;

	signed char *data;
	dma_addr_t data_dma;
};

static void tusb_disconnect(struct usb_interface *intf)
{
	printk("tusb: Entered disconnect function\n");
}

static int tusb_probe(struct usb_interface *intf,
		      const struct usb_device_id *id)
{
	printk("tusb: Entered probe function\n");
	return 0;

}

static struct usb_device_id tusb_id_table[] = {
	{
	 USB_INTERFACE_INFO(3, 1, 1),
driver_info:(unsigned long)"keyboard"},
	{
	 USB_INTERFACE_INFO(3, 1, 2),
driver_info:(unsigned long)"mouse"},
	{
	 0,
	 }
};

MODULE_DEVICE_TABLE(usb, tusb_id_table);

static struct usb_driver test_usb_driver = {
name:	"tusb_two",
probe:	tusb_probe,
disconnect:tusb_disconnect,
id_table:tusb_id_table,
};

#if 0
static int test_alloc_dev(struct usb_device *dev)
{
	printk("Entered test_alloc_dev\n");
	return 0;
}

static int test_dealloc_dev(struct usb_device *dev)
{
	printk("Entered test_dealloc_dev\n");
	return 0;
}

static int test_get_current_frame_number(struct usb_device *dev)
{
	printk("Entered test_get_current_frame_number\n");
	return 0;
}

static int test_submit_urb(struct urb *purb)
{
	printk("Entered test_submit_urb\n");
	return 0;
}

static int test_unlink_urb(struct urb *purb)
{
	printk("Entered test_unlink_urb\n");
	return 0;
}

static struct usb_operations test_device_operations = {
	.allocate = test_alloc_dev,
	.deallocate = test_dealloc_dev,
	.get_frame_number = test_get_current_frame_number,
	.submit_urb = test_submit_urb,
	.unlink_urb = test_unlink_urb,
};
#endif

static int tusb_ioctl(struct inode *ino, struct file *f,
		      unsigned int cmd, unsigned long l)
{
	int rc;
	tusb_interface_t tif;
	caddr_t *inparms;
	caddr_t *outparms;

	printk("tusb: Entered the ioctl call\n");

	rc = 0;
	inparms = NULL;
	outparms = NULL;

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

	switch (cmd) {
	case FIND_DEV:
		rc = test_find_usbdev();
		break;
	case TEST_FIND_HCD:
		rc = test_find_hcd();
		break;
	case TEST_HCD_PROBE:
		rc = test_hcd_probe();
		break;
	case TEST_HCD_REMOVE:
		rc = test_hcd_remove();
		break;
	case TEST_HCD_SUSPEND:
		rc = test_hcd_suspend();
		break;
	case TEST_HCD_RESUME:
		rc = test_hcd_resume();
		break;
	default:
		printk("Mismatching ioctl command\n");
		rc = 1;
		break;
	}

	if (!ltp_usb.dev)
		printk("tusb: After ioctl call dev DNE\n");

	/*
	 * copy in the return data, and test return code
	 */
	tif.out_rc = rc;
	rc = 0;

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
 * test_find_usbdev
 *	using our driver, attempt to find
 *	a usb device that our driver can use,
 *	and set the pointers in our test interface
 *	structure to the device pointer so that
 *	it can be used future test calls
 */
static int test_find_usbdev()
{
	struct usb_device *udev =
	    (struct usb_device *)kmalloc(sizeof(struct usb_device), GFP_KERNEL);
	struct usb_bus *bus =
	    (struct usb_bus *)kmalloc(sizeof(struct usb_bus), GFP_KERNEL);

	/* Zero out the ltp_usb */
	memset(&ltp_usb, 0, sizeof(tusb_user_t));

	ltp_usb.bus = bus;
	ltp_usb.dev = udev;

	/* allocate the usb_bus pointer */
#if 0
	bus = usb_alloc_bus(&test_device_operations);
	if (!bus) {
		printk("tusb: Did not allocate a bus\n");
		return 1;
	} else {
		printk("tusb: Allocated a bus pointer\n");
		memcpy(ltp_usb.bus, bus, sizeof(struct usb_bus));
		printk("test1\n");
	}

	/* allocate the usb_device pointer */
	udev = usb_alloc_dev(NULL, bus);
	if (udev) {
		printk("tusb: Found a usb device pointer\n");
		memcpy(ltp_usb.dev, udev, sizeof(struct usb_device));
	} else {
		printk("tusb: Failed find usb device pointer\n");
		return 1;
	}

	/* connect the new device and setup pointers */
	usb_connect(udev);
	usb_new_device(udev);
#endif

	return 0;
}

/*
 * test_find_hcd
 *	make call to pci_find_class with correct flags
 * 	to attempt to find a usb hostcontroller, that
 *	we can later use to test hcd functions, must
 * 	have either uchi or ohci usb options enabled
 *	or will not find a device
 */
static int test_find_hcd()
{
	struct pci_dev *pdev =
	    (struct pci_dev *)kmalloc(sizeof(struct pci_dev), GFP_KERNEL);

	ltp_usb.pdev = pdev;

#if 0
	/* try and get a usb hostcontroller if possible */
	pdev = pci_find_class(PCI_CLASS_SERIAL_USB << 8, NULL);
	if (pdev) {
		printk("tusb: WOOT! Found a usb host controller!\n");
		printk("tusb: Slot number: %d\n", pdev->devfn);

		memcpy(ltp_usb.pdev, pdev, sizeof(struct pci_dev));

		if (pdev->driver->id_table)
			printk("tusb: id_table exists\n");

		return 0;
	} else {
		printk("tusb: Failed to find host controller\n");
		printk("tusb: Check kernel options enabled\n");
		return 1;
	}
#else
	return 1;
#endif

}

/*
 * test_hcd_probe
 * 	make call to usb_hcd_pci_probe which will
 *	enable the usb hostcontroller, pass in a pci_dev
 * 	and a pci_device_id
 */
static int test_hcd_probe()
{
	int rc;
	struct usb_hcd *hcd = NULL;
	struct pci_dev *pdev = ltp_usb.pdev;
	struct pci_device_id *id = NULL;

	if (!pdev) {
		printk("tusb: pdev pointer not set\n");
		return 1;
	}

	id = (struct pci_device_id *)pdev->driver->id_table;

	if (!id || !id->driver_data) {
		printk("tusb: id_table not set\n");
		return 1;
	}

	/* release regions before probe call */
	hcd = pci_get_drvdata(pdev);

	if (!hcd) {
		printk("tusb: hcd pointer not found\n");
		return 1;
	} else
		release_region(pci_resource_start(pdev, hcd->region),
			       pci_resource_len(pdev, hcd->region));

	/* make test call */
	rc = usb_hcd_pci_probe(pdev, id);

	if (rc)
		printk("tusb: retval hcd probe = %d\n", rc);
	else
		printk("tusb: Success for usb_hcd_pci_probe\n");

	return rc;
}

/*
 * test_hcd_remove
 *	make call to usb_hcd_pci_remove which will
 * 	remove setup for the usb host controller
 * 	from the system, attempting to call this
 * 	before probe test call so that regions
 *	will be available to the probe test call
 */
static int test_hcd_remove()
{
	struct pci_dev *pdev = NULL;
	struct usb_hcd *hcd = NULL;
	struct hc_driver *hdrv = NULL;

	/* check that hcd pointer exists */
	if (!ltp_usb.pdev) {
		printk("tusb: pdev pointer not found\n");
		return 1;
	} else {
		pdev = ltp_usb.pdev;
		hcd = pci_get_drvdata(pdev);
	}

	if (!hdrv->stop) {
		printk("tusb: stop function not found\n");
		return 1;
	} else
		hcd->driver->stop(hcd);

	return 0;
}

/*
 * test_hcd_suspend
 *	make call to suspend with a dev pointer and
 *	a u32 state variable that is the state to
 *	move into
 */
static int test_hcd_suspend()
{
	int rc;
	struct pci_dev *pdev = NULL;

	/* check that pdev is set */
	if (!(pdev = ltp_usb.pdev)) {
		printk("tusb: Cant find host controller pci_dev pointer\n");
		return 1;
	}

	/* make call and check return value */
	rc = usb_hcd_pci_suspend(pdev, (u32) 2);
	if (rc)
		printk("tusb: Suspend retval failure\n");
	else
		printk("tusb: Suspend success\n");

	return rc;
}

/*
 * test_hcd_resume
 *	make call to resume device for power management
 *	so that device will be active and able to use
 *	again
 */
static int test_hcd_resume()
{
	int rc;
	struct pci_dev *pdev = NULL;

	/* check that pdev is set */
	if (!(pdev = ltp_usb.pdev)) {
		printk("tusb: Cant find host controller pci_dev pointer\n");
		return 1;
	}

	/* make call and check return value */
	rc = usb_hcd_pci_resume(pdev);
	if (rc)
		printk("tusb: Resume got retval, failure\n");
	else
		printk("tusb: Resume success\n");

	return rc;
}

static int tusb_init_module(void)
{
	int rc;

	SET_MODULE_OWNER(&tusb_fops);

	rc = register_chrdev(Major, DEVICE_NAME, &tusb_fops);
	if (rc < 0) {
		printk("tusb: Failed to register tusb device\n");
		return rc;
	}

	if (Major == 0)
		Major = rc;

	printk("tusb: Registration success at major number %i\n", Major);
	return usb_register(&test_usb_driver);
}

static void tusb_exit_module(void)
{

	kfree(ltp_usb.dev);

#if 0
	usb_free_bus(ltp_usb.bus);
#endif

	unregister_chrdev(Major, DEVICE_NAME);

	usb_deregister(&test_usb_driver);
}

module_init(tusb_init_module)
    module_exit(tusb_exit_module)
