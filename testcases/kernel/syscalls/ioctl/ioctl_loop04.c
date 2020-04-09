// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.jujitsu.com>
 *
 * This is a basic ioctl test about loopdevice.
 *
 * It is designed to test LOOP_SET_CAPACITY can update a live
 * loop device size when we change the size of the underlying
 * backing file. Also check sys value.
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "lapi/loop.h"
#include "tst_test.h"

#define OLD_SIZE 10240
#define NEW_SIZE 5120

static char dev_path[1024], sys_loop_sizepath[1024];
static char *wrbuf;
static int dev_num, dev_fd, file_fd, attach_flag;

static void verify_ioctl_loop(void)
{
	struct loop_info loopinfoget;

	memset(&loopinfoget, 0, sizeof(loopinfoget));
	tst_fill_file("test.img", 0, 1024, OLD_SIZE/1024);
	tst_attach_device(dev_path, "test.img");
	attach_flag = 1;

	TST_ASSERT_INT(sys_loop_sizepath, OLD_SIZE/512);
	dev_fd = SAFE_OPEN(dev_path, O_RDWR);
	file_fd = SAFE_OPEN("test.img", O_RDWR);
	SAFE_IOCTL(dev_fd, LOOP_GET_STATUS, &loopinfoget);

	if (loopinfoget.lo_flags & LO_FLAGS_READ_ONLY)
		tst_brk(TCONF, "Current environment has unexpected LO_FLAGS_READ_ONLY flag");

	SAFE_TRUNCATE("test.img", NEW_SIZE);
	SAFE_IOCTL(dev_fd, LOOP_SET_CAPACITY);

	/*check that we can't write data beyond 5K into loop device*/
	TEST(write(dev_fd, wrbuf, OLD_SIZE));
	if (TST_RET == NEW_SIZE) {
		tst_res(TPASS, "LOOP_SET_CAPACITY set loop size to %d", NEW_SIZE);
	} else {
		tst_res(TFAIL, "LOOP_SET_CAPACITY didn't set loop size to %d, its size is %ld",
				NEW_SIZE, TST_RET);
	}

	TST_ASSERT_INT(sys_loop_sizepath, NEW_SIZE/512);

	SAFE_CLOSE(file_fd);
	SAFE_CLOSE(dev_fd);
	tst_detach_device(dev_path);
	unlink("test.img");
	attach_flag = 0;
}

static void setup(void)
{
	dev_num = tst_find_free_loopdev(dev_path, sizeof(dev_path));
	if (dev_num < 0)
		tst_brk(TBROK, "Failed to find free loop device");

	wrbuf = SAFE_MALLOC(OLD_SIZE);
	memset(wrbuf, 'x', OLD_SIZE);
	sprintf(sys_loop_sizepath, "/sys/block/loop%d/size", dev_num);
}

static void cleanup(void)
{
	if (dev_fd > 0)
		SAFE_CLOSE(dev_fd);
	if (file_fd > 0)
		SAFE_CLOSE(file_fd);
	if (wrbuf)
		free(wrbuf);
	if (attach_flag)
		tst_detach_device(dev_path);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_ioctl_loop,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.needs_drivers = (const char *const []) {
		"loop",
		NULL
	}
};
