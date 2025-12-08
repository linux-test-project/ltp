// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Copyright (c) Linux Test Project, 2022
 * Author: Yang Xu <xuyang2018.jy@cn.jujitsu.com>
 */

/*\
 * Tests invalid block size of loopdevice by using :man2:`ioctl` with
 * LOOP_SET_BLOCK_SIZE and LOOP_CONFIGURE flags.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include "lapi/blkdev.h"
#include "lapi/loop.h"
#include "tst_fs.h"
#include "tst_test.h"

static char dev_path[1024];
static int dev_num, dev_fd, file_fd, attach_flag, loop_configure_sup = 1;
static unsigned int invalid_value, half_value, unalign_value;
static struct loop_config loopconfig;

static struct tcase {
	unsigned int *setvalue;
	int ioctl_flag;
	char *message;
} tcases[] = {
	{&half_value, LOOP_SET_BLOCK_SIZE,
	"Using LOOP_SET_BLOCK_SIZE with arg < 512"},

	{&invalid_value, LOOP_SET_BLOCK_SIZE,
	"Using LOOP_SET_BLOCK_SIZE with arg > BLK_MAX_BLOCK_SIZE"},

	{&unalign_value, LOOP_SET_BLOCK_SIZE,
	"Using LOOP_SET_BLOCK_SIZE with arg != power_of_2"},

	{&half_value, LOOP_CONFIGURE,
	"Using LOOP_CONFIGURE with block_size < 512"},

	{&invalid_value, LOOP_CONFIGURE,
	"Using LOOP_CONFIGURE with block_size > BLK_MAX_BLOCK_SIZE"},

	{&unalign_value, LOOP_CONFIGURE,
	"Using LOOP_CONFIGURE with block_size != power_of_2"},
};

static void verify_ioctl_loop(unsigned int n)
{
	if (tcases[n].ioctl_flag == LOOP_CONFIGURE)
		TEST(ioctl(dev_fd, LOOP_CONFIGURE, &loopconfig));
	else
		TEST(ioctl(dev_fd, LOOP_SET_BLOCK_SIZE, *(tcases[n].setvalue)));

	if (TST_RET == 0) {
		tst_res(TFAIL, "Set block size succeed unexpectedly");
		if (tcases[n].ioctl_flag == LOOP_CONFIGURE) {
			tst_detach_device_by_fd(dev_path, &dev_fd);
			dev_fd = SAFE_OPEN(dev_path, O_RDWR);
		}
		return;
	}
	if (TST_ERR == EINVAL)
		tst_res(TPASS | TTERRNO, "Set block size failed as expected");
	else
		tst_res(TFAIL | TTERRNO, "Set block size failed expected EINVAL got");
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	tst_res(TINFO, "%s", tc->message);
	if (tc->ioctl_flag == LOOP_SET_BLOCK_SIZE) {
		if (!attach_flag) {
			tst_attach_device(dev_path, "test.img");
			attach_flag = 1;
		}
		verify_ioctl_loop(n);
		return;
	}

	if (tc->ioctl_flag == LOOP_CONFIGURE && !loop_configure_sup) {
		tst_res(TCONF, "LOOP_CONFIGURE ioctl not supported");
		return;
	}
	if (attach_flag) {
		tst_detach_device_by_fd(dev_path, &dev_fd);
		dev_fd = SAFE_OPEN(dev_path, O_RDWR);
		attach_flag = 0;
	}
	loopconfig.block_size = *(tc->setvalue);
	verify_ioctl_loop(n);
}

static void setup(void)
{
	unsigned int pg_size;
	int ret;

	dev_num = tst_find_free_loopdev(dev_path, sizeof(dev_path));
	if (dev_num < 0)
		tst_brk(TBROK, "Failed to find free loop device");

	size_t bs = (BLK_MAX_BLOCK_SIZE < TST_MB) ? 1024 : 4 * BLK_MAX_BLOCK_SIZE / 1024;
	tst_fill_file("test.img", 0, bs, 1024);

	half_value = 256;
	pg_size = getpagesize();
	invalid_value = BLK_MAX_BLOCK_SIZE * 2;
	unalign_value = pg_size - 1;

	dev_fd = SAFE_OPEN(dev_path, O_RDWR);
	ret = ioctl(dev_fd, LOOP_SET_BLOCK_SIZE, 512);

	if (ret && (errno == EINVAL || errno == ENOTTY))
		tst_brk(TCONF, "LOOP_SET_BLOCK_SIZE is not supported");

	file_fd = SAFE_OPEN("test.img", O_RDWR);
	loopconfig.fd = -1;
	ret = ioctl(dev_fd, LOOP_CONFIGURE, &loopconfig);
	if (ret && errno != EBADF) {
		tst_res(TINFO | TERRNO, "LOOP_CONFIGURE is not supported");
		loop_configure_sup = 0;
		return;
	}
	loopconfig.fd = file_fd;
}

static void cleanup(void)
{
	if (dev_fd > 0)
		SAFE_CLOSE(dev_fd);
	if (file_fd > 0)
		SAFE_CLOSE(file_fd);
	if (attach_flag)
		tst_detach_device(dev_path);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.needs_tmpdir = 1,
	.needs_drivers = (const char *const []) {
		"loop",
		NULL
	}
};
