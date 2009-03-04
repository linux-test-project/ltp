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

/*
 *  FILE        : LtpAcpiCmds.c
 *  DESCRIPTION :
 *  HISTORY:
 *    06/09/2003 Initial creation mridge@us.ibm.com
 *      -Ported
 *  updated - 01/09/2005 Updates from Intel to add functionality
 *
 *  01/03/2009 Márton Németh <nm127@freemail.hu>
 *   - Updated for Linux kernel 2.6.28
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/ioctl.h>
#include <linux/pm.h>
#include <linux/acpi.h>
#include <linux/genhd.h>
#include <asm/uaccess.h>
#include "LtpAcpi.h"

#ifndef ACPI_EC_UDELAY_GLK
#define ACPI_EC_UDELAY_GLK	1000	/* Wait 1ms max. to get global lock */
#endif

static int ltpdev_open(struct block_device *bdev, fmode_t mode);
static int ltpdev_release(struct gendisk *disk, fmode_t mode);
static int ltpdev_ioctl(struct block_device *bdev, fmode_t mode, unsigned cmd, unsigned long arg);


static u32 ltp_test_sleep_button_ev_handler(void *context);
static u32 ltp_test_power_button_ev_handler(void *context);
static u32 acpi_ec_gpe_handler(void *context);

static void acpi_bus_notify (acpi_handle             handle, u32 type, void *data);
static acpi_status ltp_get_dev_callback (acpi_handle obj, u32 depth, void *context, void **ret);
static acpi_status acpi_ec_io_ports (struct acpi_resource	*resource,   void			*context);
#if 0
static acpi_status acpi_ec_space_setup (acpi_handle		region_handle,
	                                    u32			    function,
	                                    void			*handler_context,
	                                    void			**return_context);
static acpi_status acpi_ec_space_handler (u32			function,
                                          acpi_physical_address	address,
	                                      u32			bit_width,
	                                      acpi_integer		*value,
	                                      void			*handler_context,
	                                      void			*region_context);
#endif

static struct block_device_operations blkops = {
open:       ltpdev_open,
release:    ltpdev_release,
ioctl:      ltpdev_ioctl,
};

int ltp_acpi_major = LTPMAJOR;
int test_iteration = 0;

static char genhd_flags = 0;  
static struct gendisk * gd_ptr;

struct acpi_ec {
	acpi_handle			handle;
	unsigned long			uid;
	unsigned long long		gpe_bit;
	struct acpi_generic_address	status_addr;
	struct acpi_generic_address	command_addr;
	struct acpi_generic_address	data_addr;
	unsigned long			global_lock;
	spinlock_t			lock;
};

MODULE_AUTHOR("Martin Ridgeway <mridge@us.ibm.com>");
MODULE_DESCRIPTION(ACPI_LTP_TEST_DRIVER_NAME);
MODULE_LICENSE("GPL");



/*
 * Device operations for the virtual ACPI devices
 */


extern struct acpi_device		*acpi_root;

static int ltpdev_open(struct block_device *dev, fmode_t mode)
{
    printk(KERN_ALERT "ltpdev_open \n");
    return 0;
}

static int ltpdev_release(struct gendisk *disk, fmode_t mode)
{

    printk(KERN_ALERT "ltpdev_release \n");
    return 0;
}

static u32 ltp_test_power_button_ev_handler(void *context)
{
	printk(KERN_ALERT "ltp_test_power_button_ev_handler \n");
	return 1;
}

static u32 ltp_test_sleep_button_ev_handler(void *context)
{
	printk(KERN_ALERT "ltp_test_sleep_button_ev_handler \n");
	return 1;
}

static int ltpdev_ioctl(struct block_device *bdev, fmode_t mode, unsigned cmd, unsigned long arg)
{
    acpi_status        status;
//	acpi_handle        sys_bus_handle;
    acpi_handle        start_handle = 0;
    acpi_handle        parent_handle;
    acpi_handle        child_handle;
    acpi_handle        next_child_handle;
    acpi_status        level;
	struct acpi_ec		*ec;
	struct acpi_device  *device;
	struct acpi_buffer	buffer = {ACPI_ALLOCATE_BUFFER, NULL};

#if 0
	acpi_handle        tmp_handle;
	struct acpi_table_ecdt 	*ecdt_ptr;
	struct acpi_buffer	dsdt = {ACPI_ALLOCATE_BUFFER, NULL};
	struct acpi_buffer	batt_buffer = {ACPI_ALLOCATE_BUFFER, NULL};
	struct acpi_buffer	format = {sizeof(ACPI_BATTERY_FORMAT_BIF),
						ACPI_BATTERY_FORMAT_BIF};
	struct acpi_buffer	data = {0, NULL};
	union acpi_object	*package = NULL;
	u32                 start_ticks, stop_ticks, total_ticks;
#endif

    u32                 i, bm_status;
    u8                  type_a, type_b;
    u32			global_lock = 0;
    int 		state = 0;

    /*****************************************************************************/




    printk(KERN_ALERT "ltpdev_ioctl \n");
    switch (cmd) {
    case LTPDEV_CMD:

        parent_handle = start_handle;
        child_handle = 0;
        level        = 1;
        test_iteration++;

        printk(KERN_ALERT "-- IOCTL called to start ACPI tests -- Iteration:%d\n",test_iteration);

        printk(KERN_ALERT "TEST -- acpi_get_handle \n");

        status = acpi_get_handle (0, ACPI_NS_SYSTEM_BUS, &parent_handle);

	printk(KERN_ALERT "TEST -- acpi_get_object_info \n");

	status = acpi_get_object_info (parent_handle, &buffer);

        printk(KERN_ALERT "TEST -- acpi_get_next_object \n");

        status = acpi_get_next_object (ACPI_TYPE_ANY, parent_handle,
                  child_handle, &next_child_handle);

        printk(KERN_ALERT "TEST -- acpi_get_parent \n");

        status = acpi_get_parent(parent_handle, &parent_handle);

        printk(KERN_ALERT "TEST -- acpi_evaluate_object \n");

        status = acpi_evaluate_object(parent_handle, "_ON", NULL, NULL);

        printk(KERN_ALERT "TEST -- acpi_get_table \n");

//        status = acpi_get_table(ACPI_TABLE_RSDP, 1, &dsdt);
//        status = acpi_get_table(ACPI_TABLE_DSDT, 1, &dsdt);
//        status = acpi_get_table(ACPI_TABLE_FADT, 1, &dsdt);
//        status = acpi_get_table(ACPI_TABLE_FACS, 1, &dsdt);
//        status = acpi_get_table(ACPI_TABLE_PSDT, 1, &dsdt);
//        status = acpi_get_table(ACPI_TABLE_SSDT, 1, &dsdt);
//        status = acpi_get_table(ACPI_TABLE_XSDT, 1, &dsdt);

#if 0
        printk(KERN_ALERT "TEST -- acpi_get_firmware_table \n");

        status = acpi_get_firmware_table("ECDT", 1, ACPI_LOGICAL_ADDRESSING,
            (struct acpi_table_header **) &dsdt);
#endif

        printk(KERN_ALERT "TEST -- acpi_install_notify_handler \n");

        status = acpi_install_notify_handler(ACPI_ROOT_OBJECT, ACPI_SYSTEM_NOTIFY, &acpi_bus_notify, NULL);

        printk(KERN_ALERT "TEST -- acpi_remove_notify_handler \n");

        status = acpi_remove_notify_handler(ACPI_ROOT_OBJECT, ACPI_SYSTEM_NOTIFY, &acpi_bus_notify);

	printk(KERN_ALERT "TEST -- acpi_install_fixed_event_handler \n");
	status = acpi_install_fixed_event_handler(ACPI_EVENT_POWER_BUTTON, ltp_test_power_button_ev_handler, NULL);
	if(status)
		printk(KERN_ALERT "Failed installing fixed event handler \n");

	printk(KERN_ALERT "TEST -- acpi_remove_fixed_event_handler \n");
	status = acpi_remove_fixed_event_handler(ACPI_EVENT_POWER_BUTTON, ltp_test_power_button_ev_handler);
	if(status)
		printk(KERN_ALERT "Failed removing fixed event handler \n");

	printk(KERN_ALERT "TEST -- acpi_install_fixed_event_handler \n");
	status = acpi_install_fixed_event_handler(ACPI_EVENT_SLEEP_BUTTON, ltp_test_sleep_button_ev_handler, NULL);
	if(status)
		printk(KERN_ALERT "Failed installing fixed event handler \n");

	printk(KERN_ALERT "TEST -- acpi_remove_fixed_event_handler \n");
	status = acpi_remove_fixed_event_handler(ACPI_EVENT_SLEEP_BUTTON, ltp_test_sleep_button_ev_handler);
	if(status)
		printk(KERN_ALERT "Failed removing fixed event handler \n");

	printk(KERN_ALERT "TEST -- acpi_acquire_global_lock \n");
	status = acpi_acquire_global_lock(ACPI_EC_UDELAY_GLK, &global_lock);

	printk(KERN_ALERT "TEST -- acpi_release_global_lock \n");
	status = acpi_release_global_lock(global_lock);

        printk(KERN_ALERT "TEST -- acpi_bus_get_device \n");

        status = acpi_bus_get_device(next_child_handle, &device);

#if 0
	printk(KERN_ALERT "TEST -- acpi_bus_find_driver \n");
	status = acpi_bus_find_driver(device);
#endif

	printk(KERN_ALERT "TEST -- acpi_bus_get_power \n");
	status = acpi_bus_get_power(next_child_handle, &state);
	if(status)
	printk(KERN_ALERT "Error reading power state \n");

        printk(KERN_ALERT "TEST -- acpi_driver_data \n");

        ec = acpi_driver_data(device);

        if (!ec){
            printk(KERN_ALERT "Failure getting device data \n");
        }
        else {

            printk(KERN_ALERT "TEST -- acpi_install_gpe_handler \n");
            ec->status_addr = ec->command_addr;
            status = acpi_install_gpe_handler(device, ec->gpe_bit, ACPI_GPE_EDGE_TRIGGERED, &acpi_ec_gpe_handler, ec);
/*
            status = acpi_install_address_space_handler (ACPI_ROOT_OBJECT,
                    ACPI_ADR_SPACE_EC, &acpi_ec_space_handler,
                    &acpi_ec_space_setup, ec);

            if (status) {
                printk(KERN_ALERT "Failed installing address space handler \n");
            }

            acpi_remove_address_space_handler(ACPI_ROOT_OBJECT,
                ACPI_ADR_SPACE_EC, &acpi_ec_space_handler);
*/
            printk(KERN_ALERT "TEST -- acpi_remove_gpe_handler \n");
            acpi_remove_gpe_handler(device, ec->gpe_bit, &acpi_ec_gpe_handler);
        }
    
        printk(KERN_ALERT "TEST -- acpi_get_current_resources \n");
        status = acpi_get_current_resources (next_child_handle, &buffer);

        if (status) {
            printk(KERN_ALERT "Failed get_current_resources %d\n",status);
        }

#ifdef ACPI_FUTURE_USAGE
        printk(KERN_ALERT "TEST -- acpi_get_possible_resources \n");
        status = acpi_get_possible_resources (next_child_handle, &buffer);

        if (status) {
            printk(KERN_ALERT "Failed get_possible_resources %d\n",status);
        }
#endif

        printk(KERN_ALERT "TEST -- acpi_walk_resources \n");
        status = acpi_walk_resources(ec->handle, METHOD_NAME__CRS,
            acpi_ec_io_ports, ec);

        if (status) {
            printk(KERN_ALERT "Failed walk_resources %d\n",status);
        }

	printk(KERN_ALERT "TEST -- acpi_evaluate_integer \n");
	status = acpi_evaluate_integer(ec->handle, "_GPE", NULL, &ec->gpe_bit);
	if(status)
	printk(KERN_ALERT "Error obtaining GPE bit assignment\n");

#if 0
        printk(KERN_ALERT "TEST -- acpi_get_timer \n");
        status = acpi_get_timer(&total_ticks);

        if (status) {
            printk(KERN_ALERT "Failed get_timer %d\n",status);
        }
        else {
            printk(KERN_ALERT "get_timer -- total_ticks %d\n",total_ticks);
        }

        start_ticks = 20;
        stop_ticks  = 30;

        printk(KERN_ALERT "TEST -- acpi_get_timer_duration \n");
        status = acpi_get_timer_duration(start_ticks, stop_ticks, &total_ticks);

        if (status) {
            printk(KERN_ALERT "Failed get_timer_duration %d\n",status);
        }
        else {
            printk(KERN_ALERT "get_timer_duration total_ticks %d\n",total_ticks);
        }
#endif

        for (i = 0; i < ACPI_S_STATE_COUNT; i++) {
            printk(KERN_ALERT "TEST -- acpi_get_sleep_type_data \n");
		    status = acpi_get_sleep_type_data(i, &type_a, &type_b);

            if (status) {
                printk(KERN_ALERT "Failed get_sleep_type_data %d\n",status);
            }
            else {
                printk(KERN_ALERT "get_sleep_type_data [%d] type_a:%d type_b:%d\n",i, type_a,type_b);
            }
        }

        printk(KERN_ALERT "TEST -- acpi_get_register \n");

/*
 * ACPICA: Remove obsolete Flags parameter.
 * http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commitdiff;h=d8c71b6d3b21cf21ad775e1cf6da95bf87bd5ad4
 *
 */
	acpi_get_register(ACPI_BITREG_BUS_MASTER_STATUS, &bm_status);

        if (!bm_status) {
            printk(KERN_ALERT "Failed get_register [%d]\n",bm_status);
        }
        else {
            printk(KERN_ALERT "get_register [%d] \n",bm_status);
        }

//        Puts system to sleep, permenately !!!
//        status = acpi_enter_sleep_state(ACPI_STATE_S1);

#if 0
        printk(KERN_ALERT "TEST -- acpi_get_system_info \n");
        status = acpi_get_system_info(&buffer);

        if (status) {
            printk(KERN_ALERT "Failed get_system_info %d\n",status);
        }
        else {
            printk(KERN_ALERT "get_system_info buffer.length:%d buffer.pointer:%p\n",buffer.length, buffer.pointer);

            acpi_os_printf("os_printf OK %d\n",status);

            if (buffer.pointer) {
                acpi_os_free(buffer.pointer);
            }
        }
#endif

        printk(KERN_ALERT "TEST -- acpi_get_devices \n");
        status = acpi_get_devices(NULL, ltp_get_dev_callback, "LTP0001", NULL);

        if (status) {
            printk(KERN_ALERT "Failed get_devices %d\n",status);
        }


//        status = acpi_os_create_semaphore(1, 1, &tmp_handle);

        if (status) {
            printk(KERN_ALERT "Failed os_create_semaphore %d\n",status);
        }
        else {
            printk(KERN_ALERT "os_create_semaphore OK, no deleteing %d\n",status);
//            acpi_os_delete_semaphore(tmp_handle);

        }

#if 0
        printk(KERN_ALERT "TEST -- acpi_get_system_info \n");
        status = acpi_get_system_info(&batt_buffer);

        if (status) {
            printk(KERN_ALERT "Failed get_system_info %d\n",status);
        }
        else {
            printk(KERN_ALERT "get_system_info buffer.length:%d buffer.pointer:%p\n",buffer.length, buffer.pointer);

            package = (union acpi_object *) batt_buffer.pointer;

            /* Extract Package Data */

            printk(KERN_ALERT "TEST -- acpi_extract_package \n");
            status = acpi_extract_package(package, &format, &data);

            data.pointer = kmalloc(data.length, GFP_KERNEL);

            if (!data.pointer) {
                printk(KERN_ALERT "Failed getting memory kalloc \n");
            }
            else {
                memset(data.pointer, 0, data.length);

                printk(KERN_ALERT "TEST -- acpi_extract_package \n");
                status = acpi_extract_package(package, &format, &data);

                kfree(data.pointer);
            }

//            acpi_os_free(buffer.pointer);
        }
#endif

        printk(KERN_ALERT "-- IOCTL ACPI tests Complete -- Iteration:%d\n",test_iteration);

        break;
    }


    return 0;
}

static acpi_status ltp_get_dev_callback (acpi_handle obj, u32 depth, void *context, void **ret)
{
	char *name = context;
	char fullname[20];

	/*
	 * Only SBA shows up in ACPI namespace, so its CSR space
	 * includes both SBA and IOC.  Make SBA and IOC show up
	 * separately in PCI space.
	 */
	sprintf(fullname, "%s SBA", name);
    printk(KERN_ALERT "get_dev_callback SBA name %s \n", fullname);
	sprintf(fullname, "%s IOC", name);
    printk(KERN_ALERT "get_dev_callback IOC name %s \n", fullname);

	return 0;
}

/**
 * acpi_bus_notify
 * ---------------
 * Callback for all 'system-level' device notifications (values 0x00-0x7F).
 */
static void acpi_bus_notify (acpi_handle             handle,
	                         u32                     type,
                             void                    *data)
{

    printk(KERN_ALERT "Register ACPI Bus Notify callback function \n");

}

static u32 acpi_ec_gpe_handler(void *context)
{
	printk(KERN_ALERT "Register ACPI ec_gpe_handler callback function \n");
	return 1;
}

static acpi_status acpi_ec_io_ports (struct acpi_resource	*resource,   void			*context)
{
  return 0;
}

#if 0
static acpi_status acpi_ec_space_handler (u32			function,
	acpi_physical_address	address,
	u32			bit_width,
	acpi_integer		*value,
	void			*handler_context,
	void			*region_context)
{
	int			result = 0;
	struct acpi_ec		*ec = NULL;
	u32			temp = 0;

	ACPI_FUNCTION_TRACE("acpi_ec_space_handler");

	if ((address > 0xFF) || (bit_width != 8) || !value || !handler_context)
		return_VALUE(AE_BAD_PARAMETER);

	ec = (struct acpi_ec *) handler_context;

	switch (function) {
	case ACPI_READ:
		result = 0;
		*value = (acpi_integer) temp;
		break;
	case ACPI_WRITE:
		result = 0;
		break;
	default:
		result = -EINVAL;
		break;
	}

	switch (result) {
	case -EINVAL:
		return_VALUE(AE_BAD_PARAMETER);
		break;
	case -ENODEV:
		return_VALUE(AE_NOT_FOUND);
		break;
	case -ETIME:
		return_VALUE(AE_TIME);
		break;
	default:
		return_VALUE(AE_OK);
	}

}

static acpi_status acpi_ec_space_setup (
	acpi_handle		region_handle,
	u32			function,
	void			*handler_context,
	void			**return_context)
{
	/*
	 * The EC object is in the handler context and is needed
	 * when calling the acpi_ec_space_handler.
	 */
	*return_context = handler_context;

	return AE_OK;
}
#endif

int init_module(void)
{
    int                result;



    printk(KERN_ALERT "ltpdev_init_module \n");


    result = register_blkdev(ltp_acpi_major, LTP_ACPI_DEV_NAME);

    printk(KERN_ALERT "LTP ACPI: register_blkdev result=%d major %d\n",result, ltp_acpi_major);

    if (result < 0) {
        printk(KERN_ALERT "LTP ACPI: can't get major %d\n",ltp_acpi_major);
        return result;
    }
//    if (ltp_acpi_major == 0)
//      ltp_acpi_major = result; /* dynamic */

	gd_ptr = kmalloc(sizeof(struct gendisk *), GFP_KERNEL);

    if (!gd_ptr) {
        printk(KERN_ALERT "ERROR getting memory !!!\n");
        return 0;
    }

    gd_ptr = alloc_disk(1);

    printk(KERN_ALERT "gd_ptr after alloc = %p \n",gd_ptr);

    gd_ptr->major = ltp_acpi_major;
    gd_ptr->first_minor = 0;
    gd_ptr->fops = &blkops;
//    gd_ptr->minor_shift = MINOR_SHIFT_BITS;
    gd_ptr->driverfs_dev = NULL;
//    gd_ptr->disk_de = NULL;
    gd_ptr->flags = genhd_flags;


    sprintf(gd_ptr->disk_name, LTP_ACPI_DEV_NAME);

    add_disk(gd_ptr);

    return 0;
}

void cleanup_module(void)
{

    printk(KERN_ALERT "Exiting module and cleaning up \n");

    put_disk(gd_ptr);

    del_gendisk(gd_ptr);

    unregister_blkdev(ltp_acpi_major, LTP_ACPI_DEV_NAME);

}

