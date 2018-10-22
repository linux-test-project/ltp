/*
 * Copyright (c) International Business Machines  Corp., 2001
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
 * This module will test block io layer.
 *
 * module: tbio
 *
 *  FILE        : tbio.c
 *  USAGE       : kernel_space:./load_tbio.sh
 *                user_space  :./test_bio
 *
 *  DESCRIPTION : The module will test block i/o layer for kernel 2.5
 *  REQUIREMENTS:
 *                1) glibc 2.1.91 or above.
 *
 *  HISTORY     :
 *      11/19/2003 Kai Zhao (ltcd3@cn.ibm.com)
 *
 *  CODE COVERAGE: 74.9% - fs/bio.c (Total Coverage)
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>

#include "tbio.h"

MODULE_AUTHOR("Kai Zhao <ltcd3@cn.ibm.com>");
MODULE_AUTHOR("Alexey Kodanev <alexey.kodanev@oracle.com>");
MODULE_DESCRIPTION(TMOD_DRIVER_NAME);
MODULE_LICENSE("GPL");

#define prk_err(fmt, ...) \
	pr_err(TBIO_DEVICE_NAME ": " fmt "\n", ##__VA_ARGS__)
#define prk_info(fmt, ...) \
	pr_info(TBIO_DEVICE_NAME ": " fmt "\n", ##__VA_ARGS__)

static int nsectors = 4096;
module_param(nsectors, int, 0444);
MODULE_PARM_DESC(nsectors, "The number of sectors");

static struct bio *tbiop, *tbiop_dup;

static struct tbio_device {
	unsigned long size;
	spinlock_t lock;
	u8 *data;
	struct gendisk *gd;
	struct block_device *bdev;
	struct request_queue *q;
} tbio_dev;

static int send_request(struct request_queue *q, struct bio *bio,
	struct block_device *bdev, struct tbio_interface *inter,
	int writing)
{
	struct request *rq;
	rq = blk_make_request(q, bio, GFP_KERNEL);
	if (!rq) {
		prk_err("failed to make request");
		return -EFAULT;
	}

	if ((!inter->cmd_len) || (inter->cmd_len > rq->cmd_len)) {
		prk_err("invalid inter->cmd_len");
		return -EFAULT;
	}

	rq->cmd_len = inter->cmd_len;

	if (copy_from_user(rq->cmd, inter->cmd, inter->cmd_len))
		goto out_request;

	if (*(rq->cmd + rq->cmd_len - 1)) {
		prk_err("rq->cmd is not null-terminated");
		return -EFAULT;
	}

	rq->__sector = bio->bi_sector;

	if (blk_execute_rq(q, bdev->bd_disk, rq, 0))
		goto out_request;

	blk_put_request(rq);

	return 0;

out_request:

	blk_put_request(rq);
	return -EFAULT;
}

static int tbio_io(struct block_device *bdev, struct tbio_interface *uptr)
{
	int ret;
	tbio_interface_t inter;
	struct bio *bio = NULL;
	int reading = 0, writing = 0;
	void *buf = NULL;
	struct request_queue *q = bdev_get_queue(bdev);

	if (copy_from_user(&inter, uptr, sizeof(tbio_interface_t))) {
		prk_err("copy_from_user");
		return -EFAULT;
	}

	if (inter.data_len > (q->limits.max_sectors << 9)) {
		prk_err("inter.in_len > q->max_sectors << 9");
		return -EIO;
	}

	if (inter.data_len) {

		switch (inter.direction) {
		default:
			return -EINVAL;
		case TBIO_TO_DEV:
			writing = 1;
			break;
		case TBIO_FROM_DEV:
			reading = 1;
			break;
		}

		bio = bio_map_user(q, bdev, (unsigned long)inter.data,
			inter.data_len, reading, GFP_KERNEL);

		if (!bio) {
			prk_err("bio_map_user failed");
			buf = kmalloc(inter.data_len, q->bounce_gfp | GFP_USER);
			if (!buf) {
				prk_err("buffer no memory");
				return -1;
			}
			ret = copy_from_user(buf, inter.data, inter.data_len);
			if (ret)
				prk_err("copy_from_user() failed");

			prk_info("buffer %s\n, copy_from_user returns '%d'",
				(char *)buf, ret);
		}

	}

	send_request(q, bio, bdev, &inter, writing);

	if (bio)
		bio_unmap_user(bio);
	return 0;
}

static int test_bio_put(struct bio *biop)
{
	if (biop)
		bio_put(biop);

	return 0;
}

static int test_bio_clone(void)
{
	tbiop_dup = bio_clone(tbiop, GFP_NOIO);
	if (tbiop_dup == NULL) {
		prk_err("bio_clone failed");
		return -1;
	}

	test_bio_put(tbiop_dup);

	return 0;
}

static int test_bio_add_page(void)
{
	int ret = 0, i = 0, offset = 0;
	unsigned long addr = 0;
	struct page *ppage = NULL;

	for (i = 0; i < 128; i++) {

		addr = get_zeroed_page(GFP_KERNEL);

		if (addr == 0) {
			prk_err("get free page failed %ld", addr);
			ret = -1;
			break;
		}

		ppage = virt_to_page(addr);
		if (ppage == NULL) {
			prk_err("covert virture page to page struct failed");
			ret = -1;
			break;
		}

		ret = bio_add_page(tbiop, ppage, PAGE_SIZE, offset);
		if (ret < 0) {
			prk_err("bio_add_page failed");
			break;
		}
		offset += ret;
	}

	return ret;
}

static int test_do_bio_alloc(int num)
{
	tbiop = bio_alloc(GFP_KERNEL, num);
	if (tbiop == NULL) {
		prk_err("bio_alloc failed");
		return -1;
	}
	bio_put(tbiop);

	return 0;
}

static int test_bio_alloc(void)
{
	if (test_do_bio_alloc(2) < 0) {
		prk_err("can not alloc bio for %d", 2);
		return -1;
	}

	if (test_do_bio_alloc(8) < 0) {
		prk_err("can not alloc bio for %d", 8);
		return -1;
	}

	if (test_do_bio_alloc(32) < 0) {
		prk_err("can not alloc bio for %d", 32);
		return -1;
	}

	if (test_do_bio_alloc(96) < 0) {
		prk_err("can not alloc bio for %d", 96);
		return -1;
	}

	if (test_do_bio_alloc(BIO_MAX_PAGES) < 0) {
		prk_err("can not alloc bio for %d", BIO_MAX_PAGES);
		return -1;
	}

	tbiop = bio_alloc(GFP_KERNEL, BIO_MAX_PAGES);
	if (tbiop == NULL) {
		prk_err("bio_alloc failed");
		return -1;
	}

	tbiop->bi_bdev = tbio_dev.bdev;
	tbiop->bi_sector = 0;

	return 0;
}

static int test_bio_split(struct block_device *bdev,
			  struct tbio_interface *uptr)
{
	int ret;
	tbio_interface_t inter;
	struct bio *bio = NULL;
	struct bio_pair *bio_pairp = NULL;
	int reading = 0, writing = 0;
	void *buf = NULL;
	struct request_queue *q = bdev_get_queue(bdev);
	if (!q) {
		prk_err("bdev_get_queue() failed");
		return -EFAULT;
	}

	prk_info("test_bio_split");

	if (copy_from_user(&inter, uptr, sizeof(tbio_interface_t))) {
		prk_err("copy_from_user");
		return -EFAULT;
	}

	if (inter.data_len > (q->limits.max_sectors << 9)) {
		prk_err("inter.in_len > q->limits.max_sectors << 9");
		return -EIO;
	}

	prk_info("inter.data_len is %d", inter.data_len);
	if (inter.data_len) {

		switch (inter.direction) {
		default:
			return -EINVAL;
		case TBIO_TO_DEV:
			writing = 1;
			break;
		case TBIO_FROM_DEV:
			reading = 1;
			break;
		}

		bio = bio_map_user(q, bdev, (unsigned long)inter.data,
			inter.data_len, reading, GFP_KERNEL);

		if (!bio) {
			prk_err("bio_map_user failed");
			buf = kmalloc(inter.data_len, q->bounce_gfp | GFP_USER);
			if (!buf) {
				prk_err("buffer no memory");
				return -1;
			}
			ret = copy_from_user(buf, inter.data, inter.data_len);
			if (ret)
				prk_err("copy_from_user() failed");

			prk_info("buffer %s", (char *)buf);
		} else {
			bio_pairp = bio_split(bio, 2);

			if (bio_pairp == NULL) {
				prk_err("bio_split failed");
				bio_unmap_user(bio);
				return -1;
			}
		}

	}

	send_request(q, &(bio_pairp->bio1), bdev, &inter, writing);
	q = bdev_get_queue(bdev);
	send_request(q, &(bio_pairp->bio2), bdev, &inter, writing);

	if (bio_pairp)
		bio_pair_release(bio_pairp);

	if (bio)
		bio_unmap_user(bio);

	return 0;
}

static int test_bio_get_nr_vecs(void)
{
	int number = 0;

	number = bio_get_nr_vecs(tbio_dev.bdev);

	if (number < 0) {
		prk_err("bio_get_nr_vec failed");
		return -1;
	}

	prk_info("bio_get_nr_vecs: %d", number);
	return 0;
}

static int tbio_ioctl(struct block_device *blk, fmode_t mode,
	unsigned cmd, unsigned long arg)
{
	int err = 0;

	switch (cmd) {
	case LTP_TBIO_DO_IO:
		prk_info("TEST-CASE: LTP_TBIO_DO_IO:");
		err = tbio_io(tbio_dev.bdev, (struct tbio_interface *)arg);
		break;
	case LTP_TBIO_CLONE:
		prk_info("TEST-CASE: LTP_TBIO_CLONE:");
		err = test_bio_clone();
		break;
	case LTP_TBIO_ADD_PAGE:
		prk_info("TEST-CASE: LTP_TBIO_ADD_PAGE:");
		err = test_bio_add_page();
		break;
	case LTP_TBIO_ALLOC:
		prk_info("TEST-CASE: LTP_TBIO_ALLOC:");
		err = test_bio_alloc();
		break;
	case LTP_TBIO_GET_NR_VECS:
		prk_info("TEST-CASE: LTP_TBIO_GET_NR_VECS:");
		err = test_bio_get_nr_vecs();
		break;
	case LTP_TBIO_PUT:
		prk_info("TEST-CASE: LTP_TBIO_PUT:");
		err = test_bio_put(tbiop);
		break;
	case LTP_TBIO_SPLIT:
		prk_info("TEST-CASE: LTP_TBIO_SPLIT:");
		err = test_bio_split(tbio_dev.bdev,
			(struct tbio_interface *)arg);
	break;
	}

	prk_info("TEST-CASE DONE");
	return err;
}

static int tbio_transfer(struct request *req, struct tbio_device *dev)
{
	unsigned int i = 0, offset = 0;
	char *buf;
	unsigned long flags;
	size_t size;

	struct bio_vec *bv;
	struct req_iterator iter;

	size = blk_rq_cur_bytes(req);
	prk_info("bio req of size %zu:", size);
	offset = blk_rq_pos(req) * 512;

	rq_for_each_segment(bv, req, iter) {
		size = bv->bv_len;
		prk_info("%s bio(%u), segs(%u) sect(%u) pos(%lu) off(%u)",
			(bio_data_dir(iter.bio) == READ) ? "READ" : "WRITE",
			i, bio_segments(iter.bio), bio_sectors(iter.bio),
			iter.bio->bi_sector, offset);

		if (get_capacity(req->rq_disk) * 512 < offset) {
			prk_info("Error, small capacity %zu, offset %u",
				get_capacity(req->rq_disk) * 512,
				offset);
			continue;
		}

		buf = bvec_kmap_irq(bv, &flags);
		if (bio_data_dir(iter.bio) == WRITE)
			memcpy(dev->data + offset, buf, size);
		else
			memcpy(buf, dev->data + offset, size);
		offset += size;
		flush_kernel_dcache_page(bv->bv_page);
		bvec_kunmap_irq(buf, &flags);
		++i;
	}

	return 0;
}

static void tbio_request(struct request_queue *q)
{
	int ret = 0;
	struct request *req;

	while ((req = blk_fetch_request(q)) != NULL) {

		ret = tbio_transfer(req, &tbio_dev);

		spin_unlock_irq(q->queue_lock);
		blk_end_request_all(req, ret);
		spin_lock_irq(q->queue_lock);
	}
}

static int tbio_open(struct block_device *blk, fmode_t mode)
{
	tbio_dev.bdev = blk;

	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
static int tbio_release(struct gendisk *gd, fmode_t mode)
#else
static void tbio_release(struct gendisk *gd, fmode_t mode)
#endif
{

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
	return 0;
#endif
}

int tbio_media_changed(struct gendisk *gd)
{
	return 0;
}

int tbio_revalidate(struct gendisk *gd)
{
	return 0;
}

static const struct block_device_operations tbio_ops = {
	.owner = THIS_MODULE,
	.open = tbio_open,
	.ioctl = tbio_ioctl,
	.release = tbio_release,
	.media_changed = tbio_media_changed,
	.revalidate_disk = tbio_revalidate
};

static int __init tbio_init(void)
{
	tbio_dev.size = nsectors * 512;

	tbio_dev.data = vmalloc(tbio_dev.size);
	if (tbio_dev.data == NULL)
		return -ENOMEM;
	strcpy(tbio_dev.data, "tbio data");
	tbio_dev.bdev = NULL;

	TBIO_MAJOR = register_blkdev(0, DEVICE_NAME);
	if (TBIO_MAJOR <= 0) {
		prk_err("unable to get major number");
		goto out;
	}
	prk_info("register_blkdev major %d", TBIO_MAJOR);

	spin_lock_init(&tbio_dev.lock);
	tbio_dev.q = blk_init_queue(tbio_request, &tbio_dev.lock);
	if (!tbio_dev.q) {
		prk_err("failed to init queue");
		goto out_unregister;
	}

	tbio_dev.gd = alloc_disk(1);
	if (!tbio_dev.gd)
		goto out_unregister;
	tbio_dev.gd->major	= TBIO_MAJOR;
	tbio_dev.gd->first_minor	= 0;
	tbio_dev.gd->fops		= &tbio_ops;
	tbio_dev.gd->private_data	= &tbio_dev;
	tbio_dev.gd->queue	= tbio_dev.q;
	strcpy(tbio_dev.gd->disk_name, "tbio");
	set_capacity(tbio_dev.gd, nsectors);
	tbio_dev.gd->queue->queuedata = tbio_dev.gd;

	add_disk(tbio_dev.gd);

	return 0;

out_unregister:
	unregister_blkdev(TBIO_MAJOR, DEVICE_NAME);
out:
	vfree(tbio_dev.data);
	return -ENOMEM;
}
module_init(tbio_init);

static void tbio_exit(void)
{
	del_gendisk(tbio_dev.gd);
	blk_cleanup_queue(tbio_dev.q);
	put_disk(tbio_dev.gd);
	unregister_blkdev(TBIO_MAJOR, DEVICE_NAME);
	vfree(tbio_dev.data);
	prk_info("exit");
}
module_exit(tbio_exit);
