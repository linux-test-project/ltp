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
 *

 * This module will test block io layer.
 *
 * module: tbio
 *  Copyright (c) International Business Machines  Corp., 2003
 *
 *  This program is free software;  you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *  the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; 
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


#ifndef __KERNEL__
#define __KERNEL__
#endif

#include <linux/module.h>
//#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include "tbio.h"

#define BLK_DEFAULT_TIMEOUT	(60 * HZ)
MODULE_AUTHOR("Kai Zhao <ltcd3@cn.ibm.com>");
MODULE_DESCRIPTION(TMOD_DRIVER_NAME);
MODULE_LICENSE("GPL");

//module_param(major_num , int , 0);
static int hardsect_size = 1024;
//module_param(hardsect_size , int , 0);
static int nsectors = 1024;
//module_param(nsectors , int , 0);

static struct bio *tbiop = NULL , *tbiop_dup = NULL;
//static struct bio_pair *bio_pairp = NULL;
static struct request_queue Queue;

static struct tbio_device {
	unsigned long size ;
	spinlock_t lock;
	u8 *data;
	struct gendisk *gd;
	struct block_device *bdev;
} Device;

static int send_request(request_queue_t *q, struct bio *bio ,struct block_device *bdev,
			 struct tbio_interface  *inter , int writing)
{
	struct request *rq;
	void * buffer = NULL;
	unsigned long start_time;
	int err;

	rq = blk_get_request(q, writing ? WRITE : READ, __GFP_WAIT);
	rq->cmd_len = inter->cmd_len;
	//printk("inter.cmd %s\n" , inter->cmd);
	if (copy_from_user(rq->cmd, inter->cmd, inter->cmd_len))
		goto out_request;
	//printk("tbio: rq->cmd : %s\n",rq->cmd);
	if (sizeof(rq->cmd) != inter->cmd_len)
		memset(rq->cmd + inter->cmd_len, 0, sizeof(rq->cmd) - inter->cmd_len);

	rq->bio = rq->biotail = NULL;

	blk_rq_bio_prep(q , rq , bio);

	rq->data = buffer;
	rq->data_len = inter->data_len;

	rq->timeout = 0;
	if (!rq->timeout)
		rq->timeout = q->sg_timeout;
	if (!rq->timeout)
		rq->timeout = BLK_DEFAULT_TIMEOUT;

	start_time = jiffies;

	DECLARE_COMPLETION(wait);

	rq->rq_disk = bdev->bd_disk;

	rq->waiting = &wait;
	elv_add_request(q, rq, 1, 1);
	generic_unplug_device(q);
	wait_for_completion(&wait);
	//printk("tbio: completion\n");
	if (rq->errors){
		err = -EIO;
		printk("tbio: rq->errors\n");
		return err;
	}

	blk_put_request(rq);

	return 0;
out_request:
	blk_put_request(rq);
	return -EFAULT;

}


static int tbio_io(struct block_device *bdev,
		 struct tbio_interface  *uptr)
{
	tbio_interface_t inter;
	struct bio *bio = NULL;
	int reading = 0 , writing = 0 ;
	void * buffer = NULL;
	//struct request *rq;
	request_queue_t *q;
	q = bdev_get_queue(Device.bdev);


	if (copy_from_user(&inter , uptr , sizeof(tbio_interface_t))) {
		printk("tbio: copy_from_user\n");
		return -EFAULT;
	}


	if (inter.data_len > (q->max_sectors << 9)) {
		printk("tbio: inter.in_len > q->max_sectors << 9\n");
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

		bio = bio_map_user(bdev , (unsigned long )inter.data ,
					inter.data_len , reading);
	
		if(!bio) {
			printk("tbio : bio_map_user failed\n");
			buffer = kmalloc (inter.data_len , q->bounce_gfp | GFP_USER);
			if(!buffer){
				printk("tbio: buffer no memory\n");
				return -1;
			}
			copy_from_user(buffer , inter.data , inter.data_len);
			printk("tbio: buffer %s\n",(char *)buffer);
		}

	}

	send_request(q, bio ,bdev,&inter , writing);

	
	if (bio)
		bio_unmap_user(bio, reading);
	return 0;
}


static int test_bio_put(struct bio *biop)
{

	if(biop)
		bio_put(biop);

	return 0;
}


static int test_bio_clone(void)
{
	tbiop_dup = bio_clone(tbiop,GFP_NOIO);
	if( tbiop_dup == NULL ) {
		printk("tbio: bio_clone failed\n");
		return -1;
	}

	test_bio_put(tbiop_dup);

	return 0;
}

static int test_bio_add_page(void)
{
	int res = 0 ,  i = 0 , offset = 0;
	unsigned long addr = 0;
	struct page *ppage = NULL;

	for (i = 0 ; i < 128 ; i ++) {
	
		addr = get_zeroed_page(GFP_KERNEL);

		if(addr == 0) {
			printk("tbio: get free page failed %ld\n" , addr);
			return -1;
		}

		ppage = virt_to_page(addr);
		if ( ppage ==  NULL ) {
			printk ("tbio: covert virture page to page struct failed\n");
			return -1;
		}

		res = bio_add_page(tbiop , ppage , PAGE_SIZE , offset );
		if (res <0 ) {
			printk("bio_add_page :res %d\n",res);
			return -1;
		}
		offset += res;
	//	printk ("tbio: bio_add_page : %d\n", res);
	}
	return 0;
}


static int test_do_bio_alloc(int num)
{
	tbiop = bio_alloc(GFP_KERNEL , num);
	if(tbiop == NULL ) {
		printk("tbio: bio_alloc failed\n");
		return -1;
	}
	bio_put(tbiop);

	return 0;
}


static int test_bio_alloc(void)
{
	int res = 0;
	res = test_do_bio_alloc(2);
	if(res < 0){
		printk("can not alloc bio for %d\n",2);
		return -1;
	}

	res = test_do_bio_alloc(8);
	if(res < 0){
		printk("can not alloc bio for %d\n",8);
		return -1;
	}
	res = test_do_bio_alloc(32);
	if(res < 0){
		printk("can not alloc bio for %d\n",32);
		return -1;
	}
	res = test_do_bio_alloc(96);
	if(res < 0){
		printk("can not alloc bio for %d\n",96);
		return -1;
	}
	res = test_do_bio_alloc(BIO_MAX_PAGES);
	if(res < 0){
		printk("can not alloc bio for %d\n",BIO_MAX_PAGES);
		return -1;
	}


	tbiop = bio_alloc(GFP_KERNEL , BIO_MAX_PAGES);
	if(tbiop == NULL ) {
		printk("tbio: bio_alloc failed\n");
		return -1;
	}

	return 0;
}




static int test_bio_split(struct block_device *bdev,
		 struct tbio_interface  *uptr)
{
	tbio_interface_t inter;
	struct bio *bio = NULL;
	struct bio_pair *bio_pairp = NULL;
	int reading = 0 , writing = 0 ;
	void * buffer = NULL;
	request_queue_t *q;
	q = bdev_get_queue(Device.bdev);

	if (copy_from_user(&inter , uptr , sizeof(tbio_interface_t))) {
		printk("tbio: copy_from_user\n");
		return -EFAULT;
	}

	if (inter.data_len > (q->max_sectors << 9)) {
		printk("tbio: inter.in_len > q->max_sectors << 9\n");
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

		bio = bio_map_user(bdev , (unsigned long )inter.data ,
					inter.data_len , reading);
	
		if(!bio) {
			printk("tbio : bio_map_user failed\n");
			buffer = kmalloc (inter.data_len , q->bounce_gfp | GFP_USER);
			if(!buffer){
				printk("tbio: buffer no memory\n");
				return -1;
			}
			copy_from_user(buffer , inter.data , inter.data_len);
			printk("tbio: buffer %s\n",(char *)buffer);
		}
		else {
	//		printk("tbio: bio sectors %d\n", bio_sectors(bio));
	//		printk("tbio: split now\n");
			bio_pairp = bio_split(bio , bio_split_pool, 2);
		
			if ( bio_pairp == NULL ) {
				printk("tbio: bio_split failed\n");
				bio_unmap_user(bio,reading);
				return -1;
			}
		}

	}

	send_request(q, &(bio_pairp->bio1) ,bdev,&inter , writing);
	q = bdev_get_queue(Device.bdev);
	send_request(q, &(bio_pairp->bio2) ,bdev,&inter , writing);

	if (bio_pairp) {
		bio_pair_release(bio_pairp);
		return 0;
	}

	if (bio)
		bio_unmap_user(bio, reading);

	return 0;

}

static int test_bio_get_nr_vecs(void)
{
	int number = 0;

	if(!tbiop) {
		printk("tbio: tbiop is NULL\n");
		return -1;
	}

	number = bio_get_nr_vecs(tbiop->bi_bdev);

	if (number <0 ) {
		printk("tbio: bio_get_nr_vec failed\n");
		return -1;
	}
	//printk("bio_get_nr_vecs: %d\n",number);
	return 0;
}

static int tbio_ioctl(struct inode *ino, struct file *file,
                        unsigned int cmd, unsigned long arg)
{
	int err;
//	request_queue_t *q;

	//q = bdev_get_queue(Device.bdev);

	printk("ttbio: ioctl 0x%x 0x%lx\n" , cmd , arg);

	switch ( cmd ) {
	  case LTP_TBIO_DO_IO:
	    {
		err = bd_claim(Device.bdev, current);
		if (err) {
			printk("tbio:bd_claim\n");
			break;
		}
	
		err = tbio_io( Device.bdev, (struct tbio_interface *) arg);
		bd_release(Device.bdev);
	    }
	    break;

	  case LTP_TBIO_CLONE:            err = test_bio_clone(); break;
	  case LTP_TBIO_ADD_PAGE:         err = test_bio_add_page(); break;
	  case LTP_TBIO_ALLOC:            err = test_bio_alloc(); break;
	  case LTP_TBIO_GET_NR_VECS:      err = test_bio_get_nr_vecs();break;
	  case LTP_TBIO_PUT:              err = test_bio_put(tbiop);break;
	  case LTP_TBIO_SPLIT:           
	    {
	    	err = bd_claim(Device.bdev, current);
		if (err) {
			printk("tbio:bd_claim\n");
			break;
		}
	
		err = test_bio_split( Device.bdev, (struct tbio_interface *) arg);
		bd_release(Device.bdev);
	   
	    }
	    break;
	  //case LTP_TBIO_PAIR_RELEASE:     err = test_bio_pair_release();break;
	
	
	}
	return 0;
}


static void tbio_transfer(struct request *req , struct tbio_device *dev )
{

	struct bio *bio = req->bio;

	//printk("tbio: bio_data(bio) %s\n" , (char *)bio_data(bio));
	if(bio_data_dir(bio)) {
		printk("tbio: write \"%s\" to dev\n" , (char *)bio_data(bio));
		memcpy(dev->data , bio_data(bio) , bio->bi_size);
	}
	else {
		memcpy(bio_data(bio) , dev->data , bio->bi_size);
		printk("tbio: read \"%s\" from dev\n" , (char *)bio_data(bio));
	}

}

static void tbio_request(request_queue_t *q)
{
	struct request *req;

	while (( req = elv_next_request(q)) != NULL) {

		tbio_transfer(req , &Device);
		end_request(req , 1);
	}
}

static int tbio_open(struct inode *inode , struct file *filep)
{
	if( ! Device.bdev ) {
		Device.bdev = inode->i_bdev;
		//atomic_inc((atomic_t)&Device.bdev->bd_part_count);
	}

	return 0;
}

static int tbio_release(struct inode *inode , struct file *filep)
{
	return 0;
}

int tbio_media_changed(struct gendisk *gd)
{
	return 0;
}

int tbio_revalidate(struct gendisk *gd)
{
	return 0;
}

static struct block_device_operations tbio_ops = {
	.owner		=THIS_MODULE,
	.open		=tbio_open,
	.ioctl		=tbio_ioctl,
	.release	=tbio_release,
	.media_changed	=tbio_media_changed,
	.revalidate_disk	=tbio_revalidate
};


static int __init tbio_init(void)
{
	Device.size = nsectors*hardsect_size ;
    int result;
	spin_lock_init(&Device.lock);
	Device.data = vmalloc(Device.size);
	if(Device.data == NULL)
		return -ENOMEM;
	Device.bdev = NULL;

	result = register_blkdev(TBIO_MAJOR , DEVICE_NAME);//, &tbio_ops);

    printk(KERN_ALERT "LTP BIO: register_blkdev result=%d major %d\n",result, TBIO_MAJOR);

	if(result <= 0) {
		printk(KERN_WARNING "tbio:unable to get major number\n");
		goto out;
	}

	Device.gd = alloc_disk(1);
	if(! Device.gd)
		goto out_unregister;
	Device.gd->major = TBIO_MAJOR;
	Device.gd->first_minor = 0;
	Device.gd->fops = &tbio_ops;
	Device.gd->private_data = &Device;
	strcpy(Device.gd->disk_name , "tbio0");
	set_capacity(Device.gd , nsectors);
	blk_init_queue(tbio_request , &Device.lock);
	Device.gd->queue = &Queue;
	add_disk(Device.gd);

	return 0;

	out_unregister:
		unregister_chrdev(TBIO_MAJOR , "tbio");
	out:
		vfree(Device.data);
		return -ENOMEM;
}

static void tbio_exit(void)
{
	if(Device.bdev) {
		invalidate_bdev(Device.bdev,1);
		bdput(Device.bdev);
	}

	del_gendisk(Device.gd);
	put_disk(Device.gd);
	unregister_blkdev(TBIO_MAJOR , "tbio");
	vfree(Device.data);
}

module_init(tbio_init);
module_exit(tbio_exit);


