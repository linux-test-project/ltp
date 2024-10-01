// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 Andrea Cervesato andrea.cervesato@suse.com
 */

/*\
 * [Description]
 *
 * This test verifies that ioctl() FICLONERANGE feature clones file content from
 * one file to an another.
 *
 * [Algorithm]
 *
 * - populate source file
 * - clone a portion of source content inside destination file
 * - verify that source content portion has been cloned inside destination file
 * - write a single byte inside destination file
 * - verify that source content didn't change while destination did
 */

#include "tst_test.h"
#include "lapi/ficlone.h"

#define MNTPOINT "mnt"
#define SRCPATH MNTPOINT "/file0"
#define DSTPATH MNTPOINT "/file1"
#define CHUNKS 64

static struct file_clone_range *clone_range;
static int filesize;
static int offset;
static int leftsize;
static int src_fd = -1;
static int dst_fd = -1;
static char *data;
static char *buff;

static void run(void)
{
	struct stat src_stat;
	struct stat dst_stat;

	for (int i = 0; i < filesize; i++)
		data[i] = 'a' + (rand() % 21);

	src_fd = SAFE_OPEN(SRCPATH, O_CREAT | O_RDWR, 0640);
	dst_fd = SAFE_OPEN(DSTPATH, O_CREAT | O_RDWR, 0640);

	tst_res(TINFO, "Writing data inside src file");

	SAFE_WRITE(SAFE_WRITE_ALL, src_fd, data, filesize);
	SAFE_FSYNC(src_fd);

	clone_range->src_fd = src_fd;
	TST_EXP_PASS(ioctl(dst_fd, FICLONERANGE, clone_range));
	if (TST_RET == -1)
		return;

	SAFE_FSYNC(dst_fd);

	tst_res(TINFO, "Verifing that data is cloned between files");

	SAFE_FSTAT(src_fd, &src_stat);
	SAFE_FSTAT(dst_fd, &dst_stat);

	TST_EXP_EXPR(src_stat.st_ino != dst_stat.st_ino,
		"inode is different. %lu != %lu",
		src_stat.st_ino,
		dst_stat.st_ino);

	TST_EXP_EQ_LI(src_stat.st_size, filesize);
	TST_EXP_EQ_LI(dst_stat.st_size, leftsize);

	SAFE_READ(0, dst_fd, buff, leftsize);

	TST_EXP_EXPR(!strncmp(buff, data + offset, leftsize),
		"dst file has been cloned (%d bytes)",
		leftsize);

	tst_res(TINFO, "Writing a byte inside dst file");

	SAFE_LSEEK(dst_fd, 0, SEEK_SET);
	SAFE_WRITE(SAFE_WRITE_ALL, dst_fd, "!", 1);
	SAFE_FSYNC(dst_fd);

	tst_res(TINFO, "Verifing that src file content didn't change");

	SAFE_FSTAT(src_fd, &src_stat);
	SAFE_FSTAT(dst_fd, &dst_stat);

	TST_EXP_EQ_LI(src_stat.st_size, filesize);
	TST_EXP_EQ_LI(dst_stat.st_size, leftsize);

	SAFE_LSEEK(src_fd, 0, SEEK_SET);
	SAFE_READ(0, src_fd, buff, filesize);

	TST_EXP_EXPR(!strncmp(buff, data, filesize),
		"src file content didn't change");

	SAFE_CLOSE(src_fd);
	SAFE_CLOSE(dst_fd);

	SAFE_UNLINK(SRCPATH);
	SAFE_UNLINK(DSTPATH);
}

static void setup(void)
{
	struct stat sb;

	SAFE_STAT(MNTPOINT, &sb);

	filesize = sb.st_blksize * CHUNKS;
	offset = filesize / 4;
	leftsize = filesize - offset;

	clone_range->src_offset = offset;
	clone_range->src_length = leftsize;
	clone_range->dest_offset = 0;

	data = SAFE_MALLOC(filesize);
	buff = SAFE_MALLOC(filesize);

	srand(time(NULL));
}

static void cleanup(void)
{
	free(buff);
	free(data);

	if (src_fd != -1)
		SAFE_CLOSE(src_fd);

	if (dst_fd != -1)
		SAFE_CLOSE(dst_fd);
}

static struct tst_test test = {
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
