
/*
 * Module under test: linux/block/genhd.c
 *
 * Only those functions are tested here which are declared in <linux/genhd.h>
 *
 * Usage:
 *   1. make
 *   2. su
 *   3. insmod ./test_genhd.ko
 *   4. Check the test results in "dmesg"
 *   5. rmmod test_genhd
 */

#include <linux/module.h>
#include <linux/genhd.h>

MODULE_AUTHOR("Márton Németh <nm127@freemail.hu>");
MODULE_DESCRIPTION("Test block drivers");
MODULE_LICENSE("GPL");

#define BLK_DEV_NAME		"test_block"
#define MAX_MAJOR		255

static void tc20(void)
{
	struct gendisk *gd_ptr;

	gd_ptr = alloc_disk(1);
	if (!gd_ptr) {
		return;
	}
	printk(KERN_DEBUG "gd_ptr after alloc=%p\n", gd_ptr);

	del_gendisk(gd_ptr);
}

static int test_init_module(void)
{
	printk(KERN_INFO "Starting test_genhd module\n");

	tc20();

	return 0;
}

static void test_exit_module(void)
{
	printk(KERN_DEBUG "Unloading test_genhd module\n");
}

module_init(test_init_module);
module_exit(test_exit_module);
