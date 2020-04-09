// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.jujitsu.com>
 *
 * This is a basic ioctl error test about loopdevice
 * LOOP_SET_BLOCK_SIZE.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include "lapi/loop.h"
#include "tst_test.h"

static char dev_path[1024];
static int dev_num, dev_fd, attach_flag;
static unsigned int invalid_value, half_value, unalign_value;

static struct tcase {
	unsigned int *setvalue;
	int exp_err;
	char *message;
} tcases[] = {
	{&half_value, EINVAL, "arg < 512"},
	{&invalid_value, EINVAL, "arg > PAGE_SIZE"},
	{&unalign_value, EINVAL, "arg != power_of_2"},
};

static void verify_ioctl_loop(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	tst_res(TINFO, "%s", tc->message);
	TEST(ioctl(dev_fd, LOOP_SET_BLOCK_SIZE, *(tc->setvalue)));
	if (TST_RET == 0) {
		tst_res(TFAIL, "LOOP_SET_BLOCK_SIZE succeed unexpectedly");
		return;
	}

	if (TST_ERR == tc->exp_err) {
		tst_res(TPASS | TTERRNO, "LOOP_SET_BLOCK_SIZE failed as expected");
	} else {
		tst_res(TFAIL | TTERRNO, "LOOP_SET_BLOCK_SIZE failed expected %s got",
				tst_strerrno(tc->exp_err));
	}
}

static void setup(void)
{
	unsigned int pg_size;

	dev_num = tst_find_free_loopdev(dev_path, sizeof(dev_path));
	if (dev_num < 0)
		tst_brk(TBROK, "Failed to find free loop device");

	tst_fill_file("test.img", 0, 1024, 1024);
	tst_attach_device(dev_path, "test.img");
	attach_flag = 1;
	half_value = 256;
	pg_size = getpagesize();
	invalid_value = pg_size * 2 ;
	unalign_value = pg_size - 1;

	dev_fd = SAFE_OPEN(dev_path, O_RDWR);

	if (ioctl(dev_fd, LOOP_SET_BLOCK_SIZE, 512) && errno == EINVAL)
		tst_brk(TCONF, "LOOP_SET_BLOCK_SIZE is not supported");
}

static void cleanup(void)
{
	if (dev_fd > 0)
		SAFE_CLOSE(dev_fd);
	if (attach_flag)
		tst_detach_device(dev_path);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_ioctl_loop,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.needs_tmpdir = 1,
	.needs_drivers = (const char *const []) {
		"loop",
		NULL
	}
};
