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
 * This example module shows how a test driver
 * can be driven through various ioctl calls in
 * a user space program that has attained the
 * appropriate file descriptor for this device.
 *
 * author: Kai Zhao
 * date:   08/25/2003
 *
 * module: tagp
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/init.h>
#include <asm/uaccess.h>

#include <linux/pci.h>
#include <linux/agp_backend.h>
#include <linux/gfp.h>
#include <linux/page-flags.h>
#include <linux/mm.h>

//#include "agp.h"

#include "tagp.h"
#include "str_agp.h"

MODULE_AUTHOR("kai zhao <ltcd3@cn.ibm.com>");
MODULE_DESCRIPTION(tagp_DRIVER_NAME);
MODULE_LICENSE("GPL");

static int tagp_ioctl(struct inode *, struct file *, unsigned int,
		      unsigned long);
static int tagp_open(struct inode *, struct file *);
static int tagp_close(struct inode *, struct file *);

static int test_pci_find_device(void);
static int test_agp_backend_acquire(void);
static int test_agp_backend_release(void);
static int test_agp_alloc_bridge(void);
static int test_agp_put_bridge(void);
static int test_agp_create_and_free_memory(void);
//static int test_agp_free_memory(void);
static int test_agp_num_entries(void);
static int test_agp_copy_info(void);
//static int test_agp_allocate_memory(void);
static int test_get_agp_version(void);
static int test_agp_generic_enable(void);
static int test_agp_generic_create_gatt_table(void);
static int test_agp_generic_free_gatt_table(void);
static int test_agp_generic_insert_memory(void);
static int test_agp_generic_alloc_by_type(void);
static int test_agp_generic_alloc_page(void);
//static int test_agp_generic_destroy_page(void);
static int test_agp_enable(void);
static int test_global_cache_flush(void);
static int test_agp_generic_mask_memory(void);

static int Major = TAGP_MAJOR;
//static ltpmod_user_t ltp_mod;

/*
 * File operations struct, to use operations find the
 * correct file descriptor
 */
static struct file_operations tagp_fops = {
open:	tagp_open,
release:tagp_close,
ioctl:	tagp_ioctl,
};

/*
 * open and close operations, just return 0 for
 * your test modules, need them for the file
 * operations structure
 */
static int tagp_open(struct inode *ino, struct file *f)
{
	return 0;
}

static int tagp_close(struct inode *ino, struct file *f)
{
	return 0;
}

/*
 * tagp_ioctl:
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
static int tagp_ioctl(struct inode *ino, struct file *f,
		      unsigned int cmd, unsigned long l)
{
	int rc;
	tagp_interface_t tif;
	caddr_t *inparms;
	caddr_t *outparms;

	printk("Enter tagp_ioctl\n");

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
	case TEST_PCI_FIND_DEV:
		rc = test_pci_find_device();
		break;
	case TEST_BACKEND_ACQUIRE:
		rc = test_agp_backend_acquire();
		break;
	case TEST_BACKEND_RELEASE:
		rc = test_agp_backend_release();
		break;
	case TEST_ALLOC_BRIDGE:
		rc = test_agp_alloc_bridge();
		break;
	case TEST_PUT_BRIDGE:
		rc = test_agp_put_bridge();
		break;
	case TEST_CREATE_AND_FREE_MEMORY:
		rc = test_agp_create_and_free_memory();
		break;
//              case TEST_FREE_MEMORY:                  rc = test_agp_free_memory();break;
	case TEST_NUM_ENTRIES:
		rc = test_agp_num_entries();
		break;
	case TEST_COPY_INFO:
		rc = test_agp_copy_info();
		break;
//              case TEST_ALLOC_MEMORY_AND_BAND_UNBAND: rc = test_agp_allocate_memory();break;
	case TEST_GET_VERSION:
		rc = test_get_agp_version();
		break;
	case TEST_GENERIC_ENABLE:
		rc = test_agp_generic_enable();
		break;
	case TEST_GENERIC_CREATE_GATT_TABLE:
		rc = test_agp_generic_create_gatt_table();
		break;
	case TEST_GENERIC_FREE_GATT_TABLE:
		rc = test_agp_generic_free_gatt_table();
		break;
	case TEST_GENERIC_INSERT_MEMORY:
		rc = test_agp_generic_insert_memory();
		break;
	case TEST_GENERIC_ALLOC_BY_TYPE:
		rc = test_agp_generic_alloc_by_type();
		break;
	case TEST_GENERIC_ALLOC_PAGE:
		rc = test_agp_generic_alloc_page();
		break;
	case TEST_ENABLE:
		rc = test_agp_enable();
		break;
	case TEST_GLOBAL_CACHE_FLUSH:
		rc = test_global_cache_flush();
		break;
	case TEST_GENERIC_MASK_MEMORY:
		rc = test_agp_generic_mask_memory();
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
 *  Function and structure needed by agp_bridge.
 */

static struct aper_size_info_fixed test_core_agp_sizes[] = {
	{0, 0, 0},
};

static int test_fetch_size(void)
{
	printk("<1> tagp : Enter test fetch size\n");
	return 0;

}

static int test_configure(void)
{
	/* Do not config test_core_agp_size */
	printk("<1> tagp : Enter test configure\n");
	return 0;
}

static void test_cleanup(void)
{
	printk("<1> tagp : Enter test_cleanup\n");
	return;
}

static void test_tlbflush(struct agp_memory *temp)
{
	printk("<1> tagp : Enter test tlbflush\n");
	return;
}

/*
 *  structure used by agp_bridge
 */
struct agp_bridge_driver test_driver = {
	.owner = THIS_MODULE,
	.aperture_sizes = test_core_agp_sizes,
	.size_type = U8_APER_SIZE,
	.num_aperture_sizes = 7,
	.configure = test_configure,
	.fetch_size = test_fetch_size,
	.cleanup = test_cleanup,
	.tlb_flush = test_tlbflush,
	.mask_memory = agp_generic_mask_memory,
	.masks = NULL,
	.agp_enable = agp_generic_enable,
	.cache_flush = global_cache_flush,
	.create_gatt_table = agp_generic_create_gatt_table,
	.free_gatt_table = agp_generic_free_gatt_table,
	.insert_memory = agp_generic_insert_memory,
	.remove_memory = agp_generic_remove_memory,
	.alloc_by_type = agp_generic_alloc_by_type,
	.free_by_type = agp_generic_free_by_type,
	.agp_alloc_page = agp_generic_alloc_page,
	.agp_destroy_page = agp_generic_destroy_page,
};

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

static int test_agp_backend_acquire(void)
{
	printk("<1> tagp : Enter test_agp_backend_acquire\n");
	agp_backend_acquire();
	return 0;
}

static int test_agp_backend_release(void)
{
	printk("<1> tagp : Enter test_agp_backend_release\n");
	agp_backend_release();
	return 0;
}

static int test_agp_alloc_bridge(void)
{
//      struct agp_bridge_data *tmp_bridge;
	tmp_bridge = agp_alloc_bridge();
//      agp_put_bridge (tmp_bridge);
//      tmp_bridge = NULL;
	return 0;
}

static int test_agp_put_bridge(void)
{
	agp_put_bridge(tmp_bridge);
	tmp_bridge = NULL;
	return 0;
}

static int test_agp_create_and_free_memory(void)
{
	struct agp_memory *tmp_agp_memory = NULL;
	/* int scratch_pages */
	if (agp_bridge->scratch_page > 0) {
		printk("<1> tagp : agp_bridge->scratch_page : %ld\n",
		       agp_bridge->scratch_page);
		tmp_agp_memory = agp_create_memory(agp_bridge->scratch_page);
	} else {
		printk("<1> tagp : agp_bridge->scratch_page : %ld\n",
		       agp_bridge->scratch_page);
		tmp_agp_memory = agp_create_memory(64);
	}
	if (tmp_agp_memory != NULL) {
		agp_free_memory(tmp_agp_memory);
		return 0;
	}
	return 1;
}

/*
static int test_agp_free_memory(void)
{
	if (tmp_agp_memory != NULL)
	{
		agp_free_memory(tmp_agp_memory);
		return 0;
	}
	return 1;
}
*/
static int test_agp_num_entries(void)
{
	int ret = agp_num_entries();
	printk("<1> tagp : agp_num_entries return %d\n", ret);
	return 0;
}

////////////////////////////////////////////////////////////////////////////
static int test_agp_copy_info(void)
{
	struct agp_kern_info *info;
	int ret;

	info =
	    (struct agp_kern_info *)kmalloc(sizeof(struct agp_kern_info),
					    GFP_KERNEL);
	if (!info) {
		printk("<1> tagp : can not alloc spece\n");
		return 1;
	}
	ret = agp_copy_info(info);
	if (ret) {
		printk("<1> tagp : agp_copy_info failed\n");
		return 1;
	}
	kfree(info);

	return 0;
}

/*
static int test_agp_allocate_memory(void)
{
	struct agp_memory * local_agp_memory = NULL;
	int ret = 0 , i = 0;

	local_agp_memory = agp_allocate_memory(8,AGP_NORMAL_MEMORY);

	if (local_agp_memory == NULL)
	{
		printk("<1> tagp : agp_allocate_memory failed\n");
		return 1;
	}

	ret = agp_bind_memory(local_agp_memory, 64);
	if (ret)
	{
		agp_free_memory(local_agp_memory);
		printk("<1> tagp : agp bind memory failed\n");
		return 1;
	}
	printk("<1> tagp : agp bind memory success\n");
	ret = agp_unbind_memory(local_agp_memory);
	if (ret)
	{
		agp_free_memory(local_agp_memory);
		printk("<1> tagp : agp unband memory failed\n");
	}

	for (i = 0; i < 8; i++) {
		phys_to_virt(local_agp_memory->memory[i]);//virt_to_phys(addr);
		local_agp_memory->page_count--;
	}

	agp_free_memory(local_agp_memory);
	printk("<1> tagp : agp unband memory success\n");

	return 0;
}
*/
static int test_get_agp_version(void)
{
	printk("<1> tagp : Enter test_get_agp_version\n");
	get_agp_version(agp_bridge);
	return 0;
}

static int test_agp_generic_enable(void)
{
	printk("<1> tagp : Enter test_agp_generic_enable\n");
	agp_generic_enable(agp_bridge->mode);
	return 0;
}

static int test_agp_generic_create_gatt_table(void)
{
	printk("<1> tagp : Enter test_agp_generic_create_gatt_table\n");
	return agp_generic_create_gatt_table();
}

static int test_agp_generic_free_gatt_table(void)
{
	printk("<1> tagp : Enter test_agp_generic_free_gatt_table\n");
	return agp_generic_free_gatt_table();
}

static int test_agp_generic_insert_memory(void)
{
	struct agp_memory *tmp_agp_memory = NULL;
	/* int scratch_pages */
	if (agp_bridge->scratch_page > 0)
		tmp_agp_memory = agp_create_memory(agp_bridge->scratch_page);
	else
		tmp_agp_memory = agp_create_memory(64);
	if (tmp_agp_memory != NULL) {
		if (agp_generic_insert_memory(tmp_agp_memory, 16, 0)) {
			printk("<1> tagp : agp_generic_insert_memory failed\n");
			agp_free_memory(tmp_agp_memory);
			return 1;
		} else {
			printk
			    ("<1> tagp : agp_generic_insert_memory success\n");
			agp_generic_remove_memory(tmp_agp_memory, 16, 0);
			agp_free_memory(tmp_agp_memory);
		}
	}
	return 0;
}

static int test_agp_generic_alloc_by_type(void)
{
	/* size_t page_count, int type */
	agp_generic_alloc_by_type(0, 0);
	return 0;
}

static int test_agp_generic_alloc_page(void)
{
	printk("<1> tagp : Enter test_agp_generic_alloc_page\n");
	void *ppage = agp_generic_alloc_page();
	if (ppage != NULL)
		agp_generic_destroy_page(ppage);
	return 0;
}

static int test_agp_enable(void)
{
	printk("<1> tagp : Enter test_agp_enable\n");
	agp_enable(agp_bridge->mode);
	return 0;
}

static int test_global_cache_flush(void)
{
	printk("<1> tagp : Enter test_global_cache_flush\n");
	global_cache_flush();
	return 0;
}

static int test_agp_generic_mask_memory(void)
{
	printk("<1> tagp : Enter test_agp_generic_mask_memory\n");
	unsigned long temp;
	temp = agp_generic_mask_memory(1000, agp_bridge->type);
	return 0;
}

static int test_pci_find_device()
{
	struct pci_dev *pdev;	// = (struct pci_dev *)kmalloc(sizeof(struct pci_dev), GFP_KERNEL);
	struct agp_bridge_data *bridge = NULL;

	pdev = pci_find_device(PCI_VENDOR_ID_ATI, PCI_ANY_ID, NULL);

	if (pdev) {
		printk("<1> tagp : pci find device success\n");

		u8 cap_ptr;

		cap_ptr = pci_find_capability(pdev, PCI_CAP_ID_AGP);
		if (!cap_ptr) {
			printk("<1> tagp : pci find capability Failed\n");
			return -ENODEV;
		}

		printk("<1> tagp : pci find capability success \n");
		bridge = agp_alloc_bridge();
		if (!bridge) {
			printk("<1> tagp : agp alloc bridge Failed\n");
			return -ENOMEM;
		}
		printk("<1> tagp : agp alloc bridge success\n");
		bridge->driver = &test_driver;
		bridge->dev = pdev;
		bridge->capndx = cap_ptr;

		/* Fill in the mode register */
		pci_read_config_dword(pdev,
				      bridge->capndx + PCI_AGP_STATUS,
				      &bridge->mode);
		printk("<1> tagp : agp read config dword  success\n");
		pci_set_drvdata(pdev, bridge);
		printk("<1> tagp : agp set drvdata  success\n");
		return agp_add_bridge(bridge);
	}

	return 1;
}

static int __init agp_test_probe(struct pci_dev *pdev,
				 const struct pci_device_id *ent)
{

	printk("<1> tagp :Enter agp test probe\n");
	return 0;

}

static void __devexit agp_test_remove(struct pci_dev *pdev)
{
	printk("<1> tagp: Enter agp test remove\n");
	struct agp_bridge_data *bridge = pci_get_drvdata(pdev);

	agp_remove_bridge(bridge);
	agp_put_bridge(bridge);
}

static struct pci_device_id agp_test_pci_table[] __initdata = {
	{
	 .class = (PCI_CLASS_BRIDGE_HOST << 8),
	 .class_mask = ~0,
	 .vendor = PCI_ANY_ID,	//VENDOR_ID_ATI,
	 .device = PCI_ANY_ID,
	 .subvendor = PCI_ANY_ID,
	 .subdevice = PCI_ANY_ID,
	 },
	{}
};

MODULE_DEVICE_TABLE(pci, agp_test_pci_table);

static struct pci_driver agp_test_pci_driver = {
	.name = "agp_test",
	.id_table = agp_test_pci_table,
	.probe = agp_test_probe,
	.remove = agp_test_remove,
};

/*
 * tagp_init_module
 *      set the owner of tagp_fops, register the module
 *      as a char device, and perform any necessary
 *      initialization for pci devices
 */
static int __init tagp_init_module(void)
{
	int rc;

//      SET_MODULE_OWNER(&tagp_fops);
	tagp_fops.owner = THIS_MODULE;

	rc = register_chrdev(Major, DEVICE_NAME, &tagp_fops);
	if (rc < 0) {
		printk("tagp: Failed to register device.\n");
		return rc;
	}

	if (Major == 0)
		Major = rc;

	rc = pci_module_init(&agp_test_pci_driver);

	if (rc < 0) {
		printk("tagp: pci_module_init failed.\n");
		return rc;
	}

	printk("tagp: PCI module init success.\n");
	printk("tagp: Registration success.\n");

	return 0;
}

/*
 * tagp_exit_module
 *      unregister the device and any necessary
 *      operations to close devices
 */
static void __exit tagp_exit_module(void)
{
	int rc;

	/* free any pointers still allocated, using kfree */
	rc = unregister_chrdev(Major, DEVICE_NAME);
	if (rc < 0)
		printk("tagp: unregister failed\n");
	else
		printk("tagp: unregister success\n");

	pci_unregister_driver(&agp_test_pci_driver);
}

/* specify what that init is run when the module is first
loaded and that exit is run when it is removed */

module_init(tagp_init_module)
    module_exit(tagp_exit_module)
