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
 * Author: Alexey Kodanev <alexey.kodanev@oracle.com>
 *
 */

#include <linux/module.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include "ltp_uaccess.h"

MODULE_AUTHOR("Alexey Kodanev <alexey.kodanev@oracle.com>");
MODULE_DESCRIPTION("User-space access LTP test");
MODULE_LICENSE("GPL");

#define prk_err(fmt, ...) \
	pr_err(DEV_NAME ": " fmt "\n", ##__VA_ARGS__)
#define prk_info(fmt, ...) \
	pr_info(DEV_NAME ": " fmt "\n", ##__VA_ARGS__)

/*
 * Test-case result,
 * if test is passed, value will be set to 0
 */
static int test_result;

static void device_release(struct device *dev)
{
	prk_info("device released");
}

static struct device tdev = {
	.init_name	= DEV_NAME,
	.release	= device_release
};

static ssize_t sys_result(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", test_result);
}
static DEVICE_ATTR(result, S_IRUSR, sys_result, NULL);


static ssize_t sys_tcase(struct device *dev,
	struct device_attribute *attr,  const char *buf, size_t count)
{
	unsigned long ptr = 0;
	int tc = 0;
	char *str, ch, buffer[str_size];

	sscanf(buf, "%d %lu", &tc, &ptr);
	str = (char *) ptr;
	test_result = 0;

	switch (tc) {
	case TC_READ_USER:
		if (copy_from_user(buffer, str, str_size))
			prk_err("copy_from_user() failed");
		test_result = strncmp(test_str, buffer, str_size) ? 1 : 0;
		if (get_user(ch, str))
			prk_err("get_user() failed");
		test_result |= ch != test_str[0];
	break;
	case TC_WRITE_USER:
		if (copy_to_user(str + 1, test_str + 1, str_size - 1)) {
			prk_err("copy_to_user() failed");
			test_result = 1;
		}
		/* write the first skipped character */
		if (put_user(test_str[0], str)) {
			prk_err("put_user() failed");
			test_result |= 1;
		}
	break;
	}

	return count;
}
static DEVICE_ATTR(tcase, S_IWUSR, NULL, sys_tcase);

static int uaccess_init(void)
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

	err = device_create_file(&tdev, &dev_attr_tcase);
	if (err) {
		prk_err(": Can't create sysfs file 'tc'");
		goto err2;
	}

	return 0;

err2:
	device_remove_file(&tdev, &dev_attr_result);
err1:
	device_unregister(&tdev);
err0:
	return err;
}
module_init(uaccess_init)

static void uaccess_exit(void)
{
	prk_info("Unloading module");

	device_remove_file(&tdev, &dev_attr_result);
	device_remove_file(&tdev, &dev_attr_tcase);
	device_unregister(&tdev);
}
module_exit(uaccess_exit)
