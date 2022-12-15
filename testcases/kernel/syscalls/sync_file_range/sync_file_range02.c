// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linaro Limited. All rights reserved.
 * Author: Sumit Garg <sumit.garg@linaro.org>
 */

/*\
 * [Description]
 *
 * Tests if sync_file_range() does sync a test file range with a many dirty pages
 * to a block device. Also, it tests all supported filesystems on a test block
 * device.
 */

#define _GNU_SOURCE
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include "tst_test.h"
#include "lapi/sync_file_range.h"
#include "check_sync_file_range.h"

#define MNTPOINT		"mnt_point"
#define FNAME1			MNTPOINT"/test1"
#define FNAME2			MNTPOINT"/test2"
#define FNAME3			MNTPOINT"/test3"
#define FILE_SZ_MB		32
#define FILE_SZ			(FILE_SZ_MB * TST_MB)
#define MODE			0644

struct testcase {
	char *fname;
	off_t sync_off;
	off_t sync_size;
	size_t exp_sync_size;
	off_t write_off;
	size_t write_size_mb;
	const char *desc;
};

static void verify_sync_file_range(struct testcase *tc)
{
	int fd;
	unsigned long written;

	fd = SAFE_OPEN(tc->fname, O_RDWR|O_CREAT, MODE);

	lseek(fd, tc->write_off, SEEK_SET);

	tst_dev_sync(fd);
	tst_dev_bytes_written(tst_device->dev);

	tst_fill_fd(fd, 0, TST_MB, tc->write_size_mb);

	TEST(sync_file_range(fd, tc->sync_off, tc->sync_size,
			     SYNC_FILE_RANGE_WAIT_BEFORE |
			     SYNC_FILE_RANGE_WRITE |
			     SYNC_FILE_RANGE_WAIT_AFTER));

	if (TST_RET)
		tst_brk(TFAIL | TTERRNO, "sync_file_range() failed");

	written = tst_dev_bytes_written(tst_device->dev);

	fsync(fd);

	SAFE_CLOSE(fd);

	if (written >= tc->exp_sync_size)
		tst_res(TPASS, "%s", tc->desc);
	else
		tst_res(TFAIL, "%s: Synced %li, expected %li", tc->desc,
		        written, tc->exp_sync_size);
}

static struct testcase testcases[] = {
	{FNAME1,
	 0, FILE_SZ,
	 FILE_SZ,
	 0, FILE_SZ_MB,
	 "Sync equals write"},
	{FNAME2,
	 FILE_SZ/4, FILE_SZ/2,
	 FILE_SZ/2,
	 0, FILE_SZ_MB,
	 "Sync inside of write"},
	{FNAME3,
	 FILE_SZ/4, FILE_SZ/2,
	 FILE_SZ/4,
	 FILE_SZ/2, FILE_SZ_MB/4,
	 "Sync overlaps with write"},
};

static void run(unsigned int i)
{
	verify_sync_file_range(&testcases[i]);
}

static void setup(void)
{
	if (!check_sync_file_range())
		tst_brk(TCONF, "sync_file_range() not supported");

	/*
	 * Fat does not support sparse files, we have to pre-fill the file so
	 * that the zero-filled start of the file has been written to disk
	 * before the test starts.
	 */
	if (!strcmp(tst_device->fs_type, "vfat")) {
		tst_res(TINFO, "Pre-filling file");
		tst_fill_file(FNAME3, 0, TST_MB, FILE_SZ_MB);
		int fd = SAFE_OPEN(FNAME3, O_RDONLY);
		fsync(fd);
		SAFE_CLOSE(fd);
	}
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(testcases),
	.needs_root = 1,
	.mount_device = 1,
	.all_filesystems = 1,
	.skip_filesystems = (const char *const []){
		"fuse",
		"ntfs",
		"tmpfs",
		NULL
	},
	.mntpoint = MNTPOINT,
	.setup = setup,
	.test = run,
};
