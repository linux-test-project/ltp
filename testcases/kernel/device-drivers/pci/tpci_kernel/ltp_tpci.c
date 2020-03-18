/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>

#include "tpci.h"

MODULE_AUTHOR("Sean Ruyle <srruyle@us.ibm.com>");
MODULE_AUTHOR("Amit Khanna <amit.khanna@intel.com>");
MODULE_AUTHOR("Copyright (c) 2013 Oracle and/or its affiliates");
MODULE_DESCRIPTION("LTP PCI Test");
MODULE_LICENSE("GPL");

#define prk_err(fmt, ...) \
	pr_err(PCI_DEVICE_NAME ": " fmt "\n", ##__VA_ARGS__)
#define prk_info(fmt, ...) \
	pr_info(PCI_DEVICE_NAME ": " fmt "\n", ##__VA_ARGS__)
#define prk_debug(fmt, ...) \
	pr_debug(PCI_DEVICE_NAME ": " fmt "\n", ##__VA_ARGS__)

#define TPASS	0
#define TFAIL	1
#define TSKIP	32

static const struct pci_device_id ltp_pci_tbl[] = {
	{ PCI_DEVICE(PCI_ANY_ID, PCI_ANY_ID) },
	{ 0, }
};

MODULE_DEVICE_TABLE(pci, ltp_pci_tbl);

static int ltp_pci_probe(struct pci_dev *pci_dev,
	const struct pci_device_id *pci_ent)
{
	return 0;
}

static struct pci_driver ltp_pci_driver = {
	.name     = "LTP_PCI_DRIVER",
	.id_table = ltp_pci_tbl,
	.probe    = ltp_pci_probe,
};

static int pci_registered;

struct tpci_user {
	struct pci_dev		*dev;
	struct pci_bus		*bus;
	struct pci_driver	*drv;
	uint32_t		state[16];
};
static struct tpci_user ltp_pci;

/*
 * probe_pci_dev
 *	find a pci device that can be used for other test
 *	calls in this kernel module.
 */
static int probe_pci_dev(unsigned int bus, unsigned int slot)
{
	struct pci_dev *dev;

	if (ltp_pci.dev) {
		pci_dev_put(ltp_pci.dev);
		ltp_pci.dev = NULL;
	}

	dev = pci_get_domain_bus_and_slot(0, bus, slot);
	if (!dev || !dev->driver)
		return -ENODEV;

	prk_info("found pci_dev '%s', bus %u, devfn %u",
		pci_name(dev), bus, slot);

	ltp_pci.dev = dev;
	ltp_pci.bus = dev->bus;
	prk_info("Bus number: %d", dev->bus->number);
	return 0;
}

/*
 * pci_enable
 *	enable a pci device so that it may be used in
 *	later testing in the user test program
 */
static int pci_enable(void)
{
	struct pci_dev *dev = ltp_pci.dev;

	prk_info("enable pci device");

	/* check if can enable the device pointer */
	if (!dev) {
		prk_err("dev is NULL");
		return TFAIL;
	}

	if (pci_enable_device(dev)) {
		prk_err("failed to enable pci device");
		return TFAIL;
	}

	prk_info("enabled pci device");
	return TPASS;
}

static int pci_disable(void)
{
	struct pci_dev *dev = ltp_pci.dev;

	prk_info("disable pci device");

	/* check if device pointer exists */
	if (!dev) {
		prk_err("dev is NULL");
		return TFAIL;
	}

	prk_info("is pci enabled '%d', is managed '%d'",
		pci_is_enabled(dev), pci_is_managed(dev));

	pci_release_regions(dev);
	pci_disable_device(dev);

	if (dev->current_state == PCI_D3hot ||
		dev->current_state == PCI_D3cold) {

		prk_info("disabled pci device, state '%s'",
			pci_power_name(dev->current_state));
		return TPASS;

	}

	prk_err("failed to disable pci device, state '%s'",
		pci_power_name(dev->current_state));
	return TFAIL;
}

/*
 * find_bus
 *	call to pci_find_bus, use values from bus
 *	pointer in ltp_pci, make sure that returns
 *	bus with same values
 */
static int test_find_bus(void)
{
	int num = ltp_pci.bus->number;
	struct pci_bus *temp = NULL;

	prk_info("find bus");

	temp = pci_find_bus(pci_domain_nr(ltp_pci.bus), num);

	if (!temp) {
		prk_info("pci_find_bus failed");
		return TFAIL;
	} else if (temp->number != num) {
		prk_err("returned bus pointer w/ wrong bus number");
		return TFAIL;
	}

	prk_info("success returned bus pointer");
	return TPASS;
}

/*
 * find_class
 *	call to pci_find_class, using values from the
 *	pci_dev pointer in ltp_pci structure
 */
static int test_find_class(void)
{
	unsigned int num = ltp_pci.dev->class;
	struct pci_dev *temp = NULL;

	prk_info("find pci class");

	temp = pci_get_class(num, NULL);

	if (!temp) {
		prk_err("failed to find pci device from class number");
		return TFAIL;
	}

	prk_info("found pci device from class number");
	pci_dev_put(temp);

	return TPASS;
}

/*
 * find_device
 *	call to pci_find_device, using values for
 *	parameters from pci_dev pointer in the
 *	ltp_pci structure
 */
static int test_find_device(void)
{
	struct pci_dev *temp = NULL;
	unsigned short ven = ltp_pci.dev->vendor, dev = ltp_pci.dev->device;

	prk_info("get pci device");

	temp = pci_get_device(ven, dev, NULL);

	if (!temp) {
		prk_err("failed to find pci device from device info");
		return TFAIL;
	}

	prk_info("found pci device from device info");
	pci_dev_put(temp);

	return TPASS;
}

/*
 * find_subsys
 *	call to pci_find_subsys, use valued from
 *	pci_dev pointer in ltp_pci structure to
 *	find pci_dev from subsys info
 */
static int test_find_subsys(void)
{
	struct pci_dev *temp;
	unsigned short ven = ltp_pci.dev->vendor,
		dev = ltp_pci.dev->device,
		ss_ven = ltp_pci.dev->subsystem_vendor,
		ss_dev = ltp_pci.dev->subsystem_device;

	prk_info("get pci subsys");
	temp = pci_get_subsys(ven, dev, ss_ven, ss_dev, NULL);

	if (!temp) {
		prk_err("failed to find pci device from subsys info");
		return TFAIL;
	}

	prk_info("found pci device from subsys info");
	pci_dev_put(temp);

	return TPASS;
}

/*
 * test_scan_bus
 *	call to pci_do_scan_bus,  which takes
 *	a struct pci_bus pointer, which will
 *	return an integer for how far the
 *	function got in scanning bus
 */
static int test_scan_bus(void)
{
#ifdef CONFIG_HOTPLUG
	int num;
	struct pci_bus *bus = ltp_pci.bus;

	prk_info("scan pci bus");

	num = pci_rescan_bus(bus);
	/*
	 * check if returned number is greater than
	 * max number of bus or less than 0
	 */
	if (num > MAX_BUS || num < 0) {
		prk_err("failed scan bus");
		return TFAIL;
	}
	prk_info("success scan bus");
	return TPASS;
#else
	prk_info("pci_rescan_bus() is not supported");
	return TSKIP;
#endif
}

/*
 * test_slot_scan
 *	make call to pci_scan_slot, which will
 *	find the device pointer and setup the
 *	device info
 */
static int test_slot_scan(void)
{
	int ret, num = ltp_pci.dev->devfn;
	struct pci_bus *bus = ltp_pci.bus;

	prk_info("scan pci slot");

	ret = pci_scan_slot(bus, num);
	if (ret >= 0) {
		prk_info("found '%d' devices from scan slot", ret);
		return TPASS;
	}

	prk_err("pci_scan_slot failed");
	return TFAIL;
}

/*
 * test_bus_add_devices
 *	make call to pci_bus_add_devices,
 *	which will check the device pointer
 *	that is passed in for more devices
 *	that it can add
 */
static int test_bus_add_devices(void)
{
	struct pci_bus *bus = ltp_pci.bus;

	prk_info("add bus device");

	pci_bus_add_devices(bus);

	if (bus) {
		prk_info("called bus_add_device");
		return TPASS;
	}

	prk_err("bus_add_device failed");
	return TFAIL;
}

/*
 * test_match_device
 *	make call to pci_match_device, returns a
 *	pci_device_id pointer
 */
static int test_match_device(void)
{
	struct pci_dev *dev = ltp_pci.dev;
	struct pci_driver *drv;
	const struct pci_device_id *id;

	prk_info("test pci_device_id()");

	drv = pci_dev_driver(dev);

	if (!drv) {
		prk_err("driver pointer not allocated for pci_dev");
		return TFAIL;
	}

	id = pci_match_id(drv->id_table, dev);

	if (id) {
		prk_info("match device success");
		return TPASS;
	}

	prk_err("failed return pci_device_id");
	return TFAIL;
}


/*
 * test_reg_driver
 *	make call to pci_register_driver, which will
 *	register the driver for a device with the
 *	system
 */
static int test_reg_driver(void)
{
	prk_info("test pci_register_driver");
	if (pci_register_driver(&ltp_pci_driver)) {
		prk_err("unsuccessful registering pci driver");
		return TFAIL;
	}
	pci_registered = 1;
	prk_info("success driver register");
	return TPASS;
}

/*
 * test_unreg_driver
 *	make call to pci_unregister_driver, which will
 *	unregister the driver for a device from the system
 */
static int test_unreg_driver(void)
{
	pci_unregister_driver(&ltp_pci_driver);
	pci_registered = 0;
	return TPASS;
}

/*
 * test_assign_resources
 *	make calls to pci_assign_resource, will need
 *	to setup a dev pointer and resource pointer,
 */
static int test_assign_resources(void)
{
	int i, ret, rc = 0;
	struct pci_dev *dev = ltp_pci.dev;
	struct resource *r;

	prk_info("assign resources");

	for (i = 0; i < 7; ++i) {
		prk_info("assign resource #%d", i);
		r = &dev->resource[i];
		prk_info("name = %s, flags = %lu, start 0x%lx, end 0x%lx",
			r->name, r->flags,
			(unsigned long)r->start, (unsigned long)r->end);

		if (r->flags & IORESOURCE_MEM &&
			r->flags & IORESOURCE_PREFETCH) {
			ret = pci_assign_resource(dev, i);
			prk_info("assign resource to '%d', ret '%d'", i, ret);
			rc |= (ret < 0 && ret != -EBUSY) ? TFAIL : TPASS;
		}
	}

	/*
	 * enable device after call to assign resource
	 * because might error if (!r->start && r->end)
	 */
	if (pci_enable_device(dev))
		return TFAIL;

	return rc;
}

/*
 * test_save_state
 *	make call to pci_save_state, takes in a u32*
 *	buffer
 */
static int test_save_state(void)
{
	struct pci_dev *dev = ltp_pci.dev;

	prk_info("save state");

	if (pci_save_state(dev)) {
		prk_err("failed save state");
		return TFAIL;
	}

	prk_info("saved state of device");
	return TPASS;
}

/*
 * test_restore_state
 *	make call to pci_restore_state, get the state buffer
 *	should have been previously filled out by save state
 */
static int test_restore_state(void)
{
	struct pci_dev *dev = ltp_pci.dev;

	prk_info("restore state");

	pci_restore_state(dev);

	return TPASS;
}

/*
 * test_find_cap
 *	make call to pci_find_capability, which
 *	will determine if a device has a certain
 *	capability, use second parameter to specify
 *	which capability you are looking for
 */
static int test_find_cap(void)
{
	struct pci_dev *dev = ltp_pci.dev;

	prk_info("find device capability");

	if (pci_find_capability(dev, PCI_CAP_ID_PM))
		prk_info("does not have tested capability");
	else
		prk_info("device has PM capability");

	return TPASS;
}

/*
 * test_read_pci_exp_config
 *	make call to pci_config_read and determine if
 *	the PCI-Express enhanced config space of this
 *	device can be read successfully.
 */
static int test_read_pci_exp_config(void)
{
	int pos;
	u32 header;
	struct pci_dev *dev = ltp_pci.dev;

	/* skip the test if device doesn't have PCIe capability */
	pos = pci_pcie_cap(dev);
	if (!pos) {
		prk_info("device doesn't have PCI-EXP capability");
		return TSKIP;
	}
	prk_info("read the PCI Express configuration registers at 0x%x", pos);

	if (pci_read_config_dword(dev, pos, &header)) {
		prk_err("failed to read config dword");
		return TFAIL;
	}

	/* comparing the value read with PCI_CAP_ID_EXP macro */
	if ((header & 0x000000ff) == PCI_CAP_ID_EXP) {
		prk_info("correct val read using PCIE driver installed: 0x%x",
			header);
		return TPASS;
	}

	prk_err("incorrect val read. PCIE driver/device not installed: 0x%x",
		header);
	return TFAIL;
}

static int test_case(unsigned int cmd)
{
	int rc = TSKIP;

	switch (cmd) {
	case PCI_ENABLE:
		rc = pci_enable();
		break;
	case PCI_DISABLE:
		rc = pci_disable();
		break;
	case FIND_BUS:
		rc = test_find_bus();
		break;
	case FIND_CLASS:
		rc = test_find_class();
		break;
	case FIND_DEVICE:
		rc = test_find_device();
		break;
	case FIND_SUBSYS:
		rc = test_find_subsys();
		break;
	case BUS_SCAN:
		rc = test_scan_bus();
		break;
	case SLOT_SCAN:
		rc = test_slot_scan();
		break;
	case BUS_ADD_DEVICES:
		rc = test_bus_add_devices();
		break;
	case MATCH_DEVICE:
		rc = test_match_device();
		break;
	case REG_DRIVER:
		rc = test_reg_driver();
		break;
	case UNREG_DRIVER:
		rc = test_unreg_driver();
		break;
	case PCI_RESOURCES:
		rc = test_assign_resources();
		break;
	case SAVE_STATE:
		rc = test_save_state();
		break;
	case RESTORE_STATE:
		rc = test_restore_state();
		break;
	case FIND_CAP:
		rc = test_find_cap();
		break;
	case PCI_EXP_CAP_CONFIG:
		rc = test_read_pci_exp_config();
		break;
	default:
		prk_info("mismatching test-case command %d", cmd);
		break;
	}

	return rc;
}

/*
 * Test-case result,
 * if test is passed, value will be set to 0
 */
static int test_result;

static void device_release(struct device *dev)
{
	prk_info("device released\n");
}

static struct device tdev = {
	.init_name	= PCI_DEVICE_NAME,
	.release	= device_release,
};

/* print test result to sysfs file */
static ssize_t sys_result(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", test_result);
}
static DEVICE_ATTR(result, S_IRUSR, sys_result, NULL);

static ssize_t sys_tcase(struct device *dev,
	struct device_attribute *attr,  const char *buf, size_t count)
{
	int tc = 0;

	sscanf(buf, "%d", &tc);
	prk_info("test-case %d", tc);

	test_result = test_case(tc);

	return count;
}
static DEVICE_ATTR(tcase, S_IWUSR, NULL, sys_tcase);

static ssize_t sys_bus_slot(struct device *dev,
	struct device_attribute *attr,  const char *buf, size_t count)
{
	unsigned int res, bus, slot;
	int ret;

	sscanf(buf, "%u", &res);

	bus = res >> 8 & 0xFF;
	slot = res & 0xFF;

	ret = probe_pci_dev(bus, slot);
	if (ret)
		return ret;

	return count;
}
static DEVICE_ATTR(bus_slot, S_IWUSR, NULL, sys_bus_slot);

static int tpci_init_module(void)
{
	int err = 0;
	prk_info("Starting module");

	err = device_register(&tdev);
	if (err) {
		prk_err("Unable to register device");
		goto err0;
	}
	prk_info("device registered\n");

	err = device_create_file(&tdev, &dev_attr_result);
	if (err) {
		prk_err("Can't create sysfs file 'result'");
		goto err1;
	}

	err = device_create_file(&tdev, &dev_attr_tcase);
	if (err) {
		prk_err(": Can't create sysfs file 'tc'");
		goto err2;
	}

	err = device_create_file(&tdev, &dev_attr_bus_slot);
	if (err) {
		prk_err(": Can't create sysfs file 'bus_slot'");
		goto err3;
	}

	return 0;

err3:
	device_remove_file(&tdev, &dev_attr_tcase);
err2:
	device_remove_file(&tdev, &dev_attr_result);
err1:
	device_unregister(&tdev);
err0:
	return err;
}
module_init(tpci_init_module)

static void tpci_exit_module(void)
{
	prk_debug("Unloading module\n");
	if (ltp_pci.dev)
		pci_dev_put(ltp_pci.dev);

	if (pci_registered)
		pci_unregister_driver(&ltp_pci_driver);

	device_remove_file(&tdev, &dev_attr_result);
	device_remove_file(&tdev, &dev_attr_tcase);
	device_remove_file(&tdev, &dev_attr_bus_slot);
	device_unregister(&tdev);
}
module_exit(tpci_exit_module)
