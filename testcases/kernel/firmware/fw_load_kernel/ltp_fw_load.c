// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 * Author: Alexey Kodanev <alexey.kodanev@oracle.com>
 *
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*
 * Kernel module helper for the fw_load test.
 *
 * Registers a virtual device (ltp_fw_load) that exposes two sysfs
 * attributes:
 *
 *   fwnum  (write-only) - accepts the number of firmware files to
 *           request (1-32). Writing triggers request_firmware() for
 *           each file named n<i>_<fw_name> (i = 0 .. fwnum-1).
 *           Each loaded blob is verified against the expected size
 *           (fw_size) and byte pattern (every byte == i).
 *
 *   result (read-only)  - bitmask of per-file pass/fail results.
 *           Bit i is set when n<i>_<fw_name> was loaded and
 *           verified successfully.
 *
 * Module parameters:
 *   fw_name  - template firmware file name (default: load_tst.fw)
 *   fw_size  - expected firmware blob size  (default: 0x1000)
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/firmware.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alexey Kodanev <alexey.kodanev@oracle.com>");
MODULE_DESCRIPTION("This module is checking device firmware loading");

#define MODULE_NAME	"ltp_fw_load"
#define MAX_NAME	64

static char *fw_name	= "load_tst.fw";
static int fw_size	= 0x1000;

module_param(fw_name, charp, 0444);
MODULE_PARM_DESC(fw_name, "Template firmware file name: n#_name");

module_param(fw_size, int, 0444);
MODULE_PARM_DESC(fw_size, "Firmware file size");

/*
 * bit mask for each test-case,
 * if test is passed, bit will be set to 1
 */
static int test_result;

static void device_release(struct device *dev)
{
	pr_info(MODULE_NAME ": device released\n");
}

static struct device tdev = {
	.init_name	= MODULE_NAME,
	.release	= device_release,
};

/* read and verify firmware data */
static int fw_read(const u8 *data, size_t size, u8 expected)
{
	size_t i;

	pr_info(MODULE_NAME ": Firmware has size '%zu'\n", size);
	if (size != fw_size) {
		pr_err(MODULE_NAME ": Expected firmware size '%d'\n", fw_size);
		return -1;
	}

	for (i = 0; i < size; ++i) {
		if (data[i] != expected) {
			pr_err(MODULE_NAME ": Unexpected firmware data\n");
			return -1;
		}
	}

	return 0;
}

static int try_request_fw(const char *name, u8 expected)
{
	int err;
	const struct firmware *fw_entry = NULL;

	err = request_firmware(&fw_entry, name, &tdev);
	if (!err) {
		pr_info(MODULE_NAME ": firmware '%s' requested\n", name);
		err = fw_read(fw_entry->data, fw_entry->size, expected);
	} else {
		pr_err(MODULE_NAME ": Can't request firmware '%s'\n", name);
	}

	release_firmware(fw_entry);
	return err;
}

/* Print test result to sysfs file. */
static ssize_t sys_result(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", test_result);
}
static DEVICE_ATTR(result, S_IRUSR, sys_result, NULL);

/* Get the number of firmware files and perform firmware requests. */
static ssize_t sys_fwnum(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int err, fw, fw_num;

	err = kstrtoint(buf, 10, &fw_num);
	if (err || fw_num <= 0 || fw_num > 32) {
		pr_err(MODULE_NAME ": Unexpected number of firmwares '%s'", buf);
		return err ? err : -EINVAL;
	}

	test_result = 0;

	for (fw = 0; fw < fw_num; ++fw) {
		char name[MAX_NAME];

		snprintf(name, sizeof(name), "n%d_%s", fw, fw_name);
		err = try_request_fw(name, (u8)fw);
		test_result |= (err == 0) << fw;
	}

	return count;
}
static DEVICE_ATTR(fwnum, S_IWUSR, NULL, sys_fwnum);

static int test_init(void)
{
	int err;

	err = device_register(&tdev);
	if (err) {
		pr_err(MODULE_NAME ": Unable to register device\n");
		return err;
	}
	pr_info(MODULE_NAME ": device registered\n");

	err = device_create_file(&tdev, &dev_attr_result);
	if (err) {
		pr_err(MODULE_NAME ": Can't create sysfs file 'result'\n");
		device_unregister(&tdev);
		return err;
	}

	err = device_create_file(&tdev, &dev_attr_fwnum);
	if (err) {
		pr_err(MODULE_NAME ": Can't create sysfs file 'fwnum'\n");
		device_remove_file(&tdev, &dev_attr_result);
		device_unregister(&tdev);
	}

	return err;
}
module_init(test_init);

static void test_exit(void)
{
	device_remove_file(&tdev, &dev_attr_result);
	device_remove_file(&tdev, &dev_attr_fwnum);

	device_unregister(&tdev);
	pr_info(MODULE_NAME ": module exited\n");
}
module_exit(test_exit);
