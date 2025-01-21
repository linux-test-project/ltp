// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.jujitsu.com>
 */

/*\
 * [Description]
 *
 * Basic test for the BLKRRPART ioctl, it is the same as blockdev
 * --rereadpt command.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/mount.h>
#include <stdbool.h>
#include "lapi/loop.h"
#include "tst_test.h"

#define RETVAL_CHECK(x) \
       ({ value ? TST_RETVAL_EQ0(x) : TST_RETVAL_NOTNULL(x); })

static char dev_path[1024];
static int dev_num, attach_flag, dev_fd;
static char loop_partpath[1026], sys_loop_partpath[1026];

static void change_partition(const char *const cmd[])
{
	int ret;

	ret = tst_cmd(cmd, NULL, NULL, TST_CMD_PASS_RETVAL);
	if (ret)
		tst_brk(TBROK, "parted return %i", ret);
}

static void check_partition(int part_num, bool value)
{
	int ret;

	sprintf(sys_loop_partpath, "/sys/block/loop%d/loop%dp%d",
		dev_num, dev_num, part_num);
	sprintf(loop_partpath, "%sp%d", dev_path, part_num);

	ret = TST_RETRY_FN_EXP_BACKOFF(access(sys_loop_partpath, F_OK), RETVAL_CHECK, 30);
	if (ret == 0)
		tst_res(value ? TPASS : TFAIL, "access %s succeeds",
			sys_loop_partpath);
	else
		tst_res(value ? TFAIL : TPASS, "access %s fails",
			sys_loop_partpath);

	ret = TST_RETRY_FN_EXP_BACKOFF(access(loop_partpath, F_OK), RETVAL_CHECK, 30);
	if (ret == 0)
		tst_res(value ? TPASS : TFAIL, "access %s succeeds",
			loop_partpath);
	else
		tst_res(value ? TFAIL : TPASS, "access %s fails",
			loop_partpath);
}

static void verify_ioctl(void)
{
	const char *const cmd_parted_old[] = {"parted", "-s", "test.img",
					      "mklabel", "msdos", "mkpart",
					      "primary", "ext4", "1M", "10M",
					      NULL};
	const char *const cmd_parted_new[] = {"parted", "-s", "test.img",
					      "mklabel", "msdos", "mkpart",
					      "primary", "ext4", "1M", "10M",
					      "mkpart", "primary", "ext4",
					      "10M", "20M", NULL};
	struct loop_info loopinfo = {0};

	change_partition(cmd_parted_old);
	tst_attach_device(dev_path, "test.img");
	attach_flag = 1;

	loopinfo.lo_flags =  LO_FLAGS_PARTSCAN;
	SAFE_IOCTL(dev_fd, LOOP_SET_STATUS, &loopinfo);
	check_partition(1, true);
	check_partition(2, false);

	change_partition(cmd_parted_new);
	TST_RETRY_FUNC(ioctl(dev_fd, BLKRRPART, 0), TST_RETVAL_EQ0);
	check_partition(1, true);
	check_partition(2, true);

	tst_detach_device_by_fd(dev_path, dev_fd);
	dev_fd = SAFE_OPEN(dev_path, O_RDWR);
	attach_flag = 0;
}

static void setup(void)
{
	dev_num = tst_find_free_loopdev(dev_path, sizeof(dev_path));
	if (dev_num < 0)
		tst_brk(TBROK, "Failed to find free loop device");
	tst_prealloc_file("test.img", 1024 * 1024, 20);
	dev_fd = SAFE_OPEN(dev_path, O_RDWR);
}

static void cleanup(void)
{
	if (dev_fd > 0)
		SAFE_CLOSE(dev_fd);
	if (attach_flag)
		tst_detach_device(dev_path);
}

static struct tst_test test = {
	.timeout = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_ioctl,
	.needs_root = 1,
	.needs_drivers = (const char *const []) {
		"loop",
		NULL
	},
	.needs_cmds = (const char *const []) {
		"parted",
		NULL
	},
	.needs_tmpdir = 1,
};
