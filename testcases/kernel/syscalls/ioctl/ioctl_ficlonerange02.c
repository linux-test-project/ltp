// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 Andrea Cervesato andrea.cervesato@suse.com
 */

/*\
 * [Description]
 *
 * This test verifies that ioctl() FICLONERANGE feature correctly raises
 * EINVAL when:
 * - filesystem does not support overlapping reflink ranges in the same file
 * - filesystem does not support reflinking on bad blocks alignment
 */

#include "tst_test.h"
#include "lapi/ficlone.h"

#define MNTPOINT "mnt"
#define SRCPATH MNTPOINT "/file0"
#define DSTPATH MNTPOINT "/file1"
#define CHUNKS 10

static struct file_clone_range *clone_range;
static int filesize;
static int alignment;
static char *data;

static void run(void)
{
	int src_fd;
	int dst_fd;

	src_fd = SAFE_OPEN(SRCPATH, O_CREAT | O_RDWR, 0640);
	SAFE_WRITE(SAFE_WRITE_ALL, src_fd, data, filesize);
	SAFE_FSYNC(src_fd);

	dst_fd = SAFE_OPEN(DSTPATH, O_CREAT | O_RDWR, 0640);

	clone_range->src_fd = dst_fd;
	clone_range->src_offset = 0;
	clone_range->src_length = filesize;
	clone_range->dest_offset = 0;

	TST_EXP_FAIL(ioctl(dst_fd, FICLONERANGE, clone_range), EINVAL,
		"overlapping reflink ranges in the same file");

	clone_range->src_fd = src_fd;
	clone_range->src_offset = 0;
	clone_range->src_length = alignment - 1;
	clone_range->dest_offset = 0;

	TST_EXP_FAIL(ioctl(dst_fd, FICLONERANGE, clone_range), EINVAL,
		"bad blocks alignment");

	SAFE_CLOSE(src_fd);
	SAFE_CLOSE(dst_fd);
}

static void setup(void)
{
	struct stat sb;

	SAFE_STAT(MNTPOINT, &sb);

	alignment = sb.st_blksize;
	filesize = alignment * CHUNKS;

	data = SAFE_MALLOC(filesize);
}

static void cleanup(void)
{
	free(data);
}

static struct tst_test test = {
	.timeout = 4,
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.min_kver = "4.5",
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.filesystems = (struct tst_fs []) {
		{.type = "btrfs"},
		{.type = "bcachefs"},
		{
			.type = "xfs",
			.min_kver = "4.16",
			.mkfs_ver = "mkfs.xfs >= 1.5.0",
			.mkfs_opts = (const char *const []) {"-m", "reflink=1", NULL},
		},
		{}
	},
	.bufs = (struct tst_buffers []) {
		{&clone_range, .size = sizeof(struct file_clone_range)},
		{},
	}
};
