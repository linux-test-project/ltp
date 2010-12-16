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
 *

 * This pci and pci-express testing kernel module will allow test calls
 * to be driven through various ioctl calls in a
 * user space program that has attained the appropriate
 * file descriptor for this device. For the functions of
 * this module to work correctly there must be a pci / pci-express
 * device somewhere in the system. The tests do not need
 * a specific device, and the first pci device available
 * will be grabbed.
 *
 * author: Sean Ruyle (srruyle@us.ibm.com)
 * date:   5/20/2003
 * PCI-Express test scripts author: Amit Khanna (amit.khanna@intel.com)
 * date:   8/20/2004
 *
 * file:   tpci.c,
 * module: tpci
 */

#include <linux/config.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <asm/uaccess.h>
#include "tpci.h"
#include "st_tpci.h"

MODULE_AUTHOR("Sean Ruyle <srruyle@us.ibm.com>, Amit Khanna <amit.khanna@intel.com>");
MODULE_DESCRIPTION(TPCI_TEST_DRIVER_NAME);
MODULE_LICENSE("GPL");

static int tpci_ioctl (struct inode *, struct file *, unsigned int, unsigned long);
static int tpci_open (struct inode *, struct file *);
static int tpci_close (struct inode *, struct file *);

static int probe_pci_dev(void);
static int pci_enable(void);
static int pci_disable(void);
static int test_find_bus(void);
static int test_find_class(void);
static int test_find_device(void);
static int test_find_subsys(void);
static int test_scan_bus(void);
static int test_slot_scan(void);
static int test_bus_add_devices(void);
static int test_enable_bridges(void);
static int test_match_device(void);
static int test_reg_driver(void);
static int test_unreg_driver(void);
static int test_assign_resources(void);
static int test_save_state(void);
static int test_restore_state(void);
static int test_max_bus(void);
static int test_find_cap(void);
static int test_find_pci_exp_cap(void);
static int test_read_pci_exp_config(void);

static int Major = TPCI_MAJOR;
static tpci_user_t ltp_pci;

/*
 * File operations struct, to use operations find the
 * correct file descriptor
 */
static struct file_operations tpci_fops = {
	open : tpci_open,
	release: tpci_close,
	ioctl: tpci_ioctl,
};

/*
 * open and close operations
 */
static int tpci_open(struct inode *ino, struct file *f) {
	return 0;
}

static int tpci_close(struct inode *ino, struct file *f) {
	return 0;
}

/*
 * tpci_ioctl:
 * 	a user space program can drive the test functions
 *	through a call to ioctl once the correct file
 *	descriptor has been attained
 *
 * calls functions:
 *
 */
static int tpci_ioctl(struct inode *ino, struct file *f,
			unsigned int cmd, unsigned long l) {
	int 			rc;
	struct tpci_interface	tif;
	caddr_t			*inparms;
	caddr_t			*outparms;

	printk("Enter tpci_ioctl\n");

	inparms	= NULL;
	outparms = NULL;
	rc = 0;

	if (copy_from_user(&tif, (void *)l, sizeof(tif))) {
		/* Bad address */
		return(-EFAULT);
	}

	/*
	 * Setup inparms and outparms as needed
	 */
	if (tif.in_len > 0) {
		inparms = (caddr_t *)kmalloc(tif.in_len, GFP_KERNEL);
		if (!inparms) {
			return(-ENOMEM);
		}

		rc = copy_from_user(inparms, tif.in_data, tif.in_len);
		if (rc) {
			kfree(inparms);
			return(-EFAULT);
		}
	}
	if (tif.out_len > 0) {
		outparms = (caddr_t *)kmalloc(tif.out_len, GFP_KERNEL);
		if (!outparms) {
			kfree(inparms);
			return(-ENOMEM);
		}
	}

	/*
	 * determine which call to make on the cmd value
	 */
	switch(cmd) {
		case PCI_PROBE: 	rc = probe_pci_dev(); break;
		case PCI_ENABLE: 	rc = pci_enable(); break;
		case PCI_DISABLE:	rc = pci_disable(); break;
		case FIND_BUS:		rc = test_find_bus(); break;
		case FIND_CLASS:	rc = test_find_class(); break;
		case FIND_DEVICE:	rc = test_find_device(); break;
		case FIND_SUBSYS:	rc = test_find_subsys(); break;
		case BUS_SCAN:		rc = test_scan_bus(); break;
		case SLOT_SCAN:		rc = test_slot_scan(); break;
		case BUS_ADD_DEVICES:	rc = test_bus_add_devices(); break;
		case ENABLE_BRIDGES:	rc = test_enable_bridges(); break;
		case MATCH_DEVICE:	rc = test_match_device(); break;
		case REG_DRIVER:	rc = test_reg_driver(); break;
		case UNREG_DRIVER:	rc = test_unreg_driver(); break;
		case PCI_RESOURCES:	rc = test_assign_resources(); break;
		case SAVE_STATE:	rc = test_save_state(); break;
		case RESTORE_STATE:	rc = test_restore_state(); break;
		case TEST_MAX_BUS:	rc = test_max_bus(); break;
		case FIND_CAP:		rc = test_find_cap(); break;
		case FIND_PCI_EXP_CAP:  rc = test_find_pci_exp_cap(); break;
		case READ_PCI_EXP_CONFIG:  rc = test_read_pci_exp_config(); break;
		default:
			printk("Mismatching ioctl command\n");
			break;
	}

	if (!(ltp_pci.dev))
		printk("tpci: After ioctl call dev is NULL\n");

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
	if (copy_to_user((void*)l, &tif, sizeof(tif))) {
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
 * probe_pci_dev
 * 	find a pci device that can be used for other test
 * 	calls in this kernel module, select first device
 * 	that finds for use, do not need a specific device
 */
static int probe_pci_dev() {
	unsigned int i, j;
	struct pci_dev *dev = (struct pci_dev *)kmalloc(sizeof(struct pci_dev),GFP_KERNEL);
	struct pci_bus *bus = (struct pci_bus *)kmalloc(sizeof(struct pci_bus),GFP_KERNEL);

	/* Zero out the ltp_pci */
	memset(&ltp_pci, 0, sizeof(tpci_user_t));

	ltp_pci.dev = dev;
	ltp_pci.bus = bus;

	/* Probe until find a pci device */
	for (i = MAX_BUS; i > 0; i--) {
		for (j = MAX_DEVFN; j > 1; j--) {
			dev = pci_find_slot(i, j);
			if (dev && dev->driver) {
				printk("tpci: found pci_dev, bus %d, devfn %d\n", i, j);
				printk("Slot number: %d\n", dev->devfn );

				bus = dev->bus;
				printk("Bus number: %d\n", bus->number );

				/* copy data into ltp_pci struct */
				memcpy(ltp_pci.dev, dev, sizeof(struct pci_dev));
				memcpy(ltp_pci.bus, bus, sizeof(struct pci_bus));

				return 0;
			}
		}
	}

	/* if reaches here did not find a pci device */
	printk("tpci: failed to find pci device\n");
	return 1;
}

/*
 * pci_enable
 * 	enable a pci device so that it may be used in
 * 	later testing in the user test program
 */
static int pci_enable() {
	int rc = 0;

	struct pci_dev *dev = ltp_pci.dev;

	/* check if can enable the device pointer */
	if (!dev) {
		printk("tpci: dev is NULL\n");
		return 1;
	}

	if (pci_enable_device(dev)) {
		printk("tpci: failed to enable pci device\n");
		rc = 1;
	}
	else {
		printk("tpci: enabled pci device\n");
		rc = 0;
	}

	return rc;
}

/*
 * pci_disable
 *	call to pci_disable_device
 */
static int pci_disable() {
	int rc = 0;

	struct pci_dev *dev = ltp_pci.dev;

	/* check if device pointer exists */
	if (!dev) {
		printk("tpci: dev is NULL\n");
		return 1;
	}

	pci_disable_device(dev);

	if (dev->current_state == 4 || dev->current_state == 3) {
		printk("tpci: disabled pci device\n");
		rc = 0;
	}
	else {
		printk("tpci: failed to disable pci device\n");
		rc = 1;
	}

	return rc;
}

/*
 * find_bus
 *	call to pci_find_bus, use values from bus
 *	pointer in ltp_pci, make sure that returns
 * 	bus with same values
 */
static int test_find_bus() {
	int rc;
	int num = ltp_pci.bus->number;
	struct pci_bus *temp = NULL;

	temp = pci_find_bus(num);

	if (!temp) {
		printk("tpci: pci_find_bus failed to return bus pointer\n");
		rc = 1;
	}
	else if (temp->number != num) {
		printk("tpci: returned bus pointer w/ wrong bus number\n");
		rc = 1;
	}
	else {
		printk("tpci: success returned bus pointer \n");
		rc = 0;
	}

	return rc;
}

/*
 * find_class
 *	call to pci_find_class, using values from the
 *	pci_dev pointer in ltp_pci structure
 */
static int test_find_class() {
	int rc;
	unsigned int num = ltp_pci.dev->class;
	struct pci_dev *temp = NULL;

	temp = pci_find_class(num, NULL);

	if (!temp) {
		printk("tpci: failed to find pci device from class number\n");
		rc = 1;
	}
	else {
		printk("tpci: found pci device from class number\n");
		rc = 0;
	}

	return rc;
}

/*
 * find_device
 * 	call to pci_find_device, using values for
 *	parameters from pci_dev pointer in the
 *	ltp_pci structure
 */
static int test_find_device() {
	int rc;
	struct pci_dev *temp = NULL;
	unsigned short ven = ltp_pci.dev->vendor,
		       dev = ltp_pci.dev->device;

	temp = pci_find_device(ven, dev, NULL);

        if (!temp) {
                printk("tpci: failed to find pci device from device info\n");
                rc = 1;
        }
        else {
                printk("tpci: found pci device from device info\n");
                rc = 0;
        }

        return rc;
}

/*
 * find_subsys
 *	call to pci_find_subsys, use valued from
 *	pci_dev pointer in ltp_pci structure to
 *	find pci_dev from subsys info
 */
static int test_find_subsys() {
	int rc;
	struct pci_dev *temp = NULL;
	unsigned short  ven = ltp_pci.dev->vendor,
			dev = ltp_pci.dev->device,
			ss_ven = ltp_pci.dev->subsystem_vendor,
			ss_dev = ltp_pci.dev->subsystem_device;

	temp = pci_find_subsys(ven, dev, ss_ven, ss_dev, NULL);

	if (!temp) {
                printk("tpci: failed to find pci device from subsys info\n");
                rc = 1;
        }
        else {
                printk("tpci: found pci device from subsys info\n");
                rc = 0;
        }

        return rc;
}

/*
 * test_scan_bus
 *	call to pci_do_scan_bus,  which takes
 *	a struct pci_bus pointer, which will
 * 	return a an integer for how far the
 *	function got in scanning bus
 */
static int test_scan_bus() {
	int rc, num;
	struct pci_bus *bus = ltp_pci.bus;

	num = pci_do_scan_bus(bus);

	/*
	 * check if returned number is greater than
	 * max number of bus or less than 0
	 */
	if (num > MAX_BUS ||  num < 0) {
		printk("tpci: Failed scan bus\n");
		rc = 1;
	}
	else {
		printk("tpci: Success scan bus\n");
		rc = 0;
	}

	return rc;
}

/*
 * test_slot_scan
 *	make call to pci_scan_slot, which will
 *	find the device pointer and setup the
 *	device info
 */
static int test_slot_scan() {
	int rc, ret;
	int num = ltp_pci.dev->devfn;
	struct pci_bus *bus = ltp_pci.bus;

	ret = pci_scan_slot(bus, num);

	if (ret > 0) {
		printk("tpci: Found device from scan slot\n");
		rc = 0;
	}
	else {
		printk("tpci: Failed find device from scan slot\n");
		rc = 1;
	}

	return rc;
}

/*
 * test_bus_add_devices
 *	make call to pci_bus_add_devices,
 *	which will check the device pointer
 *	that is passed in for more devices
 *	that it can add
 */
static int test_bus_add_devices() {
	int rc;
	struct pci_bus *bus = ltp_pci.bus;

	pci_bus_add_devices(bus);

	if (bus) {
		printk("tpci: Called bus_add_device\n");
		rc = 0;
	}
	else {
		printk("tpci: bus_add_device failed\n");
		rc = 1;
	}

	return rc;
}

/*
 * test_enable_bridges
 *	make call to pci_enable_bridges,
 *	use bus pointer from the ltp_pci
 *	structure
 */
static int test_enable_bridges() {
	int rc;
	struct pci_bus *bus = ltp_pci.bus;

	pci_enable_bridges(bus);

	if (bus) {
                printk("tpci: Called enable bridges\n");
                rc = 0;
        }
        else {
                printk("tpci: enable_bridges failed\n");
                rc = 1;
        }

        return rc;
}

/*
 * test_match_device
 *	make call to pci_match_device, returns a
 *	pci_device_id pointer
 */
static int test_match_device() {
	int rc;
	struct pci_dev *dev = ltp_pci.dev;
	struct pci_driver *drv;
	const struct pci_device_id *id;

	drv = pci_dev_driver(dev);

	if (!drv) {
		printk("driver pointer not allocated for pci_dev\n");
		return 1;
	}

	id = pci_match_device(drv->id_table, dev);

	if (id) {
		printk("tpci: Match device success\n");
		rc = 0;
	}
	else {
		printk("tpci: Failed return pci_device_id \n");
		rc = 1;
	}

	return rc;
}

/*
 * test_reg_driver
 *	make call to pci_register_driver, which will
 *	register the driver for a device with the
 *	system
 */
static int test_reg_driver() {
	int rc, ret;
	struct pci_driver *drv = (struct pci_driver *)kmalloc(sizeof(struct pci_driver), GFP_KERNEL);
	struct pci_driver *tmp = ltp_pci.dev->driver;

	/* zero out drv structure */
	memset(drv, 0, sizeof(struct pci_driver));

	/* copy in structure of tmp, reset some fields */
	drv->name = "Tmod_driver";
	drv->driver = tmp->driver;

	/* copy structure into ltp_pci.drv */
	ltp_pci.drv = drv;
	memcpy(ltp_pci.drv, drv, sizeof(struct pci_driver));

	if (!drv) {
		printk("tpci: Device does not have a driver pointer\n");
		return 1;
	}

	ret = pci_register_driver(drv);

	if (ret) {
		printk("tpci: Success driver register\n");
		rc = 0;
	}
	else {
		rc = 1;
		printk("tpci: unsuccessful registering pci driver\n");
	}

	return rc;
}

/*
 * test_unreg_driver
 *	make call to pci_unregister_driver, which will
 *	unregister the driver for a device from the system
 */
static int test_unreg_driver() {
	int rc;
        struct pci_driver *drv = ltp_pci.drv;

        if (!drv) {
                printk("tpci: Device does not have a driver pointer\n");
                return 1;
        }

        pci_unregister_driver(drv);
        if (!drv) {
                printk("tpci: Unsuccesful driver unregister\n");
		rc = 1;
	}
        else {
                printk("tpci: unregistering pci driver\n");
		rc = 0;
	}

        return rc;
}

/*
 * test_assign_resources
 *	make calls to pci_assign_resource, will need
 *	to setup a dev pointer and resource pointer,
 */
static int test_assign_resources() {
	int rc;
	struct pci_dev *dev = ltp_pci.dev;
	int resno;

	for (resno = 0; resno < 7; resno++) {
		struct resource *r = dev->resource +resno;
		if (r->flags)
			pci_assign_resource(dev, resno);
	}

	/*
	 * enable device after call to assign resource
	 * because might error if (!r->start && r->end)
	*/
	rc = pci_enable_device(dev);

	return rc;
}

/*
 * test_save_state
 *	make call to pci_save_state, takes in a u32*
 * 	buffer
 */
static int test_save_state() {
	int rc;
	u32 *buffer = ltp_pci.state;
	struct pci_dev *dev = ltp_pci.dev;

	rc = pci_save_state(dev, buffer);
	if (rc)
		printk("tpci: Failed save state\n");
	else
		printk("tpci: Saved state of device\n");

	return rc;
}

/*
 * test_restore_state
 *	make call to pci_restore_state, get the state buffer
 *	should have been previously filled out by save state
 */
static int test_restore_state() {
	int rc;
	u32 *buffer = ltp_pci.state;
	struct pci_dev *dev = ltp_pci.dev;

	rc = pci_restore_state(dev, buffer);
        if (rc)
                printk("tpci: Failed restore state\n");
        else
                printk("tpci: Restored state of device\n");

        return rc;
}

/*
 * test_max_bus
 *	make call to pci_max_busnr, which will determine
 *	the max number of bus on the system
 */
static int test_max_bus() {
	int rc, ret;

	ret = pci_max_busnr();
	if (ret) {
		printk("Found max busnr\n");
		rc = 0;
	}
	else {
		printk("Did not return max busnr\n");
		rc = 1;
	}

	return rc;
}

/*
 * test_find_cap
 *	make call to pci_find_capability, which
 *	will determine if a device has a certain
 *	capability, use second parameter to specify
 *	which capability you are looking for
 */
static int test_find_cap() {
	int rc;
	struct pci_dev *dev = ltp_pci.dev;

	rc = pci_find_capability(dev, PCI_CAP_ID_PM);
	if (rc)
		printk("tpci: Does not have tested capability\n");
	else
		printk("tpci: Device has PM capability\n");

	return rc;
}

/*
 * test_find_pci_exp_cap
 *	make call to pci_find_capability, which will
 *  determine if a device has PCI-EXPRESS capability,
 *  use second parameter to specify which capability
 *  you are looking for
 */
static int test_find_pci_exp_cap() {
	int rc;
	struct pci_dev *dev = ltp_pci.dev;

	rc = pci_find_capability(dev, PCI_CAP_ID_EXP);
	if (rc)
	 	printk("tpci: Device has PCI-EXP capability\n");
	else
		printk ("tpci: Device doesn't have PCI-EXP capability\n");
	return rc;
}

/*
 * test_read_pci_exp_config
 *	make call to pci_config_read and determine if
 *  the PCI-Express enhanced config space of this
 *  device can be read successfully.
 */
static int test_read_pci_exp_config() {
	int rc;
	int reg= 100, len= 4; /*PCI-Exp enhanced config register 0x100, 4 implies dword access*/
	struct pci_dev *dev = ltp_pci.dev;

	u32 data, *value;

	printk("tpci: Device(%d) on bus(%d) & slot(%d) \n",dev,dev->bus->number,dev->devfn);
	printk("tpci: Reading the PCI Express configuration registers---\n");

	printk("tpci: Reading PCI-Express AER CAP-ID REGISTER at Enh-Cfg AddrSpace 0x100\n");

	rc = pci_config_read(0, dev->bus->number, PCI_SLOT(dev->devfn), PCI_FUNC(dev->devfn), reg        , len, &data);

	*value = (u32)data;

	if (*value==AER_CAP_ID_VALUE) /*comparing the value read with AER_CAP_ID_VALUE macro */
	printk("tpci: \nCorrect value read using PCI-Express driver installed\n\n");
	else
	printk("tpci: \nIncorrect value read. PCI-Express driver/device not installed\n\n");

	return rc;
}

/*
 * tpci_init_module
 * 	set the owner of tpci_fops, register the module
 * 	as a char device, and perform any necessary
 * 	initialization for pci devices
 */
static int tpci_init_module(void) {
	int rc;

	SET_MODULE_OWNER(&tpci_fops);

	rc = register_chrdev(Major, DEVICE_NAME, &tpci_fops);
	if (rc < 0) {
		printk("tpci: Failed to register device.\n");
		return rc;
	}

	if (Major == 0)
		Major = rc;

	printk("tpci: Registration success.\n");
	return 0;
}

/*
 * tpci_exit_module
 * 	unregister the device and any necessary
 * 	operations to close for pci devices
 */
static void tpci_exit_module(void) {
	int rc;

	kfree(ltp_pci.dev);
	kfree(ltp_pci.bus);
	kfree(ltp_pci.drv);

	rc = unregister_chrdev(Major, DEVICE_NAME);
	if (rc < 0)
		printk("tpci: unregister failed\n");
	else
		printk("tpci: unregister success\n");

}

module_init(tpci_init_module)
module_exit(tpci_exit_module)