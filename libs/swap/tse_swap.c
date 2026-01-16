// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 * Copyright (c) Linux Test Project, 2014-2024
 * Author: Stanislav Kholmanskikh <stanislav.kholmanskikh@oracle.com>
 */

#include <sys/statvfs.h>
#include <linux/fs.h>
#include <errno.h>
#include <linux/fiemap.h>
#include <stdlib.h>
#include <stdbool.h>

#define TST_NO_DEFAULT_MAIN
#define DEFAULT_MAX_SWAPFILE 32
#define BUFSIZE 200

#include "tst_test.h"
#include "tse_swap.h"
#include "lapi/syscalls.h"
#include "tst_kconfig.h"
#include "tst_kvercmp.h"
#include "tst_safe_stdio.h"

static const char *const swap_supported_fs[] = {
	"btrfs",
	"ext2",
	"ext3",
	"ext4",
	"xfs",
	"vfat",
	"exfat",
	"ntfs",
	NULL
};

static void set_nocow_attr(const char *filename)
{
	int fd;
	int attrs;

	tst_res(TINFO, "FS_NOCOW_FL attribute set on %s", filename);

	fd = SAFE_OPEN(filename, O_RDONLY);

	SAFE_IOCTL(fd, FS_IOC_GETFLAGS, &attrs);

	attrs |= FS_NOCOW_FL;

	SAFE_IOCTL(fd, FS_IOC_SETFLAGS, &attrs);

	SAFE_CLOSE(fd);
}

static int prealloc_contiguous_file(const char *path, size_t bs, size_t bcount)
{
	int fd;

	fd = open(path, O_CREAT|O_WRONLY|O_TRUNC, 0600);
	if (fd < 0)
		return -1;

	/* Btrfs file need set 'nocow' attribute */
	if (tst_fs_type(path) == TST_BTRFS_MAGIC)
		set_nocow_attr(path);

	if (tst_prealloc_size_fd(fd, bs, bcount)) {
		close(fd);
		unlink(path);
		return -1;
	}

	if (close(fd) < 0) {
		unlink(path);
		return -1;
	}

	return 0;
}

static int file_is_contiguous(const char *filename)
{
	int fd, contiguous = 0;
	struct fiemap *fiemap;

	if (tst_fibmap(filename) == 0) {
		contiguous = 1;
		goto out;
	}

	if (tst_fs_type(filename) == TST_TMPFS_MAGIC)
		goto out;

	fd = SAFE_OPEN(filename, O_RDONLY);

	fiemap = (struct fiemap *)SAFE_MALLOC(sizeof(struct fiemap)
					      + sizeof(struct fiemap_extent));

	memset(fiemap, 0, sizeof(struct fiemap) + sizeof(struct fiemap_extent));

	fiemap->fm_start = 0;
	fiemap->fm_length = ~0;
	fiemap->fm_flags = 0;
	fiemap->fm_extent_count = 1;

	SAFE_IOCTL(fd, FS_IOC_FIEMAP, fiemap);

	/*
	 * fiemap->fm_mapped_extents != 1:
	 *   This checks if the file does not have exactly one extent. If there are more
	 *   or zero extents, the file is not stored in a single contiguous block.
	 *
	 * fiemap->fm_extents[0].fe_logical != 0:
	 *   This checks if the first extent does not start at the logical offset 0 of
	 *   the file. If it doesn't, it indicates that the file's first block of data
	 *   is not at the beginning of the file, which implies non-contiguity.
	 *
	 * (fiemap->fm_extents[0].fe_flags & FIEMAP_EXTENT_LAST) != FIEMAP_EXTENT_LAST:
	 *   This checks if the first extent does not have the FIEMAP_EXTENT_LAST flag set.
	 *   If the flag isn't set, it means that this extent is not the last one, suggesting
	 *   that there are more extents and the file is not contiguous.
	 */
	if (fiemap->fm_mapped_extents != 1 ||
		fiemap->fm_extents[0].fe_logical != 0 ||
		(fiemap->fm_extents[0].fe_flags & FIEMAP_EXTENT_LAST) != FIEMAP_EXTENT_LAST) {

		tst_res(TINFO, "File '%s' is not contiguous", filename);
		contiguous = 0;
	}

	SAFE_CLOSE(fd);
	free(fiemap);

out:
	return contiguous;
}

int make_swapfile(const char *file, const int lineno,
			const char *swapfile, unsigned int num,
			int safe, enum swapfile_method method)
{
	struct statvfs fs_info;
	unsigned long blk_size;
	unsigned int blocks = 0;
	size_t pg_size = sysconf(_SC_PAGESIZE);
	char mnt_path[PATH_MAX];

	safe_statvfs(file, lineno, ".", &fs_info);

	blk_size = fs_info.f_bsize;

	if (method == SWAPFILE_BY_SIZE) {
		tst_res_(file, lineno, TINFO, "create a swapfile size of %u megabytes (MB)", num);
		blocks = num * 1024 * 1024 / blk_size;
	} else if (method == SWAPFILE_BY_BLKS) {
		blocks = num;
		tst_res_(file, lineno, TINFO, "create a swapfile with %u block numbers", blocks);
	} else {
		tst_brk_(file, lineno, TBROK, "Invalid method, please see include/tse_swap.h");
	}

	/* To guarantee at least one page can be swapped out */
	if (blk_size * blocks < pg_size) {
		tst_res_(file, lineno, TWARN, "Swapfile size is less than the system page size. "
			"Using page size (%lu bytes) instead of block size (%lu bytes).",
			(unsigned long)pg_size, blk_size);
		blk_size = pg_size;
	}

	safe_sscanf(file, lineno, swapfile, "%[^/]", mnt_path);

	if (!tst_fs_has_free(mnt_path, blk_size * blocks, TST_BYTES))
		tst_brk_(file, lineno, TCONF, "Insufficient disk space to create swap file");

	/* create file */
	if (prealloc_contiguous_file(swapfile, blk_size, blocks) != 0)
		tst_brk_(file, lineno, TBROK, "Failed to create swapfile");

	/* Fill the file if needed (specific to old xfs filesystems) */
	if (tst_fs_type(swapfile) == TST_XFS_MAGIC) {
		if (tst_fill_file(swapfile, 0, blk_size, blocks) != 0)
			tst_brk_(file, lineno, TBROK, "Failed to fill swapfile");
	}

	/* make the file swapfile */
	const char *const argv[] = {"mkswap", swapfile, NULL};

	return tst_cmd(argv, "/dev/null", "/dev/null", safe ?
			TST_CMD_PASS_RETVAL | TST_CMD_TCONF_ON_MISSING : 0);
}

bool is_swap_supported(const char *filename)
{
	int i, sw_support = 0;
	int ret = SAFE_MAKE_SMALL_SWAPFILE(filename);
	int fi_contiguous = file_is_contiguous(filename);
	long fs_type = tst_fs_type(filename);
	const char *fstype = tst_fs_type_name(fs_type);

	if (fs_type == TST_BTRFS_MAGIC &&
			tst_kvercmp(5, 0, 0) < 0)
		tst_brk(TCONF, "Swapfile on Btrfs (kernel < 5.0) not implemented");

	for (i = 0; swap_supported_fs[i]; i++) {
		if (strstr(fstype, swap_supported_fs[i])) {
			sw_support = 1;
			break;
		}
	}

	if (ret != 0) {
		if (fi_contiguous == 0 && sw_support == 0) {
			tst_brk(TCONF, "mkswap on %s not supported", fstype);
		} else {
			tst_res(TFAIL, "mkswap on %s failed", fstype);
			return false;
		}
	}

	TEST(tst_syscall(__NR_swapon, filename, 0));
	if (TST_RET == -1) {
		if (errno == EPERM) {
			tst_brk(TCONF, "Permission denied for swapon()");
		} else if (errno == EINVAL && fi_contiguous == 0 && sw_support == 0) {
			tst_brk(TCONF, "Swapfile on %s not implemented", fstype);
		} else {
			tst_res(TFAIL | TTERRNO, "swapon() on %s failed", fstype);
			return false;
		}
	}

	TEST(tst_syscall(__NR_swapoff, filename, 0));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "swapoff on %s failed", fstype);
		return false;
	}

	return true;
}

int tse_count_swaps(void)
{
	FILE *fp;
	int used = -1;
	char buf[BUFSIZE];

	fp = SAFE_FOPEN("/proc/swaps", "r");
	if (fp == NULL)
		return -1;

	while (fgets(buf, BUFSIZE, fp) != NULL)
		used++;

	SAFE_FCLOSE(fp);
	if (used < 0)
		tst_brk(TBROK, "can't read /proc/swaps to get used swapfiles resource total");

	return used;
}
