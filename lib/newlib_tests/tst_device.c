// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Linux Test Project
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */
#define _GNU_SOURCE

#include <stdlib.h>
#include <sys/mount.h>
#include <stdint.h>
#include <stdio.h>
#include <lapi/loop.h>
#include <time.h>

#include "tst_test.h"

#define DEVBLOCKSIZE 2048
#define DEV_MIN_SIZE 310

static char *mntpoint;
static uint64_t ltp_dev_size;

static int set_block_size(int fd)
{
	return ioctl(fd, LOOP_SET_BLOCK_SIZE, DEVBLOCKSIZE);
}

static void setup(void)
{
	int fd;
	int ret;

	ret = asprintf(&mntpoint, "%s/mnt", tst_get_tmpdir());
	if (ret < 0)
		tst_brk(TBROK, "asprintf failure");

	fd = SAFE_OPEN(tst_device->dev, O_RDONLY);

	SAFE_IOCTL(fd, BLKGETSIZE64, &ltp_dev_size);

	TST_RETRY_FN_EXP_BACKOFF(set_block_size(fd), TST_RETVAL_EQ0, 10);

	SAFE_CLOSE(fd);

	SAFE_MKFS(tst_device->dev, tst_device->fs_type, NULL, NULL);

	SAFE_MKDIR(mntpoint, 0777);
	SAFE_MOUNT(tst_device->dev, mntpoint, tst_device->fs_type, 0, 0);
}

static void cleanup(void)
{
	if (tst_is_mounted(mntpoint))
		SAFE_UMOUNT(mntpoint);
}

static void test_dev_min_size(void)
{
	uint64_t size;

	size = ltp_dev_size / 1024 / 1024;

	if (size == DEV_MIN_SIZE)
		tst_res(TPASS, "Got expected device size %lu", size);
	else
		tst_res(TFAIL, "Expected device size is %d but got %lu",
			DEV_MIN_SIZE, size);
}

static void test_tst_find_backing_dev(void)
{
	char block_dev[100];

	tst_find_backing_dev(mntpoint, block_dev);

	if (!strcmp(tst_device->dev, block_dev))
		tst_res(TPASS, "%s belongs to %s block dev", mntpoint,
			block_dev);
	else
		tst_res(TFAIL, "%s should belong to %s, but %s is returned",
			mntpoint, tst_device->dev, block_dev);
}

static void test_tst_dev_block_size(void)
{
	int block_size;

	block_size = tst_dev_block_size(mntpoint);

	if (block_size == DEVBLOCKSIZE)
		tst_res(TPASS, "%s has %d block size", mntpoint, block_size);
	else
		tst_res(TFAIL, "%s has %d block size, but expected is %i",
			mntpoint, block_size, DEVBLOCKSIZE);
}

static void do_test(void)
{
	test_dev_min_size();
	test_tst_find_backing_dev();
	test_tst_dev_block_size();
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_device = 1,
	.dev_min_size = DEV_MIN_SIZE,
	.test_all = do_test,
	.setup = setup,
	.cleanup = cleanup,
	.min_kver = "4.14",
};
