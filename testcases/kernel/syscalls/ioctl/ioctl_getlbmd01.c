// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify :manpage:`ioctl(2)` with FS_IOC_GETLBMD_CAP on block devices.
 *
 * - fill struct logical_block_metadata_cap with non-zero pattern, call
 *   FS_IOC_GETLBMD_CAP on a block device without integrity support
 *   and verify the kernel zeroed out all fields
 * - call FS_IOC_GETLBMD_CAP on a regular file and verify it fails
 *   with ENOTTY
 */

#include <sys/ioctl.h>
#include "tst_test.h"
#include "lapi/fs.h"

static int dev_fd = -1;
static int file_fd = -1;

static struct logical_block_metadata_cap *meta_cap;

static void run(void)
{
	memset(meta_cap, 0xff, sizeof(*meta_cap));

	TST_EXP_PASS(ioctl(dev_fd, FS_IOC_GETLBMD_CAP, meta_cap),
		"FS_IOC_GETLBMD_CAP on block device");

	if (!TST_PASS)
		return;

	TST_EXP_EQ_LU(meta_cap->lbmd_flags, 0);
	TST_EXP_EQ_LU(meta_cap->lbmd_interval, 0);
	TST_EXP_EQ_LU(meta_cap->lbmd_size, 0);
	TST_EXP_EQ_LU(meta_cap->lbmd_opaque_size, 0);
	TST_EXP_EQ_LU(meta_cap->lbmd_opaque_offset, 0);
	TST_EXP_EQ_LU(meta_cap->lbmd_pi_size, 0);
	TST_EXP_EQ_LU(meta_cap->lbmd_pi_offset, 0);
	TST_EXP_EQ_LU(meta_cap->lbmd_guard_tag_type, 0);
	TST_EXP_EQ_LU(meta_cap->lbmd_app_tag_size, 0);
	TST_EXP_EQ_LU(meta_cap->lbmd_ref_tag_size, 0);
	TST_EXP_EQ_LU(meta_cap->lbmd_storage_tag_size, 0);

	TST_EXP_FAIL(ioctl(file_fd, FS_IOC_GETLBMD_CAP, meta_cap), ENOTTY,
		"FS_IOC_GETLBMD_CAP on regular file");
}

static void setup(void)
{
	dev_fd = SAFE_OPEN(tst_device->dev, O_RDONLY);

	SAFE_TOUCH("testfile", 0644, NULL);
	file_fd = SAFE_OPEN("testfile", O_RDONLY);
}

static void cleanup(void)
{
	if (file_fd != -1)
		SAFE_CLOSE(file_fd);

	if (dev_fd != -1)
		SAFE_CLOSE(dev_fd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_device = 1,
	.needs_root = 1,
	.min_kver = "6.17",
	.needs_kconfigs = (const char *[]) {
		"CONFIG_BLK_DEV_INTEGRITY=y",
		NULL,
	},
	.bufs = (struct tst_buffers[]) {
		{&meta_cap, .size = sizeof(*meta_cap)},
		{},
	},
};
