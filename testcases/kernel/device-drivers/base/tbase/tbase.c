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
 * This test module is for executing and testing
 * the kernel code from drivers/base. This module
 * is driven by a user space program through
 * calls to the ioctl
 *
 * author: Sean Ruyle
 * date:   07/14/2003
 *
 * module: tbase
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/sysdev.h>
#include <asm/uaccess.h>

#include "tbase.h"
#include "str_mod.h"

MODULE_AUTHOR("Sean Ruyle <srruyle@us.ibm.com>");
MODULE_DESCRIPTION(TMOD_DRIVER_NAME);
MODULE_LICENSE("GPL");

static int tbase_ioctl(struct inode *, struct file *, unsigned int,
		       unsigned long);
static int tbase_open(struct inode *, struct file *);
static int tbase_close(struct inode *, struct file *);

static int test_device_register(void);
static int test_device_unregister(void);
static int test_bus_add(void);
static int test_get_drv(void);
static int test_put_drv(void);
static int test_reg_firm(void);
static int test_create_file(void);
static int test_dev_suspend(void);
static int test_dev_file(void);
static int test_bus_rescan(void);
static int test_bus_file(void);
static int test_class_reg(void);
static int test_class_get(void);
static int test_class_file(void);
static int test_classdev_reg(void);
static int test_classint_reg(void);
static int test_sysdev_cls_reg(void);
static int test_sysdev_reg(void);

static int Major = TBASEMAJOR;
static ltpmod_user_t ltp_mod;

/*
 * File operations struct, to use operations find the
 * correct file descriptor
 */
static struct file_operations tbase_fops = {
open:	tbase_open,
release:tbase_close,
ioctl:	tbase_ioctl,
};

static int tbase_open(struct inode *ino, struct file *f)
{
	return 0;
}

static int tbase_close(struct inode *ino, struct file *f)
{
	return 0;
}

/* my bus stuff */
struct device_driver test_driver;
struct device test_device;

static int test_device_match(struct device *dev, struct device_driver *drv)
{

	printk("tbase: driver is %s\n", drv->name);
//      printk("tbase: device is %s\n", dev->name);

	if (drv == &test_driver && dev == &test_device) {
		printk("tbase: match\n");
		return 1;
	} else {
		printk("tbase: no match\n");
		return 0;
	}

}

struct bus_type test_bus_type = {
	.name = "test_bus",
	.match = test_device_match,
};

/* my driver stuff */
int test_dev_probe(struct device *dev)
{
	printk("tbase: Entered test_dev_probe\n");
	return 0;
}

int test_dev_remove(struct device *dev)
{
	printk("tbase: Entered test_dev_remove\n");
	return 0;
}

struct device_driver test_driver = {
	.name = "TestDriver",
	.bus = &test_bus_type,
	.probe = test_dev_probe,
	.remove = test_dev_remove,
};

/* my device stuff */
struct device test_device = {
//      .name = "TestDevice",
	.bus = &test_bus_type,
	.bus_id = "test_bus",
};

/* my class stuff */
static void test_class_release(struct class_device *class_dev)
{
	printk("tbase: Entered test_class_release\n");
}

int test_class_hotplug(struct class_device *dev, char **envp,
		       int num_envp, char *buffer, int buffer_size)
{
	printk("tbase: Entered test_class_hotplug\n");
	return 0;
}

struct class test_class = {
	.name = "TestClass",
	.hotplug = test_class_hotplug,
	.release = test_class_release,
};

/* my class device stuff */
struct class_device test_class_dev = {
	.class_id = "test_bus",
	.dev = &test_device,
	.class = &test_class,
};

/* my class interface stuff */
int test_intf_add(struct class_device *class_dev)
{
	printk("tbase: Entered test_intf_add for the test class_interface\n");
	return 0;
}

void test_intf_rem(struct class_device *class_dev)
{
	printk("tbase: Entered test_intf_rem for the test class interface\n");
}

struct class_interface test_interface = {
	.class = &test_class,
	.add = &test_intf_add,
	.remove = &test_intf_rem,
};

/* my sys_device stuff */
int test_resume(struct sys_device *dev)
{
	printk("tbase: Entered test resume for sys device\n");
	return 0;
}

struct sysdev_class test_sysclass = {
	set_kset_name("TestSysclass"),
	.resume = test_resume,
};

struct sys_device test_sys_device = {
	.id = 0,
	.cls = &test_sysclass,
};

/* my attribute stuff */
static inline ssize_t
store_new_id(struct device_driver *driver, const char *buf, size_t count)
{
	printk("tbase: Entered store new id\n");
	return count;
}

/* create attribute driver_attr_new_id */
DRIVER_ATTR(new_id, 0200, NULL, store_new_id);

/* create attribute dev_attr_test_id */
DEVICE_ATTR(test_id, S_IRUGO, NULL, NULL);

/* create attribute bus_attr_test_id */
BUS_ATTR(test_id, S_IRUGO, NULL, NULL);

/* create attribute class_attr_test_id */
CLASS_ATTR(test_id, 0644, NULL, NULL);

/* create attribute class_device_attr_test_id */
CLASS_DEVICE_ATTR(test_id, 0644, NULL, NULL);

/*
 * tbase_ioctl:
 *      a user space program can drive the test functions
 *      through a call to ioctl once the correct file
 *      descriptor has been attained
 */
static int tbase_ioctl(struct inode *ino, struct file *f,
		       unsigned int cmd, unsigned long l)
{
	int rc;
	tmod_interface_t tif;
	caddr_t *inparms;
	caddr_t *outparms;

	printk("Enter tbase_ioctl\n");

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
	case REG_DEVICE:
		rc = test_device_register();
		break;
	case UNREG_DEVICE:
		rc = test_device_unregister();
		break;
	case BUS_ADD:
		rc = test_bus_add();
		break;
	case GET_DRV:
		rc = test_get_drv();
		break;
	case PUT_DRV:
		rc = test_put_drv();
		break;
	case REG_FIRM:
		rc = test_reg_firm();
		break;
	case CREATE_FILE:
		rc = test_create_file();
		break;
	case DEV_SUSPEND:
		rc = test_dev_suspend();
		break;
	case DEV_FILE:
		rc = test_dev_file();
		break;
	case BUS_RESCAN:
		rc = test_bus_rescan();
		break;
	case BUS_FILE:
		rc = test_bus_file();
		break;
	case CLASS_REG:
		rc = test_class_reg();
		break;
	case CLASS_UNREG:
		class_unregister(&test_class);
		break;
	case CLASS_GET:
		rc = test_class_get();
		break;
	case CLASS_FILE:
		rc = test_class_file();
		break;
	case CLASSDEV_REG:
		rc = test_classdev_reg();
		break;
	case CLASSINT_REG:
		rc = test_classint_reg();
		break;
	case SYSDEV_CLS_REG:
		rc = test_sysdev_cls_reg();
		break;
	case SYSDEV_CLS_UNREG:
		sysdev_class_unregister(&test_sysclass);
		break;
	case SYSDEV_REG:
		rc = test_sysdev_reg();
		break;
	case SYSDEV_UNREG:
		sys_device_unregister(&test_sys_device);
		break;
	default:
		printk("tbase: Mismatching ioctl command\n");
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
			printk
			    ("tbase: Unsuccessful copy_to_user of outparms\n");
			rc = -EFAULT;
		}
	}

	/* copy tif structure into l so that can be used by user program */
	if (copy_to_user((void *)l, &tif, sizeof(tif))) {
		printk("tbase: Unsuccessful copy_to_user of tif\n");
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
 * test_device_register
 *	makes call to device register passing in
 *	the device pointer that we found in a previos
 *	function, returns an error code
 */
static int test_device_register()
{
	struct device *dev = ltp_mod.dev;
	struct device_driver *drv = dev->driver;

	/* check if device register returns an error */
	if (device_register(dev)) {
		printk("tbase: Device not registered\n");
		return 1;
	} else
		printk("tbase: Device registered\n");

	driver_unregister(drv);

	/* check if driver_register returns an error */
	if (driver_register(drv)) {
		printk("tbase: Driver not registered\n");
		return 1;
	} else
		printk("tbase: Driver registered\n");

	return 0;

}

/*
 * test_device_unregister
 * 	make test call to device_unregister which
 * 	will in turn make calls that will decrememnt
 * 	the reference count and clean up as required
 */
static int test_device_unregister()
{
	struct device *dev = ltp_mod.dev;

	/* increment reference count */
	get_device(dev);

	/* reset remove pointer */
	if (dev->driver->remove)
		dev->driver->remove = NULL;

	device_unregister(dev);
	//check that reference count is smaller by one

	return 0;
}

/*
 * test_bus_add
 *	make call to bus_add_device, which will
 *	in turn add the device that is passed in
 *	to the bus
 */
static int test_bus_add()
{
	/* check if device register returns an error */
	if (bus_add_device(&test_device)) {
		printk("tbase: Device not added to bus\n");
		return 1;
	} else {
		printk("tbase: Device added to bus\n");
		return 0;
	}
}

/*
 * test_get_drv
 *	make test call to get_driver which should
 *	return a pointer to the driver passed in
 *	and increase the reference count to that
 *	kobject
 */
static int test_get_drv()
{
	int a, rc;
	struct device_driver *drv = &test_driver, *tmp = NULL;

	/* get reference count before test call */
	a = atomic_read(&drv->kobj.refcount);

	/* make test call */
	if ((tmp = get_driver(drv))) {
		rc = 0;
		printk("tbase: get driver returned driver\n");
	} else {
		rc = 1;
		printk("tbase: get driver failed to return driver\n");
	}

	/* check reference count */
	if ((a == (atomic_read(&drv->kobj.refcount) - 1))) {
		rc = 0;
		printk("tbase: correctly set ref count get driver\n");
	} else {
		rc = 1;
		printk("tbase: incorrect ref count get driver\n");
	}

	return rc;
}

/*
 * test_class_get
 *	make test call to class_get which should return
 *	a pointer to the class passed in and increase
 *	the reference count to that kobject
 */
static int test_class_get()
{
	int rc;
	struct class *tmp = NULL;

	/* get reference count before test call */
	tmp = class_get(&test_class);
	if (tmp == &test_class) {
		printk("tbase: Success get class\n");
		rc = 0;
	} else {
		printk("tbase: Failure get class\n");
		rc = 1;
	}

	class_put(&test_class);
	return rc;
}

/*
 * test_put_drv
 *      make test call to put_driver which should
 *      decrease the reference count to the kobject
 *      pointer in the driver structure
 */
static int test_put_drv()
{
	int a, rc;
	struct device_driver *drv = &test_driver;

	/* get reference count before test call */
	a = atomic_read(&drv->kobj.refcount);

	/* make test call */
	put_driver(drv);

	/* check reference count */
	if ((a == (atomic_read(&drv->kobj.refcount) + 1))) {
		rc = 0;
		printk("tbase: correctly set ref count put driver\n");
	} else {
		rc = 1;
		printk("tbase: incorrect ref count put driver\n");
	}

	return rc;
}

/*
 * test_reg_firm
 *	test call to register_firmware, which will
 *	register the subsystem, takes in a struct
 *	subsystem pointer, we can use our bus pointer
 *	that should have been found in a previous test
 *	to pass in a subsystem pointer, returns an
 *	error code
 */
static int test_reg_firm()
{
	struct subsystem *subsys = NULL;

	/* check pointer exists */
	if (!(subsys = &test_bus_type.subsys)) {
		printk("tbase: subsys pointer not set in reg firmware\n");
		return 1;
	}

	/* unregiser firmware */
	firmware_unregister(subsys);

	/* make test call */
	if (firmware_register(subsys)) {
		printk("tbase: failed register firmware\n");
		return 1;
	} else {
		printk("tbase: regsitered firmware\n");
		return 0;
	}

}

/*
 * test_create_file
 *	make test call to create sysfs file for the
 *	driver and if that call is successful then
 *	make a call to remove the file
 */
static int test_create_file()
{
	struct device_driver *drv = &test_driver;

	if (driver_create_file(drv, &driver_attr_new_id)) {
		printk("tbase: failed create sysfs file\n");
		return 1;
	} else {
		printk("tbase: created sysfs file\n");
		driver_remove_file(drv, &driver_attr_new_id);
		return 0;
	}

}

/*
 * test_dev_suspend
 *	make test call to device_suspend and
 *	if that call is successful then make
 *	a call to device_resume
 */
static int test_dev_suspend()
{
	int error = 0;

	error = device_suspend(SUSPEND_SAVE_STATE);
	if (error)
		printk("tbase: Failed on device suspend call\n");
	else {
		printk("tbase: Successful on device suspend call\n");
		device_resume();
	}

	error = device_suspend(SUSPEND_DISABLE);
	if (error)
		printk("tbase: Failed on device suspend call\n");
	else {
		printk("tbase: Successful on device suspend call\n");
		device_resume();
	}

	return error;

}

/*
 * test_dev_file
 *	make test call to device_create_file
 *	and if that call is successful make
 *	another call to device_remove_file
 */
static int test_dev_file()
{
	struct device *dev = &test_device;

	if (device_create_file(dev, &dev_attr_test_id)) {
		printk("tbase: failed to create dev sysfs file\n");
		return 1;
	} else {
		printk("tbase: created dev sysfs file\n");
		device_remove_file(dev, &dev_attr_test_id);
		return 0;
	}

}

/*
 * test_bus_rescan
 *	make test call to bus_rescan_devices which
 *	will rescan the bus and attempt to match devices
 *	to drivers, will return 0 for no matches or
 *	the number of matches made, check that the
 *	value returned is not negative
 */
static int test_bus_rescan()
{
	int count = 0;

	count = bus_rescan_devices(&test_bus_type);
	if (count == 0)
		printk("tbase: found no device/driver matches\n");
	else if (count > 0)
		printk("tbase; found match\n");
	else {
		printk("tbase: bus rescan failed\n");
		return count;
	}

	return 0;
}

/*
 * test_bus_file
 *      make test call to bus_create_file
 *      and if that call is successful make
 *      another call to bus_remove_file
 */
static int test_bus_file()
{
	struct bus_type *bus = &test_bus_type;

	if (bus_create_file(bus, &bus_attr_test_id)) {
		printk("tbase: failed to create bus sysfs file\n");
		return 1;
	} else {
		printk("tbase: created bus sysfs file\n");
		bus_remove_file(bus, &bus_attr_test_id);
		return 0;
	}

}

/*
 * test_class_file
 *      make test call to class_create_file
 *      and if that call is successful make
 *      another call to class_remove_file
 */
static int test_class_file()
{
	struct class *cls = &test_class;

	if (class_create_file(cls, &class_attr_test_id)) {
		printk("tbase: failed to create class sysfs file\n");
		return 1;
	} else {
		printk("tbase: created class sysfs file\n");
		class_remove_file(cls, &class_attr_test_id);
		return 0;
	}

}

/*
 * test_class_reg
 *	make test call to class_register
 *	with the test_class that is defined
 *	in this module, if that call is
 *	successful then call unregister
 */
static int test_class_reg()
{
	int error;

	error = class_register(&test_class);
	if (error)
		printk("tbase: class register failed\n");
	else
		printk("tbase: class register succeeded\n");

	return error;
}

/*
 * test_classdev_reg
 *	make test call to class_device_register
 *	and if that returns successful then
 *	make call to class_device_unregister
 */
static int test_classdev_reg()
{
	int rc = 0;

	if (class_device_register(&test_class_dev)) {
		printk("tbase: Failed to register class device\n");
		rc = 1;
	} else {
		printk("tbase: Registered class device\n");

		/* make class device sysfs file */
		if (class_device_create_file
		    (&test_class_dev, &class_device_attr_test_id)) {
			rc = 1;
			printk
			    ("tbase: Failed to create class device sysfs file\n");
		} else {
			printk("tbase: Created class device sysfs file\n");
			class_device_remove_file(&test_class_dev,
						 &class_device_attr_test_id);
		}

		class_device_unregister(&test_class_dev);
	}

	return rc;
}

/*
 * test_classint_reg
 *	make test call to class_interface_register
 *	and if that returns successfule then
 *	make call to class_interface_unregister
 */
static int test_classint_reg()
{

	if (class_interface_register(&test_interface)) {
		printk("tbase: Failed to register class interface\n");
		return 1;
	} else {
		printk("tbase: Registered class interface\n");
		class_interface_unregister(&test_interface);
		return 0;
	}

}

/*
 * test_sysdev_cls_reg
 *	make test call to sysdev_class_register
 *	to register the test_sysclass pointer
 *	as a sysdev_class with the system, check
 *	the return code
 */
static int test_sysdev_cls_reg()
{

	if (sysdev_class_register(&test_sysclass)) {
		printk("tbase: Failed to register sysdev class\n");
		return 1;
	} else {
		printk("tbase: Registered sysdev class\n");
		return 0;
	}

}

/*
 * test_sysdev_reg
 *      make test call to sys_device_register
 *      to register the test_sysdev pointer
 *      as a sys_device with the system, check
 *      the return code
 */
static int test_sysdev_reg()
{

	if (sys_device_register(&test_sys_device)) {
		printk("tbase: Failed to register sysdev \n");
		return 1;
	} else {
		printk("tbase: Registered sysdev \n");
		return 0;
	}

}

/*
 * tbase_init_module
 *      set the owner of tbase_fops, register the module
 *      as a char device, and perform any necessary
 *      initialization
 */
static int tbase_init_module(void)
{
	int rc;

	bus_register(&test_bus_type);
	driver_register(&test_driver);
	device_register(&test_device);

	tbase_fops.owner = THIS_MODULE;

	printk("tbase: *** Register device %s **\n", DEVICE_NAME);

	rc = register_chrdev(Major, DEVICE_NAME, &tbase_fops);
	if (rc < 0) {
		printk("tbase: Failed to register device.\n");
		return rc;
	}

	if (Major == 0)
		Major = rc;

	/* call any other init functions you might use here */

	printk("tbase: Registration success.\n");
	return 0;
}

/*
 * tmod_exit_module
 *      unregister the device and any necessary
 *      operations to close devices
 */
static void tbase_exit_module(void)
{
	int rc;

	device_unregister(&test_device);
	driver_unregister(&test_driver);
	bus_unregister(&test_bus_type);

	/* free any pointers still allocated, using kfree */

	rc = unregister_chrdev(Major, DEVICE_NAME);
	if (rc < 0)
		printk("tbase: unregister failed\n");
	else
		printk("tbase: unregister success\n");

}

/* specify what that init is run when the module is first
loaded and that exit is run when it is removed */

module_init(tbase_init_module)
    module_exit(tbase_exit_module)
