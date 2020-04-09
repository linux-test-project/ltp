// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.jujitsu.com>
 *
 * This is a basic ioctl test about loopdevice LOOP_SET_STATUS64
 * and LOOP_GET_STATUS64.
 * Test its lo_sizelimit field. If lo_sizelimit is 0,it means max
 * available. If sizelimit is less than loop_size, loopsize will
 * be truncated.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include "lapi/loop.h"
#include "tst_test.h"

static char dev_path[1024], sys_loop_sizepath[1024], sys_loop_sizelimitpath[1024];
static int dev_num, dev_fd, file_fd, attach_flag;

static struct tcase {
	unsigned int set_sizelimit;
	unsigned int exp_loopsize;
	char *message;
} tcases[] = {
	{1024 * 4096, 2048, "When sizelimit is greater than loopsize "},
	{1024 * 512, 1024, "When sizelimit is less than loopsize"},
};

static void verify_ioctl_loop(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	struct loop_info64 loopinfo, loopinfoget;

	tst_res(TINFO, "%s", tc->message);
	memset(&loopinfo, 0, sizeof(loopinfo));
	memset(&loopinfoget, 0, sizeof(loopinfoget));

	loopinfo.lo_sizelimit = tc->set_sizelimit;
	TST_RETRY_FUNC(ioctl(dev_fd, LOOP_SET_STATUS64, &loopinfo), TST_RETVAL_EQ0);

	TST_ASSERT_INT(sys_loop_sizepath, tc->exp_loopsize);
	TST_ASSERT_INT(sys_loop_sizelimitpath, tc->set_sizelimit);
	SAFE_IOCTL(dev_fd, LOOP_GET_STATUS64, &loopinfoget);
	if (loopinfoget.lo_sizelimit == tc->set_sizelimit)
		tst_res(TPASS, "LOOP_GET_STATUS64 gets correct lo_sizelimit(%d)", tc->set_sizelimit);
	else
		tst_res(TFAIL, "LOOP_GET_STATUS64 gets wrong lo_sizelimit(%llu), expect %d",
				loopinfoget.lo_sizelimit, tc->set_sizelimit);
	/*Reset*/
	loopinfo.lo_sizelimit = 0;
	TST_RETRY_FUNC(ioctl(dev_fd, LOOP_SET_STATUS, &loopinfo), TST_RETVAL_EQ0);
}

static void setup(void)
{
	dev_num = tst_find_free_loopdev(dev_path, sizeof(dev_path));
	if (dev_num < 0)
		tst_brk(TBROK, "Failed to find free loop device");

	tst_fill_file("test.img", 0, 1024 * 1024, 1);
	tst_attach_device(dev_path, "test.img");
	attach_flag = 1;

	sprintf(sys_loop_sizepath, "/sys/block/loop%d/size", dev_num);
	sprintf(sys_loop_sizelimitpath, "/sys/block/loop%d/loop/sizelimit", dev_num);

	dev_fd = SAFE_OPEN(dev_path, O_RDWR);
	tst_res(TINFO, "original loop size 2048 sectors");
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
	.test = verify_ioctl_loop,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.needs_tmpdir = 1,
	.needs_drivers = (const char *const []) {
		"loop",
		NULL
	}
};
