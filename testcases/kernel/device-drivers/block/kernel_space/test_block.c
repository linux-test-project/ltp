
/*
 * Module under test: linux/block/genhd.c
 *
 * Only those functions are tested here which are declared in <linux/fs.h>
 *
 * Usage:
 *   1. make
 *   2. su
 *   3. insmod ./test_block.ko
 *   4. Check the test results in "dmesg"
 *   5. rmmod test_block
 *
 * Changes:
 * 16 Jan 2009  0.2  Added "tc" parameter to run test cases separately
 * 11 Jan 2009  0.1  First release
 */

#include <linux/module.h>
#include <linux/fs.h>

static unsigned int __initdata tc;
module_param_named(tc, tc, int, 0);
MODULE_PARM_DESC(tc, "Test Case to run. Default is 0 which means that run all tests.");

MODULE_AUTHOR("Márton Németh <nm127@freemail.hu>");
MODULE_DESCRIPTION("Test block drivers");
MODULE_LICENSE("GPL");

#define BLK_DEV_NAME		"test_block"
#define MAX_MAJOR		255

/*
 * Analysis of "int register_blkdev(unsigned int major, const char *name)"
 *
 * Equivalence classes:
 *
 *  Parameter  | Values                   | Valid?  | Covered in
 *  -----------+--------------------------+---------+-------------
 *  major      | [0]                      | valid   | tc01, tc02
 *             |--------------------------+---------+-------------
 *             | [1..255]                 | valid   | tc05
 *             |--------------------------+---------+-------------
 *             | [256..UINT_MAX]          | invalid | tc03, tc04
 *  -----------+--------------------------+---------+-------------
 *  name       | [valid pointer to a zero |         |
 *             |  terminated string]      | valid   | tc01, tc02
 *             |--------------------------+---------+-------------
 *             | [valid pointer to a zero |         |
 *             |  length zero terminated  | invalid | tc06
 *             |  string]                 |         |
 *             |--------------------------+---------+-------------
 *             | [NULL]                   | invalid | tc07
 *  -----------+--------------------------+---------+-------------
 *
 */

static void tc01(void)
{
	int major1;
	int major2;

	printk(KERN_INFO "Test Case 1: register_blkdev() with auto allocating major numbers (major=0)\n");

	major1 = register_blkdev(0, BLK_DEV_NAME);
	printk(KERN_DEBUG "major1 = %i\n", major1);

	major2 = register_blkdev(0, BLK_DEV_NAME);
	printk(KERN_DEBUG "major2 = %i\n", major2);

	if (0 < major1) {
		unregister_blkdev(major1, BLK_DEV_NAME);
	} else {
		printk(KERN_DEBUG "first call to register_blkdev() failed with error %i\n", major1);
	}

	if (0 < major2) {
		unregister_blkdev(major2, BLK_DEV_NAME);
	} else {
		printk(KERN_DEBUG "second call to register_blkdev() failed with error %i\n", major2);
	}

	printk(KERN_INFO "Test Case 1: UNRESLOVED\n");
}

static void tc02(void)
{
	int major[MAX_MAJOR+1];
	int i;

	/* Try to allocate block devices until all major number are used. After this
	 * register_blkdev() should return -EBUSY
	 */

	printk(KERN_INFO "Test Case 2: stress test of register_blkdev() with auto allocating major numbers (major=0)\n");

	memset(major, 0, sizeof(major));

	for (i=0; i<sizeof(major)/sizeof(*major); i++) {
		major[i] = register_blkdev(0, BLK_DEV_NAME);
		printk(KERN_DEBUG "major[%i] = %i\n", i, major[i]);
		if (major[i] <= 0 && major[i] != -EBUSY) {
			printk(KERN_INFO "unexpected return value from register_blkdev(): %i\n", major[i]);
		}
		if (major[i] < 0) {
			printk(KERN_DEBUG "register_blkdev() failed with error %i\n", major[i]);
		}
	}

	for (i=0; i<sizeof(major)/sizeof(*major); i++) {
		if (0 < major[i]) {
			unregister_blkdev(major[i], BLK_DEV_NAME);
			major[i] = 0;
		}
	}

	printk(KERN_INFO "Test Case 2: UNRESLOVED\n");
}

static void tc03(void)
{
	int major;

	printk(KERN_INFO "Test Case 3: register_blkdev() with major=256\n");

	major = register_blkdev(256, BLK_DEV_NAME);
	printk(KERN_DEBUG "major = %i\n", major);

	if (0 < major) {
		unregister_blkdev(major, BLK_DEV_NAME);
	} else {
		printk(KERN_DEBUG "register_blkdev() failed with error %i\n", major);
	}

	printk(KERN_INFO "Test Case 3: UNRESLOVED\n");
}

static void tc04(void)
{
	int major;

	printk(KERN_INFO "Test Case 4: register_blkdev() with major=%u\n", UINT_MAX);

	major = register_blkdev(UINT_MAX, BLK_DEV_NAME);
	printk(KERN_DEBUG "major = %i\n", major);

	if (0 < major) {
		unregister_blkdev(major, BLK_DEV_NAME);
	} else {
		printk(KERN_DEBUG "register_blkdev() failed with error %i\n", major);
	}

	printk(KERN_INFO "Test Case 4: UNRESLOVED\n");
}

static void tc05(void)
{
	int major;
	int major2;
	int major3;

	printk(KERN_INFO "Test Case 5: register_blkdev() with major != 0\n");

	/* autosearch for a free major number */
	major = register_blkdev(0, BLK_DEV_NAME);
	printk(KERN_DEBUG "major = %i\n", major);

	if (0 < major) {
		unregister_blkdev(major, BLK_DEV_NAME);

		major2 = register_blkdev(major, BLK_DEV_NAME);
		major3 = register_blkdev(major, BLK_DEV_NAME);

		if (0 < major2) {
			unregister_blkdev(major2, BLK_DEV_NAME);
		} else {
			printk(KERN_DEBUG "first call to register_blkdev() with major=%i failed with error %i\n",
				major, major2);
		}

		if (0 < major3) {
			unregister_blkdev(major3, BLK_DEV_NAME);
		} else {
			printk(KERN_DEBUG "second call to register_blkdev() with major=%i failed with error %i\n",
				major, major3);
		}

	} else {
		printk(KERN_DEBUG "register_blkdev() failed with error %i\n", major);
	}

	printk(KERN_INFO "Test Case 5: UNRESLOVED\n");
}

static void tc06(void)
{
	int major;

	printk(KERN_INFO "Test Case 6: register_blkdev() with name=\"\"\n");

	major = register_blkdev(0, "");
	printk(KERN_DEBUG "major = %i\n", major);

	if (0 < major) {
		unregister_blkdev(major, "");
	} else {
		printk(KERN_DEBUG "register_blkdev() failed with error %i\n", major);
	}

	printk(KERN_INFO "Test Case 6: UNRESLOVED\n");
}

static void tc07(void)
{
	int major;

	printk(KERN_INFO "Test Case 7: register_blkdev() with name=NULL\n");

	major = register_blkdev(0, NULL);
	printk(KERN_DEBUG "major = %i\n", major);

	if (0 < major) {
		unregister_blkdev(major, NULL);
	} else {
		printk(KERN_DEBUG "register_blkdev() failed with error %i\n", major);
	}

	printk(KERN_INFO "Test Case 7: UNRESLOVED\n");
}

static void tc10(void) {
	int major;

	printk(KERN_INFO "Test Case 10: unregister_blkdev() with major=0\n");

	major = register_blkdev(0, BLK_DEV_NAME);
	printk(KERN_DEBUG "major = %i\n", major);

	if (0 < major) {
		printk(KERN_DEBUG "calling unregister_blkdev() with major=0\n");
		unregister_blkdev(0, BLK_DEV_NAME);
		printk(KERN_DEBUG "calling unregister_blkdev() with major=%i\n", major);
		unregister_blkdev(major, BLK_DEV_NAME);
	} else {
		printk(KERN_DEBUG "register_blkdev() failed with error %i\n", major);
	}

	printk(KERN_INFO "Test Case 10: UNRESLOVED\n");
}

static int test_init_module(void)
{
	printk(KERN_INFO "Starting test_block module\n");

	if (tc == 0 || tc == 1)
		tc01();

	if (tc == 0 || tc == 2)
		tc02();

	if (tc == 0 || tc == 3)
		tc03();

	if (tc == 0 || tc == 4)
		tc04();

	if (tc == 0 || tc == 5)
		tc05();

	if (tc == 0 || tc == 6)
		tc06();

	if (tc == 0 || tc == 7)
		tc07();

	if (tc == 0 || tc == 10)
		tc10();

	return 0;
}

static void test_exit_module(void)
{
	printk(KERN_DEBUG "Unloading test_block module\n");
}

module_init(test_init_module);
module_exit(test_exit_module);