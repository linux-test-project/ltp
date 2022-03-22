// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Copyright (c) Linux Test Project, 2020-2022
 * Author: Yang Xu <xuyang2018.jy@cn.jujitsu.com>
 */

/*\
 * [Description]
 *
 * Tests ioctl() on loopdevice with LOOP_SET_DIRECT_IO flag.
 *
 * Tests whether LOOP_SET_DIRECT_IO can update a live loop device dio mode.
 * It requires the backing file also supports dio mode and the lo_offset is
 * aligned with the logical block size.
 *
 * The direct I/O error handling is a bit messy on Linux, some filesystems
 * return error when it coudln't be enabled, some silently fall back to regular
 * buffered I/O.
 *
 * The LOOP_SET_DIRECT_IO ioctl() may ignore all checks if it cannot get the
 * logical block size which is the case if the block device pointer in the
 * backing file inode is not set. In this case the direct I/O appears to be
 * enabled but falls back to buffered I/O later on. This is the case at least
 * for Btrfs. Because of that the test passes both with failure as well as
 * success with non-zero offset.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mount.h>
#include "lapi/loop.h"
#include "tst_test.h"

#define DIO_MESSAGE "In dio mode"
#define NON_DIO_MESSAGE "In non dio mode"

static char dev_path[1024], sys_loop_diopath[1024], backing_file_path[1024];;
static int dev_num, dev_fd, block_devfd, attach_flag, logical_block_size;

static void check_dio_value(int flag)
{
	struct loop_info loopinfoget;

	memset(&loopinfoget, 0, sizeof(loopinfoget));

	SAFE_IOCTL(dev_fd, LOOP_GET_STATUS, &loopinfoget);
	tst_res(TINFO, "%s", flag ? DIO_MESSAGE : NON_DIO_MESSAGE);

	if (loopinfoget.lo_flags & LO_FLAGS_DIRECT_IO)
		tst_res(flag ? TPASS : TFAIL, "lo_flags has LO_FLAGS_DIRECT_IO flag");
	else
		tst_res(flag ? TFAIL : TPASS, "lo_flags doesn't have LO_FLAGS_DIRECT_IO flag");

	TST_ASSERT_INT(sys_loop_diopath, flag);
}

static void verify_ioctl_loop(void)
{
	struct loop_info loopinfo;

	memset(&loopinfo, 0, sizeof(loopinfo));
	TST_RETRY_FUNC(ioctl(dev_fd, LOOP_SET_STATUS, &loopinfo), TST_RETVAL_EQ0);

	tst_res(TINFO, "Without setting lo_offset or sizelimit");
	SAFE_IOCTL(dev_fd, LOOP_SET_DIRECT_IO, 1);
	check_dio_value(1);

	SAFE_IOCTL(dev_fd, LOOP_SET_DIRECT_IO, 0);
	check_dio_value(0);

	tst_res(TINFO, "With offset equal to logical_block_size");
	loopinfo.lo_offset = logical_block_size;
	TST_RETRY_FUNC(ioctl(dev_fd, LOOP_SET_STATUS, &loopinfo), TST_RETVAL_EQ0);
	TEST(ioctl(dev_fd, LOOP_SET_DIRECT_IO, 1));
	if (TST_RET == 0) {
		tst_res(TPASS, "LOOP_SET_DIRECT_IO succeeded");
		check_dio_value(1);
		SAFE_IOCTL(dev_fd, LOOP_SET_DIRECT_IO, 0);
	} else {
		tst_res(TFAIL | TTERRNO, "LOOP_SET_DIRECT_IO failed");
	}

	tst_res(TINFO, "With nonzero offset less than logical_block_size");
	loopinfo.lo_offset = logical_block_size / 2;
	TST_RETRY_FUNC(ioctl(dev_fd, LOOP_SET_STATUS, &loopinfo), TST_RETVAL_EQ0);

	TEST(ioctl(dev_fd, LOOP_SET_DIRECT_IO, 1));
	if (TST_RET == 0) {
		tst_res(TPASS, "LOOP_SET_DIRECT_IO succeeded, offset is ignored");
		SAFE_IOCTL(dev_fd, LOOP_SET_DIRECT_IO, 0);
		return;
	}
	if (TST_ERR == EINVAL)
		tst_res(TPASS | TTERRNO, "LOOP_SET_DIRECT_IO failed as expected");
	else
		tst_res(TFAIL | TTERRNO, "LOOP_SET_DIRECT_IO failed expected EINVAL got");
}

static void setup(void)
{
	char bd_path[100];

	dev_num = tst_find_free_loopdev(dev_path, sizeof(dev_path));
	if (dev_num < 0)
		tst_brk(TBROK, "Failed to find free loop device");

	sprintf(sys_loop_diopath, "/sys/block/loop%d/loop/dio", dev_num);
	tst_fill_file("test.img", 0, 1024, 1024);

	tst_attach_device(dev_path, "test.img");
	attach_flag = 1;
	dev_fd = SAFE_OPEN(dev_path, O_RDWR);

	if (ioctl(dev_fd, LOOP_SET_DIRECT_IO, 0) && errno == EINVAL)
		tst_brk(TCONF, "LOOP_SET_DIRECT_IO is not supported");

	/*
	 * from __loop_update_dio():
	 *   We support direct I/O only if lo_offset is aligned with the
	 *   logical I/O size of backing device, and the logical block
	 *   size of loop is bigger than the backing device's and the loop
	 *   needn't transform transfer.
	 */
	sprintf(backing_file_path, "%s/test.img", tst_get_tmpdir());
	tst_find_backing_dev(backing_file_path, bd_path);
	block_devfd = SAFE_OPEN(bd_path, O_RDWR);
	SAFE_IOCTL(block_devfd, BLKSSZGET, &logical_block_size);
	tst_res(TINFO, "backing dev(%s) logical_block_size is %d", bd_path, logical_block_size);
	SAFE_CLOSE(block_devfd);
	if (logical_block_size > 512)
		TST_RETRY_FUNC(ioctl(dev_fd, LOOP_SET_BLOCK_SIZE, logical_block_size), TST_RETVAL_EQ0);
}

static void cleanup(void)
{
	if (dev_fd > 0)
		SAFE_CLOSE(dev_fd);
	if (block_devfd > 0)
		SAFE_CLOSE(block_devfd);
	if (attach_flag)
		tst_detach_device(dev_path);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_ioctl_loop,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.skip_filesystems = (const char *const []) {
		"tmpfs",
		"overlayfs",
		NULL
	},
	.needs_drivers = (const char *const []) {
		"loop",
		NULL
	}
};
