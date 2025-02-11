// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * It is a basic test for STATX_DIOALIGN mask on block device.
 *
 * - STATX_DIOALIGN   Want stx_dio_mem_align and stx_dio_offset_align value
 *
 * These two values are tightly coupled to the kernel's current DIO
 * restrictions on block devices.
 *
 * Minimum Linux version required is v6.1.
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/mount.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "tst_test.h"
#include "lapi/stat.h"

static char sys_bdev_dma_path[1024], sys_bdev_logical_path[1024];

static void verify_statx(void)
{
	struct statx buf;

	TST_EXP_PASS_SILENT(statx(AT_FDCWD, tst_device->dev, 0, STATX_DIOALIGN, &buf),
		"statx(AT_FDCWD, %s, 0, STATX_DIOALIGN, &buf)", tst_device->dev);

	if (!(buf.stx_mask & STATX_DIOALIGN)) {
		tst_res(TCONF, "Filesystem does not support STATX_DIOALIGN");
		return;
	}

#ifdef HAVE_STRUCT_STATX_STX_DIO_MEM_ALIGN
	/*
	 * This test is tightly coupled to the kernel's current DIO restrictions
	 * on block devices. The general rule of DIO needing to be aligned to the
	 * block device's logical block size was relaxed to allow user buffers
	 * (but not file offsets) aligned to the DMA alignment instead. See v6.0
	 * commit bf8d08532bc1 ("iomap: add support for dma aligned direct-io") and
	 * they are subject to further change in the future.
	 * Also can see commit 2d985f8c6b9 ("vfs: support STATX_DIOALIGN on block devices).
	 */
	TST_ASSERT_ULONG(sys_bdev_dma_path, buf.stx_dio_mem_align - 1);
	TST_ASSERT_ULONG(sys_bdev_logical_path, buf.stx_dio_offset_align);
#else
	tst_res(TCONF, "glibc statx struct miss stx_dio_mem_align field");
#endif
}

static void setup(void)
{
	char full_name[256];
	char *dev_name;

	strcpy(full_name, tst_device->dev);
	dev_name = SAFE_BASENAME(full_name);
	sprintf(sys_bdev_logical_path, "/sys/block/%s/queue/logical_block_size", dev_name);

	/*
	 * Since /sys/block/%s/queue doesn't exist for partition, we need to
	 * use a while to search block device instead of partition.
	 */
	while (access(sys_bdev_logical_path, F_OK) != 0) {
		dev_name[strlen(dev_name)-1] = '\0';
		sprintf(sys_bdev_logical_path, "/sys/block/%s/queue/logical_block_size", dev_name);
	}

	sprintf(sys_bdev_dma_path, "/sys/block/%s/queue/dma_alignment", dev_name);
	if (access(sys_bdev_dma_path, F_OK) != 0)
		tst_brk(TCONF, "dma_alignment sysfs file doesn't exist");
}

static struct tst_test test = {
	.test_all = verify_statx,
	.setup = setup,
	.needs_device = 1,
	.needs_root = 1,
};
