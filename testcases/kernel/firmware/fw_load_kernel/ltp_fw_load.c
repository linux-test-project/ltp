/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Author:
 * Alexey Kodanev <alexey.kodanev@oracle.com>
 *
 * This module is trying to load external test firmware files (n#_load_tst.fw).
 * In the end, it writes results to /sys/devices/ltp_fw_load/result file.
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

#define TCID	"ltp_fw_load"

static char *fw_name	= "load_tst.fw";
static int fw_size	= 0x1000;
static int max_name	= 64;
static int fw;

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
	pr_info(TCID ": device released\n");
}

static struct device tdev = {
	.init_name	= TCID,
	.release	= device_release,
};

/* read and print firmware data */
static int fw_read(const u8 *data, size_t size)
{
	size_t i;
	pr_info(TCID ": Firmware has size '%zu'\n", size);
	if (size != fw_size) {
		pr_err(TCID ": Expected firmware size '%d'\n", fw_size);
		return -1;
	}
	for (i = 0; i < size; ++i) {
		if (data[i] != (u8)fw) {
			pr_err(TCID ": Unexpected firmware data\n");
			return -1;
		}
	}
	return 0;
}

static int try_request_fw(const char *name)
{
	int err;
	const struct firmware *fw_entry = NULL;
	err = request_firmware(&fw_entry, name, &tdev);
	if (!err) {
		pr_info(TCID ": firmware '%s' requested\n", name);
		err = fw_read(fw_entry->data, fw_entry->size);
	} else
		pr_err(TCID ": Can't request firmware '%s'\n", name);
	release_firmware(fw_entry);
	return err;
}

/* print test result to sysfs file */
static ssize_t sys_result(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", test_result);
}
static DEVICE_ATTR(result, S_IRUSR, sys_result, NULL);

/*
 * get the number of firmware files and
 * perform firmware requests
 */
static ssize_t sys_fwnum(struct device *dev,
	struct device_attribute *attr,  const char *buf, size_t count)
{
	int err, fw_num = 0;

	sscanf(buf, "%d", &fw_num);
	if (fw_num <= 0 || fw_num > 32) {
		pr_err(TCID ": Unexpected number of firmwares '%d'", fw_num);
		return count;
	}
	for (fw = 0; fw < fw_num; ++fw) {
		char name[max_name];
		snprintf(name, max_name, "n%d_%s", fw, fw_name);
		err = try_request_fw(name);
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
		pr_err(TCID ": Unable to register device\n");
		return err;
	}
	pr_info(TCID ": device registered\n");

	err = device_create_file(&tdev, &dev_attr_result);
	if (err) {
		pr_err(TCID ": Can't create sysfs file 'result'\n");
		device_unregister(&tdev);
		return err;
	}
	err = device_create_file(&tdev, &dev_attr_fwnum);
	if (err) {
		pr_err(TCID ": Can't create sysfs file 'fwnum'\n");
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
	pr_info(TCID ": module exited\n");
}
module_exit(test_exit);
