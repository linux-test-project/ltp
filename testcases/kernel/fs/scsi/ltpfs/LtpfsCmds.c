
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/pm.h>
#include <linux/genhd.h>
#include <linux/bio.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/bio.h>
#include <linux/blk.h>
#include <linux/slab.h>
#include <linux/mempool.h>
#include <linux/workqueue.h>
#include <linux/namei.h>
#include <linux/mount.h>
#include <linux/quotaops.h>
#include <linux/pagemap.h>
#include <linux/dnotify.h>
#include <linux/smp_lock.h>
#include <linux/personality.h>
#include <linux/security.h>
#include <linux/buffer_head.h>
#include <asm/namei.h>
#include <asm/uaccess.h>

#define ACC_MODE(x) ("\000\004\002\006"[(x)&O_ACCMODE])
#define IS_POSIX(fl)	(fl->fl_flags & FL_POSIX)
#define TEST_MEM_SIZE  4096
#define FALSE          0
#include "Ltpfs.h"

static int ltpdev_open(struct inode *inode, struct file *pfile);
static int ltpdev_release(struct inode *inode, struct file *pfile);
static int ltpdev_ioctl(struct inode *pinode, struct file *pfile,
			unsigned int cmd, unsigned long arg);
static int do_buffer_c_tests(void);

static struct block_device_operations blkops = {
open:	ltpdev_open,
release:ltpdev_release,
ioctl:	ltpdev_ioctl,
};

int ltp_fs_major = LTPMAJOR;
int test_iteration = 0;

static char genhd_flags = 0;
static struct gendisk *gd_ptr;
static spinlock_t bdev_lock __cacheline_aligned_in_smp = SPIN_LOCK_UNLOCKED;

MODULE_AUTHOR("Martin Ridgeway <mridge@us.ibm.com>");
MODULE_DESCRIPTION(FS_LTP_TEST_DRIVER_NAME);
MODULE_LICENSE("GPL");

/*
 * Device operations for the virtual FS devices
 */

static struct pm_dev *ltp_pm_dev = NULL;
struct block_device *ltplookup_bdev(const char *path);
int path_lookup(const char *name, unsigned int flags, struct nameidata *nd);
//static int __emul_lookup_dentry(const char *name, struct nameidata *nd);
void path_release(struct nameidata *nd);

static int ltpdev_open(struct inode *pinode, struct file *pfile)
{
	printk(KERN_ALERT "ltpdev_open \n");
	return 0;
}

static int ltpdev_release(struct inode *pinode, struct file *pfile)
{

	printk(KERN_ALERT "ltpdev_release \n");
	return 0;
}

static int ltpdev_ioctl(struct inode *pinode, struct file *pfile,
			unsigned int cmd, unsigned long arg)
{

	struct bio *my_bio = NULL;
	struct bio *my_bio_copy = NULL;
	request_queue_t *q = NULL;
	struct block_device *bdev = NULL;
	unsigned long uaddr;

	unsigned int bytes_done = 100;

	int error = 0;
	int rc = 0;

    /*****************************************************************************/

	printk(KERN_ALERT "ltpdev_ioctl fs tests\n");

	switch (cmd) {

	case LTPAIODEV_CMD:
		printk(KERN_ALERT "Running AIO FS tests \n");
		printk(KERN_ALERT "AIO FS tests complete\n");
		break;

	case LTPBIODEV_CMD:

		printk(KERN_ALERT "Running BIO FS tests \n");

		my_bio = bio_alloc(GFP_KERNEL, 0);
		if (!my_bio) {
			printk(KERN_ALERT
			       "Error getting kernel slab memory !!\n");
		} else {
			printk(KERN_ALERT "kernel slab memory alloc OK\n");
		}

		bio_endio(my_bio, bytes_done, error);

		printk(KERN_ALERT "Return from bio_endio = %d \n", error);

		my_bio_copy = bio_clone(my_bio, GFP_ATOMIC);

		if (!my_bio_copy) {
			printk(KERN_ALERT
			       "Error getting kernel bio clone !!\n");
		} else {
			printk(KERN_ALERT "kernel bio clone OK\n");
		}

		my_bio_copy = bio_clone(my_bio, GFP_NOIO);

		if (!my_bio_copy) {
			printk(KERN_ALERT
			       "Error getting kernel bio clone !!\n");
		} else {
			printk(KERN_ALERT "kernel bio clone OK\n");
		}

//        q = bdev_get_queue(my_bio->bi_bdev);

//        rc = bio_phys_segments(q, my_bio);

//        rc = bio_hw_segments(q, my_bio);

		bdev = lookup_bdev(LTP_FS_DEVICE_NAME);

		printk(KERN_ALERT "return from bdev size %d\n",
		       bdev->bd_block_size);

		printk(KERN_ALERT "Return from phys_segments = %d \n", rc);

//        Don't use this API, causes system to hang and corrupts FS
//        bio_put(my_bio);

		(char *)uaddr = kmalloc(TEST_MEM_SIZE, GFP_KERNEL);

		my_bio_copy = bio_map_user(bdev, uaddr, TEST_MEM_SIZE, FALSE);

		printk(KERN_ALERT "Return from bio_map_user %p\n", my_bio_copy);

		do_buffer_c_tests();

		printk(KERN_ALERT "BIO FS tests complete\n");

		break;
	}

	return 0;
}

static int ltp_pm_callback(struct pm_dev *dev, pm_request_t rqst, void *data)
{
	return 0;
}

int init_module(void)
{
	int result;

	printk(KERN_ALERT "ltpdev_init_module \n");

	ltp_pm_dev = pm_register(PM_UNKNOWN_DEV, 0, ltp_pm_callback);

	result = register_blkdev(ltp_fs_major, LTP_FS_DEV_NAME);

	printk(KERN_ALERT "LTP FS: register_blkdev result=%d major %d\n",
	       result, ltp_fs_major);

	if (result < 0) {
		printk(KERN_ALERT "LTP FS: can't get major %d\n", ltp_fs_major);
		return result;
	}

	gd_ptr = kmalloc(sizeof(struct gendisk *), GFP_KERNEL);

	if (!gd_ptr) {
		printk(KERN_ALERT "ERROR getting memory !!!\n");
		return 0;
	}

	gd_ptr = alloc_disk(1);

	printk(KERN_ALERT "gd_ptr after alloc = %p \n", gd_ptr);

	gd_ptr->major = ltp_fs_major;
	gd_ptr->first_minor = 0;
	gd_ptr->fops = &blkops;
	gd_ptr->driverfs_dev = NULL;
	gd_ptr->capacity = MAX_NUM_DISKS;
	gd_ptr->flags = genhd_flags;

	sprintf(gd_ptr->disk_name, LTP_FS_DEV_NAME);

	add_disk(gd_ptr);

	return 0;
}

void cleanup_module(void)
{

	printk(KERN_ALERT "Exiting module and cleaning up \n");

	pm_unregister(ltp_pm_dev);

	put_disk(gd_ptr);

	del_gendisk(gd_ptr);

	unregister_blkdev(ltp_fs_major, LTP_FS_DEV_NAME);

}

static int do_buffer_c_tests()
{
	int line_no = 0;

	printk(KERN_ALERT "Starting buffer.c coverage tests... \n");

	__buffer_error("Test file", line_no);

	printk(KERN_ALERT "buffer.c coverage tests complete...\n");

	return 0;
}

/**
 * lookup_bdev  - lookup a struct block_device by name
 *
 * @path:	special file representing the block device
 *
 * Get a reference to the blockdevice at @path in the current
 * namespace if possible and return it.  Return ERR_PTR(error)
 * otherwise.
 */
struct block_device *lookup_bdev(const char *path)
{
	struct block_device *bdev;
	struct inode *inode;
	struct nameidata nd;
	int error;

	if (!path || !*path)
		return ERR_PTR(-EINVAL);

	error = path_lookup(path, LOOKUP_FOLLOW, &nd);
	if (error)
		return ERR_PTR(error);

	inode = nd.dentry->d_inode;
	error = -ENOTBLK;
	if (!S_ISBLK(inode->i_mode))
		goto fail;
	error = -EACCES;
	if (nd.mnt->mnt_flags & MNT_NODEV)
		goto fail;
	error = bd_acquire(inode);
	if (error)
		goto fail;
	bdev = inode->i_bdev;

out:
	path_release(&nd);
	return bdev;
fail:
	bdev = ERR_PTR(error);
	goto out;
}

int bd_acquire(struct inode *inode)
{
	struct block_device *bdev;
	spin_lock(&bdev_lock);
	if (inode->i_bdev) {
		atomic_inc(&inode->i_bdev->bd_count);
		spin_unlock(&bdev_lock);
		return 0;
	}
	spin_unlock(&bdev_lock);
	bdev = bdget(kdev_t_to_nr(inode->i_rdev));
	if (!bdev)
		return -ENOMEM;
	spin_lock(&bdev_lock);
	if (!inode->i_bdev) {
		inode->i_bdev = bdev;
		inode->i_mapping = bdev->bd_inode->i_mapping;
		list_add(&inode->i_devices, &bdev->bd_inodes);
	} else if (inode->i_bdev != bdev)
		BUG();
	spin_unlock(&bdev_lock);
	return 0;
}
