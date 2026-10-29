// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Wei Gao <wegao@suse.com>
 */

/*\
 * Verify basic fiemap ioctl functionality, including:
 *
 * - The ioctl returns EBADR if it receives invalid fm_flags.
 * - 0 extents are reported for an empty file.
 * - The ioctl correctly retrieves single and multiple extent mappings after writing to the file.
 */

#include <linux/fs.h>
#include <linux/fiemap.h>
#include <stdlib.h>
#include <sys/statvfs.h>

#include "tst_test.h"

#define MNTPOINT "mntpoint"
#define TESTFILE "testfile"
#define NUM_EXTENT 3

static void print_extens(struct fiemap *fiemap)
{
	tst_res(TDEBUG, "File extent count: %u", fiemap->fm_mapped_extents);

	for (unsigned int i = 0; i < fiemap->fm_mapped_extents; ++i) {
		tst_res(TDEBUG, "Extent %u: Logical offset: %llu, Physical offset: %llu, flags: %u, Length: %llu",
				i + 1,
				fiemap->fm_extents[i].fe_logical,
				fiemap->fm_extents[i].fe_physical,
				fiemap->fm_extents[i].fe_flags,
				fiemap->fm_extents[i].fe_length);
	}
}

static void check_extent_count(struct fiemap *fiemap, unsigned int fm_mapped_extents)
{
	TST_EXP_EXPR(fiemap->fm_mapped_extents == fm_mapped_extents,
		"extent fm_mapped_extents is %d", fm_mapped_extents);
}

static void check_extent(struct fiemap *fiemap,	int index_extents, unsigned int fe_mask,
					unsigned int fe_flags, unsigned int fe_length)
{
	struct fiemap_extent *extent = &fiemap->fm_extents[index_extents];

	TST_EXP_EQ_LU((extent->fe_flags & fe_mask), fe_flags);
	TST_EXP_EXPR(extent->fe_physical >= 1, "fe_physical > %d", 1);
	TST_EXP_EQ_LU(extent->fe_length, fe_length);
}

static void verify_ioctl(void)
{
	int fd;
	struct fiemap *fiemap;
	struct statvfs fs_info;
	unsigned long blk_size;

	fd = SAFE_OPEN(TESTFILE, O_RDWR | O_CREAT, 0644);

	SAFE_STATVFS(".", &fs_info);

	blk_size = fs_info.f_bsize;

	fiemap = SAFE_MALLOC(sizeof(struct fiemap) + sizeof(struct fiemap_extent) * NUM_EXTENT);
	fiemap->fm_start = 0;
	fiemap->fm_length = ~0ULL;
	fiemap->fm_extent_count = 1;

	fiemap->fm_flags = -1;
	TST_EXP_FAIL(ioctl(fd, FS_IOC_FIEMAP, fiemap), EBADR);

	fiemap->fm_flags =  0;
	TST_EXP_PASS(ioctl(fd, FS_IOC_FIEMAP, fiemap));
	print_extens(fiemap);
	TST_EXP_EXPR(fiemap->fm_mapped_extents == 0,
		"Empty file should have 0 extends mapped");

	char *buf = SAFE_MALLOC(blk_size);

	SAFE_WRITE(SAFE_WRITE_ANY, fd, buf, blk_size);
	fiemap->fm_flags = FIEMAP_FLAG_SYNC;
	TST_EXP_PASS(ioctl(fd, FS_IOC_FIEMAP, fiemap));
	print_extens(fiemap);
	check_extent_count(fiemap, 1);
	check_extent(fiemap, 0, FIEMAP_EXTENT_LAST, FIEMAP_EXTENT_LAST, blk_size);

	fiemap->fm_extent_count = NUM_EXTENT;
	SAFE_LSEEK(fd, 2 * blk_size, SEEK_SET);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, buf, blk_size);
	SAFE_LSEEK(fd, 4 * blk_size, SEEK_SET);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, buf, blk_size);
	TST_EXP_PASS(ioctl(fd, FS_IOC_FIEMAP, fiemap));
	print_extens(fiemap);
	check_extent_count(fiemap, NUM_EXTENT);

	for (int i = 0; i < NUM_EXTENT - 1; i++)
		check_extent(fiemap, i, FIEMAP_EXTENT_LAST, 0, blk_size);

	check_extent(fiemap, NUM_EXTENT - 1, FIEMAP_EXTENT_LAST, FIEMAP_EXTENT_LAST, blk_size);

	free(buf);
	free(fiemap);
	SAFE_CLOSE(fd);
	SAFE_UNLINK(TESTFILE);
}

static void setup(void)
{
	SAFE_CHDIR(MNTPOINT);
}

static struct tst_test test = {
	.setup = setup,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.skip_filesystems = (const char *const[]) {
		"exfat", "vfat", "fuse", "ntfs", "tmpfs", NULL
	},
	.test_all = verify_ioctl,
	.needs_root = 1,
};
