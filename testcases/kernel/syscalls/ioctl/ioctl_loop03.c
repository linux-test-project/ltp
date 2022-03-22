// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Copyright (c) Linux Test Project, 2022
 * Author: Yang Xu <xuyang2018.jy@cn.jujitsu.com>
 */

/*\
 * [Description]
 *
 * Tests ioctl() on loopdevice with LOOP_CHANGE_FD flag.
 *
 * Tests whether LOOP_CHANGE_FD can not succeed (get EINVAL error)
 * when loop_dev is not read only.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "lapi/loop.h"
#include "tst_test.h"

static char dev_path[1024];
static int dev_num, dev_fd, file_fd, attach_flag;

static void verify_ioctl_loop(void)
{
	TEST(ioctl(dev_fd, LOOP_CHANGE_FD, file_fd));
	if (TST_RET == 0) {
		tst_res(TFAIL, "LOOP_CHANGE_FD succeeded unexpectedly");
		return;
	}
	if (TST_ERR == EINVAL)
		tst_res(TPASS | TTERRNO, "LOOP_CHANGE_FD failed as expected");
	else
		tst_res(TFAIL | TTERRNO, "LOOP_CHANGE_FD failed expected EINVAL got");
}

static void setup(void)
{
	struct loop_info loopinfoget;

	memset(&loopinfoget, 0, sizeof(loopinfoget));
	dev_num = tst_find_free_loopdev(dev_path, sizeof(dev_path));
	if (dev_num < 0)
		tst_brk(TBROK, "Failed to find free loop device");

	tst_fill_file("test.img", 0, 1024, 10);
	tst_attach_device(dev_path, "test.img");
	attach_flag = 1;

	dev_fd = SAFE_OPEN(dev_path, O_RDWR);
	file_fd = SAFE_OPEN("test.img", O_RDWR);
	SAFE_IOCTL(dev_fd, LOOP_GET_STATUS, &loopinfoget);

	if (loopinfoget.lo_flags & LO_FLAGS_READ_ONLY)
		tst_brk(TCONF, "Current environment has unexpected LO_FLAGS_READ_ONLY flag");
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
	.test_all = verify_ioctl_loop,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.needs_drivers = (const char *const []) {
		"loop",
		NULL
	}
};
