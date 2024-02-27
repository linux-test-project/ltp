// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
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

#include "tst_test.h"
#include "libswap.h"
#include "lapi/syscalls.h"
#include "tst_kconfig.h"
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

	fiemap = (struct fiemap *)SAFE_MALLOC(sizeof(struct fiemap) + sizeof(struct fiemap_extent));
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

int make_swapfile(const char *swapfile, int blocks, int safe)
{
	struct statvfs fs_info;
	unsigned long blk_size, bs;
	size_t pg_size = sysconf(_SC_PAGESIZE);

	if (statvfs(".", &fs_info) == -1)
		return -1;

	blk_size = fs_info.f_bsize;

	/* To guarantee at least one page can be swapped out */
	if (blk_size * blocks < pg_size)
		bs = pg_size;
	else
		bs = blk_size;

	if (!tst_fs_has_free(".", bs * blocks, TST_BYTES))
		tst_brk(TCONF, "Insufficient disk space to create swap file");

	/* create file */
	if (prealloc_contiguous_file(swapfile, bs, blocks) != 0)
		tst_brk(TBROK, "Failed to create swapfile");

	/* Fill the file if needed (specific to old xfs filesystems) */
	if (tst_fs_type(swapfile) == TST_XFS_MAGIC) {
		if (tst_fill_file(swapfile, 0, bs, blocks) != 0)
			tst_brk(TBROK, "Failed to fill swapfile");
	}

	/* make the file swapfile */
	const char *argv[2 + 1];

	argv[0] = "mkswap";
	argv[1] = swapfile;
	argv[2] = NULL;

	return tst_cmd(argv, "/dev/null", "/dev/null", safe ?
				   TST_CMD_PASS_RETVAL | TST_CMD_TCONF_ON_MISSING : 0);
}

bool is_swap_supported(const char *filename)
{
	int i, sw_support = 0;
	int ret = make_swapfile(filename, 10, 1);
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

/*
 * Get kernel constant MAX_SWAPFILES value.
 */
int tst_max_swapfiles(void)
{
	unsigned int swp_migration_num = 0, swp_hwpoison_num = 0, swp_device_num = 0, swp_pte_marker_num = 0;
	struct tst_kconfig_var migration = TST_KCONFIG_INIT("CONFIG_MIGRATION");
	struct tst_kconfig_var memory = TST_KCONFIG_INIT("CONFIG_MEMORY_FAILURE");
	struct tst_kconfig_var device = TST_KCONFIG_INIT("CONFIG_DEVICE_PRIVATE");
	struct tst_kconfig_var marker = TST_KCONFIG_INIT("CONFIG_PTE_MARKER");

	tst_kconfig_read(&migration, 1);
	tst_kconfig_read(&memory, 1);
	tst_kconfig_read(&device, 1);
	tst_kconfig_read(&marker, 1);

	if (migration.choice == 'y') {
		if (tst_kvercmp(5, 19, 0) < 0)
			swp_migration_num = 2;
		else
			swp_migration_num = 3;
	}

	if (memory.choice == 'y')
		swp_hwpoison_num = 1;

	if (device.choice == 'y') {
		if (tst_kvercmp(4, 14, 0) >= 0)
			swp_device_num = 2;
		if (tst_kvercmp(5, 14, 0) >= 0)
			swp_device_num = 4;
	}

	if ((marker.choice == 'y' && tst_kvercmp(5, 19, 0) >= 0) ||
		tst_kvercmp(6, 2, 0) >= 0) {
		swp_pte_marker_num = 1;
	}

	return DEFAULT_MAX_SWAPFILE - swp_migration_num - swp_hwpoison_num - swp_device_num - swp_pte_marker_num;
}
