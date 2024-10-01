// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 Andrea Cervesato andrea.cervesato@suse.com
 */

/*\
 * [Description]
 *
 * This test verifies that ioctl() FICLONE/FICLONERANGE feature correctly raises
 * exceptions when it's supposed to.
 */

#include "tst_test.h"
#include "lapi/ficlone.h"

#define MNTPOINT "mnt"

static struct file_clone_range *clone_range;

static int invalid_fd = -1;
static int rw_file = -1;
static int ro_file = -1;
static int wo_file = -1;
static int dir_fd = -1;
static int immut_fd = -1;
static int mnt_file = -1;

static struct tcase {
	int *src_fd;
	int *dst_fd;
	int errno_exp;
	char *msg;
} tcases[] = {
	{&invalid_fd, &rw_file, EBADF, "invalid source"},
	{&rw_file, &invalid_fd, EBADF, "invalid destination"},
	{&rw_file, &ro_file, EBADF, "read-only destination"},
	{&wo_file, &rw_file, EBADF, "write-only source"},
	{&rw_file, &dir_fd, EISDIR, "source is a directory"},
	{&dir_fd, &rw_file, EISDIR, "destination is a directory"},
	{&mnt_file, &immut_fd, EPERM, "destination is immutable"},
	{&rw_file, &mnt_file, EXDEV, "destination is on a different mount"},
	{&mnt_file, &rw_file, EXDEV, "source is on a different mount"},
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TST_EXP_FAIL(ioctl(*tc->dst_fd, FICLONE, *tc->src_fd),
		tc->errno_exp,
		"%s", tc->msg);

	clone_range->src_fd = *tc->src_fd;

	TST_EXP_FAIL(ioctl(*tc->dst_fd, FICLONERANGE, clone_range),
		tc->errno_exp,
		"%s", tc->msg);
}

static void setup(void)
{
	int attr;
	struct stat sb;

	rw_file = SAFE_OPEN("ok_only", O_CREAT | O_RDWR, 0640);
	ro_file = SAFE_OPEN("rd_only", O_CREAT | O_RDONLY, 0640);
	wo_file = SAFE_OPEN("rw_only", O_CREAT | O_WRONLY, 0640);

	if (access("mydir", F_OK) == -1)
		SAFE_MKDIR("mydir", 0640);

	dir_fd = SAFE_OPEN("mydir", O_DIRECTORY, 0640);

	attr = FS_IMMUTABLE_FL;
	immut_fd = SAFE_OPEN(MNTPOINT"/immutable", O_CREAT | O_RDWR, 0640);
	SAFE_IOCTL(immut_fd, FS_IOC_SETFLAGS, &attr);

	mnt_file = SAFE_OPEN(MNTPOINT"/file", O_CREAT | O_RDWR, 0640);

	SAFE_STAT(MNTPOINT, &sb);

	clone_range->src_offset = 0;
	clone_range->src_length = sb.st_blksize;
	clone_range->dest_offset = 0;
}

static void cleanup(void)
{
	int attr;

	SAFE_IOCTL(immut_fd, FS_IOC_GETFLAGS, &attr);
	attr &= ~FS_IMMUTABLE_FL;
	SAFE_IOCTL(immut_fd, FS_IOC_SETFLAGS, &attr);
	SAFE_CLOSE(immut_fd);

	SAFE_CLOSE(rw_file);
	SAFE_CLOSE(ro_file);
	SAFE_CLOSE(wo_file);
	SAFE_CLOSE(dir_fd);
	SAFE_CLOSE(mnt_file);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
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
