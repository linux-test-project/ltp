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
 */

/*
 *  HISTORY:
 *    06/09/2003 Initial creation mridge@us.ibm.com
 *      -Ported
 *  updated - 01/09/2005 Updates from Intel to add functionality
 *
 *  01/03/2009 Márton Németh <nm127@freemail.hu>
 *   - Updated for Linux kernel 2.6.28
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
#include <linux/dmi.h>
#include <linux/nls.h>

#include "ltp_acpi.h"

MODULE_AUTHOR("Martin Ridgeway <mridge@us.ibm.com>");
MODULE_AUTHOR("Alexey Kodanev <alexey.kodanev@oracle.com>");
MODULE_DESCRIPTION("ACPI LTP Test Driver");
MODULE_LICENSE("GPL");
ACPI_MODULE_NAME("LTP_ACPI")

#define prk_err(fmt, ...) \
	pr_err(ACPI_TEST_NAME ": " fmt "\n", ##__VA_ARGS__)
#define prk_alert(fmt, ...) \
	pr_alert(ACPI_TEST_NAME ": " fmt "\n", ##__VA_ARGS__)
#define prk_info(fmt, ...) \
	pr_info(ACPI_TEST_NAME ": " fmt "\n", ##__VA_ARGS__)

static int acpi_failure(acpi_status status, const char *name)
{
	if (ACPI_FAILURE(status)) {
		ACPI_EXCEPTION((AE_INFO, status, name));
		return 1;
	}
	return 0;
}

/* points to the string of the last found object _STR */
static char *str_obj_result;

/* sysfs device path of the last found device */
static char *sysfs_path;

/* first found device with _CRS */
static acpi_handle res_handle;

static acpi_status get_str_object(acpi_handle handle)
{
	int res;
	acpi_status status;
	acpi_handle temp = 0;
	union acpi_object *str_obj;
	char *buf = NULL;

	struct acpi_buffer buffer = {ACPI_ALLOCATE_BUFFER, NULL};

	status = acpi_get_handle(handle, "_STR", &temp);

	if (ACPI_SUCCESS(status) &&
	    !acpi_evaluate_object(handle, "_STR", NULL, &buffer)) {

		str_obj = buffer.pointer;

		buf = kmalloc(str_obj->buffer.length / 2, GFP_KERNEL);
		if (!buf) {
			kfree(str_obj);
			return AE_NO_MEMORY;
		}

		res = utf16s_to_utf8s((wchar_t *)str_obj->buffer.pointer,
			str_obj->buffer.length, UTF16_LITTLE_ENDIAN, buf,
			str_obj->buffer.length / 2);

		buf[res] = '\0';

		kfree(str_obj_result);
		str_obj_result = buf;
		kfree(str_obj);
	}

	return status;
}

static void get_crs_object(acpi_handle handle)
{
	acpi_status status;
	acpi_handle temp;
	if (!res_handle) {
		status = acpi_get_handle(handle, METHOD_NAME__CRS, &temp);
		if (ACPI_SUCCESS(status))
			res_handle = handle;
	}
}

static void get_sysfs_path(acpi_handle handle)
{
	acpi_status status;
	struct acpi_device *device;

	kfree(sysfs_path);
	sysfs_path = NULL;

	status = acpi_bus_get_device(handle, &device);
	if (ACPI_SUCCESS(status))
		sysfs_path = kobject_get_path(&device->dev.kobj, GFP_KERNEL);
}

/* acpi handle of the last visited device */
static acpi_handle start_parent;

static int acpi_traverse(acpi_handle parent, acpi_handle child)
{
	static char indent[64];
	const char * const ind_end = indent + 63;
	static const char *ind = ind_end;
	acpi_status status;
	struct acpi_device_info *dev_info;
	acpi_handle new_child;

	if (!indent[0])
		memset(indent, 0x20, 63);

	while (parent) {
		status = acpi_get_next_object(ACPI_TYPE_DEVICE,
			parent, child, &new_child);

		if (ACPI_FAILURE(status)) {
			ind += 4;

			child = parent;
			status = acpi_get_parent(child, &parent);

			/* no more devices */
			if (ACPI_FAILURE(status)) {
				start_parent = 0;
				kfree(str_obj_result);
				str_obj_result = NULL;
				return 0;
			}
			continue;
		}

		status = acpi_get_object_info(new_child, &dev_info);
		if (acpi_failure(status, "acpi_object_info failed"))
			return 1;

		get_sysfs_path(new_child);

		get_crs_object(new_child);

		if (ind < indent)
			ind = indent;
		else if (ind > ind_end)
			ind = ind_end;

		/*
		 * if we find _STR object we will stop here
		 * and save last visited child
		 */
		if (ACPI_SUCCESS(get_str_object(new_child))) {
			prk_info("%s%4.4s: has '_STR' '%s' path '%s'",
				ind, (char *)&dev_info->name, str_obj_result,
				(sysfs_path) ? sysfs_path : "no path");
			ind -= 4;
			start_parent = new_child;
			kfree(dev_info);
			return 0;
		}
		prk_info("%s%4.4s: path '%s'", ind, (char *)&dev_info->name,
			(sysfs_path) ? sysfs_path : "no path");

		ind -= 4;
		parent = new_child;
		child = 0;
		kfree(dev_info);
	}

	return 0;
}

static int acpi_traverse_from_root(void)
{
	acpi_status status;
	struct acpi_device_info *dev_info;
	acpi_handle parent = 0, child = 0;

	if (!start_parent) {
		status = acpi_get_handle(NULL, ACPI_NS_ROOT_PATH, &parent);
		if (acpi_failure(status, "acpi_get_handle"))
			return 1;
		status = acpi_get_object_info(parent, &dev_info);
		if (acpi_failure(status, "acpi_object_info failed"))
			return 1;
		prk_info("start from %4.4s", (char *)&dev_info->name);
		kfree(dev_info);
	} else {
		/* continue with the last visited child */
		parent = start_parent;
	}

	return acpi_traverse(parent, child);
}

/* first found device with _STR */
static acpi_handle dev_handle;
static int acpi_hw_reduced;

/* check if PM2 control register supported */
static bool pm2_supported;

static int acpi_init(void)
{
	acpi_status status;
	acpi_handle parent_handle;
	struct acpi_table_fadt *fadt;
	struct acpi_table_header *table;
	struct acpi_device_info *dev_info;
	pm2_supported = true;

	status = acpi_get_table(ACPI_SIG_FADT, 0, &table);
	if (ACPI_SUCCESS(status)) {
		fadt = (struct acpi_table_fadt *)table;
		if (fadt->flags & ACPI_FADT_HW_REDUCED)
			acpi_hw_reduced = 1;
		if (fadt->pm2_control_block == 0 || fadt->pm2_control_length == 0)
			pm2_supported = false;
	}
	if (acpi_hw_reduced)
		prk_alert("Detected the Hardware-reduced ACPI mode");

	prk_alert("TEST -- acpi_get_handle ");
	status = acpi_get_handle(NULL, "\\_SB", &parent_handle);
	if (acpi_failure(status, "acpi_get_handle"))
		return 1;

	/* get first device on SYS bus, it will be used in other tests */
	while (acpi_get_next_object(ACPI_TYPE_DEVICE,
		parent_handle, 0, &dev_handle) == 0) {
		parent_handle = dev_handle;
	}

	status = acpi_get_object_info(dev_handle, &dev_info);
	if (acpi_failure(status, "acpi_object_info failed"))
		return 1;

	prk_alert("ACPI object name %4.4s, type %d", (char *)&dev_info->name,
		dev_info->type);
	kfree(dev_info);

	prk_alert("TEST -- acpi_get_parent ");
	status = acpi_get_parent(dev_handle, &parent_handle);
	return acpi_failure(status, "acpi_get_parent failed");
}

/*
 * acpi_bus_notify
 * ---------------
 * Callback for all 'system-level' device notifications (values 0x00-0x7F).
 */
static void acpi_bus_notify(acpi_handle handle, u32 type, void *data)
{
	prk_alert("Register ACPI Bus Notify callback function");
}

static int acpi_test_notify_handler(void)
{
	acpi_status status;

	prk_alert("TEST -- acpi_install_notify_handler");

	status = acpi_install_notify_handler(dev_handle,
		ACPI_SYSTEM_NOTIFY, &acpi_bus_notify, NULL);

	if (ACPI_SUCCESS(status)) {
		prk_alert("TEST -- acpi_remove_notify_handler");
		status = acpi_remove_notify_handler(dev_handle,
			ACPI_SYSTEM_NOTIFY, &acpi_bus_notify);
		return acpi_failure(status, "acpi_remove_notify_handler");
	} else if (status != AE_ALREADY_EXISTS) {
		return acpi_failure(status, "acpi_install_notify_handler");
	}

	return 0;
}

static u32 ltp_test_power_button_ev_handler(void *context)
{
	prk_alert("ltp_test_power_button_ev_handler");
	return 1;
}

static u32 ltp_test_sleep_button_ev_handler(void *context)
{
	prk_alert("ltp_test_sleep_button_ev_handler");
	return 1;
}

static int acpi_test_event_handler(void)
{
	int err = 0;
	acpi_status status;

	prk_alert("TEST -- acpi_install_fixed_event_handler");
	if (acpi_hw_reduced) {
		prk_alert("Skipped due to the HW-reduced mode");
		return 0;
	}
	status = acpi_install_fixed_event_handler(ACPI_EVENT_POWER_BUTTON,
		ltp_test_power_button_ev_handler, NULL);

	if (ACPI_SUCCESS(status)) {
		prk_alert("TEST -- acpi_remove_fixed_event_handler");
		status = acpi_remove_fixed_event_handler(
			ACPI_EVENT_POWER_BUTTON,
			ltp_test_power_button_ev_handler);
		err = acpi_failure(status, "remove fixed event handler");
	} else if (status != AE_ALREADY_EXISTS) {
		err = acpi_failure(status, "install fixed event handler");
	}

	prk_alert("TEST -- acpi_install_fixed_event_handler");
	status = acpi_install_fixed_event_handler(ACPI_EVENT_RTC,
		ltp_test_sleep_button_ev_handler, NULL);

	if (ACPI_SUCCESS(status)) {
		prk_alert("TEST -- acpi_remove_fixed_event_handler");
		status = acpi_remove_fixed_event_handler(
			ACPI_EVENT_RTC,
			ltp_test_sleep_button_ev_handler);
		err |= acpi_failure(status, "remove fixed event handler");
	} else if (status != AE_ALREADY_EXISTS) {
		err |= acpi_failure(status, "install fixed event handler");
	}

	return err;
}

#ifndef ACPI_EC_UDELAY_GLK
#define ACPI_EC_UDELAY_GLK	1000	/* Wait 1ms max. to get global lock */
#endif

static int acpi_global_lock(void)
{
	acpi_status status;
	u32 global_lock = 0;

	prk_alert("TEST -- acpi_acquire_global_lock ");
	if (acpi_hw_reduced) {
		prk_alert("Skipped due to the HW-reduced mode");
		return 0;
	}
	status = acpi_acquire_global_lock(ACPI_EC_UDELAY_GLK, &global_lock);
	if (acpi_failure(status, "acpi_acquire_global_lock"))
		return 1;

	prk_alert("TEST -- acpi_release_global_lock ");
	status = acpi_release_global_lock(global_lock);
	return acpi_failure(status, "acpi_release_global_lock");
}

static int acpi_test_bus(void)
{
	int state = 0;
	acpi_status status;
	acpi_handle bus_handle;
	struct acpi_device *device;

	status = acpi_get_handle(NULL, "\\_SB", &bus_handle);
	if (acpi_failure(status, "acpi_get_handle"))
		return 1;

	prk_alert("TEST -- acpi_bus_get_device");
	status = acpi_bus_get_device(bus_handle, &device);
	if (acpi_failure(status, "acpi_bus_get_device"))
		return 1;

	prk_alert("TEST -- acpi_bus_update_power ");
	status = acpi_bus_update_power(device->handle, &state);
	if (acpi_failure(status, "error reading power state"))
		return 1;

	prk_info("acpi bus power state is %d", state);
	return 0;
}

static acpi_status acpi_ec_io_ports(struct acpi_resource *resource,
	void *context)
{
	return 0;
}

static int acpi_test_resources(void)
{
	int err = 0;
	acpi_status status;
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };

	/* skip if we don't find device with _CRC */
	if (res_handle == 0)
		return 0;

	prk_alert("TEST -- acpi_get_current_resources");
	status = acpi_get_current_resources(res_handle, &buffer);
	err = acpi_failure(status, "failed get_current_resources");
	ACPI_FREE(buffer.pointer);

#ifdef ACPI_FUTURE_USAGE
	prk_alert("TEST -- acpi_get_possible_resources");
	status = acpi_get_possible_resources(res_handle, &buffer);
	err |= acpi_failure(status, "get_possible_resources");
#endif

	prk_alert("TEST -- acpi_walk_resources ");
	status = acpi_walk_resources(res_handle, METHOD_NAME__CRS,
		acpi_ec_io_ports, NULL);
	err |= acpi_failure(status, "Failed walk_resources");

	return err;
}

static int acpi_sleep_test(void)
{
	int err = 0;
	acpi_status status;
	u32 i;
	u8 type_a, type_b;
	prk_alert("TEST -- acpi_get_sleep_type_data ");

	for (i = 0; i < ACPI_S_STATE_COUNT; ++i) {
		status = acpi_get_sleep_type_data(i, &type_a, &type_b);
		if (ACPI_SUCCESS(status)) {
			prk_info("get_sleep_type_data S%d a:%d b:%d",
				i, type_a, type_b);
		} else if (status != AE_NOT_FOUND) {
			err |= 1;
		}
	}

	return err;
}

static int acpi_test_register(void)
{
	int i, err = 0;
	u32 val;
	acpi_status status;

	prk_alert("TEST -- acpi_read_bit_register");
	if (acpi_hw_reduced) {
		prk_alert("Skipped due to the HW-reduced mode");
		return 0;
	}
	/*
	 * ACPICA: Remove obsolete Flags parameter.
	 * http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;
	 * a=commitdiff;h=d8c71b6d3b21cf21ad775e1cf6da95bf87bd5ad4
	 *
	 * ACPICA: Rename ACPI bit register access functions
	 * http://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/
	 * commit/?id=50ffba1bd3120b069617455545bc27bcf3cf7579
	 */
	for (i = 0; i < ACPI_NUM_BITREG; ++i) {
		if (i == ACPI_BITREG_ARB_DISABLE && !pm2_supported)
			continue;
		status = acpi_read_bit_register(i, &val);
		err |= acpi_failure(status, "acpi_read_bit_register");
		if (ACPI_SUCCESS(status))
			prk_alert("get register: %02x val: %04x", i, val);
	}

	return err;
}

static acpi_status ltp_get_dev_callback(acpi_handle obj, u32 depth,
	void *context, void **ret)
{
	char *name = context;
	char fullname[20];

	/*
	 * Only SBA shows up in ACPI namespace, so its CSR space
	 * includes both SBA and IOC.  Make SBA and IOC show up
	 * separately in PCI space.
	 */
	sprintf(fullname, "%s SBA", name);
	prk_info("get_dev_callback SBA name %s", fullname);
	sprintf(fullname, "%s IOC", name);
	prk_info("get_dev_callback IOC name %s", fullname);

	return 0;
}

static int acpi_test_dev_callback(void)
{
	acpi_status status;
	prk_alert("TEST -- acpi_get_devices ");
	status = acpi_get_devices(NULL, ltp_get_dev_callback, "LTP0001", NULL);
	return acpi_failure(status, "acpi_get_devices");
}

static int current_test_case;
static int test_result;

static void device_release(struct device *dev)
{
	prk_info("device released");
}

static struct device tdev = {
	.init_name	= ACPI_TEST_NAME,
	.release	= device_release,
};

/* print test result to sysfs file */
static ssize_t sys_result(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", test_result);
}
static DEVICE_ATTR(result, S_IRUSR, sys_result, NULL);

/* print found device description */
static ssize_t sys_str(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (str_obj_result)
		return scnprintf(buf, PAGE_SIZE, "%s", str_obj_result);
	else
		return 0;
}
static DEVICE_ATTR(str, S_IRUSR, sys_str, NULL);

/* print found device's sysfs path */
static ssize_t sys_path(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (sysfs_path)
		return scnprintf(buf, PAGE_SIZE, "%s", sysfs_path);
	else
		return 0;
}
static DEVICE_ATTR(path, S_IRUSR, sys_path, NULL);

static ssize_t sys_acpi_disabled(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d", acpi_disabled);
}
static DEVICE_ATTR(acpi_disabled, S_IRUSR, sys_acpi_disabled, NULL);

static ssize_t sys_tcase(struct device *dev,
	struct device_attribute *attr,  const char *buf, size_t count)
{
	sscanf(buf, "%d", &current_test_case);
	prk_info("test-case %d", current_test_case);

	switch (current_test_case) {
	case ACPI_INIT:
		test_result = acpi_init();
	break;
	case ACPI_TRAVERSE:
		test_result = acpi_traverse_from_root();
	break;
	case ACPI_NOTIFY_HANDLER:
		test_result = acpi_test_notify_handler();
	break;
	case ACPI_EVENT_HANDLER:
		test_result = acpi_test_event_handler();
	break;
	case ACPI_GLOBAL_LOCK:
		test_result = acpi_global_lock();
	break;
	case ACPI_TEST_BUS:
		test_result = acpi_test_bus();
	break;
	case ACPI_TEST_RESOURCES:
		test_result = acpi_test_resources();
	break;
	case ACPI_SLEEP_TEST:
		test_result = acpi_sleep_test();
	break;
	case ACPI_TEST_REGISTER:
		test_result = acpi_test_register();
	break;
	case ACPI_TEST_DEV_CALLBACK:
		test_result = acpi_test_dev_callback();
	break;
	}

	return count;
}
static DEVICE_ATTR(tcase, S_IWUSR, NULL, sys_tcase);

int init_module(void)
{
	int err = 0;
	prk_info("Starting module");

	err = device_register(&tdev);
	if (err) {
		prk_err("Unable to register device");
		goto err0;
	}
	prk_info("device registered");

	err = device_create_file(&tdev, &dev_attr_result);
	if (err) {
		prk_err("Can't create sysfs file 'result'");
		goto err1;
	}

	err = device_create_file(&tdev, &dev_attr_str);
	if (err) {
		prk_err("Can't create sysfs file 'str'");
		goto err2;
	}

	err = device_create_file(&tdev, &dev_attr_tcase);
	if (err) {
		prk_err(": Can't create sysfs file 'tc'");
		goto err3;
	}

	err = device_create_file(&tdev, &dev_attr_path);
	if (err) {
		prk_err(": Can't create sysfs file 'path'");
		goto err4;
	}

	err = device_create_file(&tdev, &dev_attr_acpi_disabled);
	if (err) {
		prk_err("Can't create sysfs file 'acpi_disabled'");
		goto err5;
	}

	return 0;

err5:
	device_remove_file(&tdev, &dev_attr_path);
err4:
	device_remove_file(&tdev, &dev_attr_tcase);
err3:
	device_remove_file(&tdev, &dev_attr_str);
err2:
	device_remove_file(&tdev, &dev_attr_result);
err1:
	device_unregister(&tdev);
err0:
	return err;
}

void cleanup_module(void)
{
	prk_info("Unloading module\n");

	kfree(str_obj_result);
	kfree(sysfs_path);

	device_remove_file(&tdev, &dev_attr_result);
	device_remove_file(&tdev, &dev_attr_str);
	device_remove_file(&tdev, &dev_attr_tcase);
	device_remove_file(&tdev, &dev_attr_path);
	device_unregister(&tdev);
}
