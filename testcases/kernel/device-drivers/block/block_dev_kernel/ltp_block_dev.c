// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 *
 * Only those functions are tested here which are declared in <linux/fs.h>
 *
 * Changes:
 * 16 Jan 2009  0.2  Added "tc" parameter to run test cases separately
 * 11 Jan 2009  0.1  First release
 */

#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#ifndef DISK_NAME_LEN
# include <linux/genhd.h>
#endif

MODULE_AUTHOR("Márton Németh <nm127@freemail.hu>");
MODULE_AUTHOR("Copyright (c) 2013 Oracle and/or its affiliates");
MODULE_DESCRIPTION("Test block drivers");
MODULE_LICENSE("GPL");

#define BLK_DEV_NAME		"ltp_block_dev"
#define MAX_MAJOR		255

#define prk_err(fmt, ...) \
	pr_err(BLK_DEV_NAME ": " fmt, ##__VA_ARGS__)
#define prk_info(fmt, ...) \
	pr_info(BLK_DEV_NAME ": " fmt, ##__VA_ARGS__)
#define prk_debug(fmt, ...) \
	pr_debug(BLK_DEV_NAME ": " fmt, ##__VA_ARGS__)

/*
 * Analysis of "int register_blkdev(unsigned int major, const char *name)"
 *
 * Equivalence classes:
 *
 *  Parameter  | Values                   | Valid?  | Covered in
 *  -----------+--------------------------+---------+-------------
 *  major      | [0]                      | valid   | tc01, tc02
 *             |--------------------------+---------+-------------
 *             | [1..255]                 | valid   | tc03
 *             |--------------------------+---------+-------------
 *             | [256..511]               | valid   | tc04
 *             | [512..UINT_MAX]          | invalid | tc05
 *  -----------+--------------------------+---------+-------------
 *  name       | [valid pointer to a zero |         |
 *             |  terminated string]      | valid   | tc01, tc02
 *             |--------------------------+---------+-------------
 *             | [valid pointer to a zero |         |
 *             |  length zero terminated  | valid   | tc06
 *             |  string]                 |         |
 *             |--------------------------+---------+-------------
 *             | [NULL]                   | invalid | tc08, tc09
 *  -----------+--------------------------+---------+-------------
 *
 */

#define result_str(pass) ((pass == 0) ? ("FAIL") : ("PASS"))

/*
 * bit mask for each test-case,
 * if test is passed, bit will be set to 1
 */
static int test_result;

static void device_release(struct device *dev)
{
	prk_info("device released\n");
}

static struct device tdev = {
	.init_name	= BLK_DEV_NAME,
	.release	= device_release,
};

static int tc01(void)
{
	int major1, major2;
	int pass = 1;

	prk_info("Test Case 1: register_blkdev() with auto allocating "
		"major numbers (major=0)\n");

	major1 = register_blkdev(0, BLK_DEV_NAME);
	prk_debug("major1 = %i\n", major1);

	major2 = register_blkdev(0, BLK_DEV_NAME);
	prk_debug("major2 = %i\n", major2);

	if (major1 >= 0) {
		unregister_blkdev(major1, BLK_DEV_NAME);
	} else {
		pass = 0;
		prk_debug("1st call to register_blkdev() failed, error %i\n",
			major1);
	}

	if (major2 >= 0) {
		unregister_blkdev(major2, BLK_DEV_NAME);
	} else {
		pass = 0;
		prk_debug("2nd call to register_blkdev() failed, error %i\n",
			major2);
	}

	prk_info("Test Case Result: %s\n", result_str(pass));
	return pass;
}

static int tc02(void)
{
	int major[MAX_MAJOR + 1];
	int i, pass = 2;

	/* Try to allocate block devices until all major numbers are used.
	 * After this register_blkdev() should return -EBUSY
	 */

	prk_info("Test Case 2: stress test of register_blkdev() "
		"with auto allocating major numbers (major=0)\n");

	memset(major, 0, sizeof(major));

	for (i = 0; i < sizeof(major) / sizeof(*major); ++i) {
		major[i] = register_blkdev(0, BLK_DEV_NAME);
		prk_debug("major[%i] = %i\n", i, major[i]);

		if (major[i] == -EBUSY) {
			prk_info("device is busy, register_blkdev() ret %i\n",
				major[i]);
		} else if (major[i] < 0) {
			prk_debug("register_blkdev() failed with error %i\n",
				major[i]);
			pass = 0;
		}
	}

	for (i = 0; i < sizeof(major) / sizeof(*major); ++i) {
		if (major[i] >= 0)
			unregister_blkdev(major[i], BLK_DEV_NAME);
	}

	prk_info("Test Case Result: %s\n", result_str(pass));
	return pass;
}

static int tc03(void)
{
	int major, major2, major3;
	int pass = 4;

	prk_info("Test Case 3: register_blkdev() with major != 0\n");

	/* autosearch for a free major number */
	major = register_blkdev(0, BLK_DEV_NAME);
	prk_debug("major = %i\n", major);

	if (major > 0) {
		unregister_blkdev(major, BLK_DEV_NAME);

		/* expected to return 0 */
		major2 = register_blkdev(major, BLK_DEV_NAME);

		/* this call has to fail with EBUSY return value */
		major3 = register_blkdev(major, BLK_DEV_NAME);

		if (major2 == 0) {
			unregister_blkdev(major, BLK_DEV_NAME);
		} else {
			pass = 0;
			prk_debug("1st call to register_blkdev() with major=%i "
				"failed with error %i\n", major, major2);
		}

		if (major3 == 0) {
			unregister_blkdev(major, BLK_DEV_NAME);
			pass = 0;
		} else {
			if (major3 != -EBUSY)
				pass = 0;
			prk_debug("2nd call to register_blkdev() with major=%i "
				"failed with error %i\n", major, major3);
		}

	} else {
		pass = 0;
		prk_debug("register_blkdev() failed with error %i\n", major);
	}

	prk_info("Test Case Result: %s\n", result_str(pass));
	return pass;
}

static int tc04(void)
{
	int major, pass = 8;
	unsigned int i, test_major[2] = {256, 511};

	prk_info("Test Case 4: register_blkdev() with major=%u/%u\n",
		 test_major[0], test_major[1]);

	for (i = 0; i < sizeof(test_major) / sizeof(unsigned int); i++) {
		major = register_blkdev(test_major[i], BLK_DEV_NAME);
		prk_debug("major = %i\n", major);

		if (major == 0) {
			unregister_blkdev(test_major[i], BLK_DEV_NAME);
		} else if (major == -EBUSY) {
			prk_debug("device was busy, register_blkdev() with major %u skipped\n",
				  test_major[i]);
		} else {
			pass = 0;
			prk_debug("register_blkdev() with major %u got error %i\n",
				  test_major[i], major);
		}
	}

	prk_info("Test Case Result: %s\n", result_str(pass));
	return pass;
}

static int tc05(void)
{
	int major, pass = 16;
	unsigned int i, test_major[2] = {512, UINT_MAX};

	prk_info("Test Case 5: register_blkdev() with major=%u/%u\n",
		 test_major[0], test_major[1]);

	for (i = 0; i < sizeof(test_major) / sizeof(unsigned int); i++) {
		major = register_blkdev(test_major[i], BLK_DEV_NAME);
		prk_debug("major = %i\n", major);

		if (major == 0) {
			unregister_blkdev(test_major[i], BLK_DEV_NAME);
#ifdef BLKDEV_MAJOR_MAX
			pass = 0;
#endif
		} else {
			prk_debug("register_blkdev() with major %u got error %i\n",
				  test_major[i], major);
#ifndef BLKDEV_MAJOR_MAX
			pass = 0;
#endif
		}
	}

	prk_info("Test Case Result: %s\n", result_str(pass));
	return pass;
}

static int tc06(void)
{
	int major, pass = 32;

	prk_info("Test Case 6: register_blkdev() with name=\"\"\n");

	major = register_blkdev(0, "");
	prk_debug("major = %i\n", major);

	if (major >= 0) {
		unregister_blkdev(major, "");
	} else {
		pass = 0;
		prk_debug("register_blkdev() failed with error %i\n", major);
	}

	prk_info("Test Case Result: %s\n", result_str(pass));
	return pass;
}

static int tc07(void)
{
	int major, pass = 64;

	prk_info("Test Case 7: unregister_blkdev() with major=0\n");

	major = register_blkdev(0, BLK_DEV_NAME);
	prk_debug("major = %i\n", major);

	if (major >= 0) {
		prk_debug("calling unregister_blkdev() with major=0\n");
		unregister_blkdev(0, BLK_DEV_NAME);
		prk_debug("calling unregister_blkdev() with major=%i\n", major);
		unregister_blkdev(major, BLK_DEV_NAME);
	} else {
		pass = 0;
		prk_debug("register_blkdev() failed with error %i\n", major);
	}

	prk_info("Test Case Result: %s\n", result_str(pass));
	return pass;
}

static int tc08(void)
{
	int major, pass = 128;

	prk_info("Test Case 8: register_blkdev() with name=NULL\n");

	/* should fail with -EINVAL */
	major = register_blkdev(0, NULL);
	prk_debug("major = %i\n", major);

	if (major >= 0) {
		unregister_blkdev(major, NULL);
		pass = 0;
	} else {
		prk_debug("register_blkdev() failed with error %i\n", major);
	}

	prk_info("Test Case Result: %s\n", result_str(pass));
	return pass;
}

static int tc09(void)
{
	int major, pass = 256;

	prk_info("Test Case 9: unregister_blkdev() with name=NULL\n");

	major = register_blkdev(0, BLK_DEV_NAME);
	prk_debug("major = %i\n", major);

	if (major >= 0) {
		/* kernel should silently ignore this */
		unregister_blkdev(major, NULL);
		unregister_blkdev(major, BLK_DEV_NAME);
	} else {
		pass = 0;
		prk_debug("register_blkdev() failed with error %i\n", major);
	}

	prk_info("Test Case Result: %s\n", result_str(pass));
	return pass;
}

/* print test result to sysfs file */
static ssize_t sys_result(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", test_result);
}
static DEVICE_ATTR(result, S_IRUSR, sys_result, NULL);

/*
 * get test-case number and run it
 */
static ssize_t sys_tcase(struct device *dev,
	struct device_attribute *attr,  const char *buf, size_t count)
{
	int tc = 0;
	sscanf(buf, "%d", &tc);
	if (tc < 0 || tc > 9) {
		prk_err(": Unexpected test-case number '%d'", tc);
		return count;
	}

	test_result = 0;

	if (tc == 0 || tc == 1)
		test_result |= tc01();

	if (tc == 0 || tc == 2)
		test_result |= tc02();

	if (tc == 0 || tc == 3)
		test_result |= tc03();

	if (tc == 0 || tc == 4)
		test_result |= tc04();

	if (tc == 0 || tc == 5)
		test_result |= tc05();

	if (tc == 0 || tc == 6)
		test_result |= tc06();

	if (tc == 0 || tc == 7)
		test_result |= tc07();

	if (tc == 0 || tc == 8)
		test_result |= tc08();

	if (tc == 0 || tc == 9)
		test_result |= tc09();

	return count;
}
static DEVICE_ATTR(tcase, S_IWUSR, NULL, sys_tcase);

static int test_init_module(void)
{
	int err = 0;
	prk_info("Starting module\n");

	err = device_register(&tdev);
	if (err) {
		prk_err("Unable to register device\n");
		goto err0;
	}
	prk_info("device registered\n");

	err = device_create_file(&tdev, &dev_attr_result);
	if (err) {
		prk_err("Can't create sysfs file 'result'\n");
		goto err1;
	}

	err = device_create_file(&tdev, &dev_attr_tcase);
	if (err) {
		prk_err(": Can't create sysfs file 'tc'\n");
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
module_init(test_init_module);

static void test_exit_module(void)
{
	prk_debug("Unloading module\n");
	device_remove_file(&tdev, &dev_attr_result);
	device_remove_file(&tdev, &dev_attr_tcase);
	device_unregister(&tdev);
}
module_exit(test_exit_module);
