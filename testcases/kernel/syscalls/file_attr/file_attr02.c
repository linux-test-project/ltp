// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2025 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that `file_getattr` is correctly reading filesystems additional
 * attributes. We are running test on XFS only, since it's the only filesystem
 * currently implementing the features we need.
 */

#include "tst_test.h"
#include "lapi/fs.h"

#define MNTPOINT "mntpoint"
#define FILENAME "ltp_file"
#define BLOCKS 1024
#define PROJID 16

static int fd = -1;
static int dfd = -1;
static struct fsxattr xattr;
static struct file_attr *attr;

static void run(void)
{
	memset(attr, 0, sizeof(*attr));

	TST_EXP_PASS(file_getattr(
		dfd, FILENAME,
		attr, FILE_ATTR_SIZE_LATEST,
		0));

	TST_EXP_EQ_LI(attr->fa_xflags, xattr.fsx_xflags);
	TST_EXP_EQ_LI(attr->fa_extsize, xattr.fsx_extsize);
	TST_EXP_EQ_LI(attr->fa_cowextsize, xattr.fsx_cowextsize);
	TST_EXP_EQ_LI(attr->fa_nextents, xattr.fsx_nextents);
	TST_EXP_EQ_LI(attr->fa_projid, PROJID);
	TST_EXP_EQ_LI(attr->fa_projid, xattr.fsx_projid);
}

static void setup(void)
{
	int block_size;

	block_size = tst_dev_block_size(MNTPOINT);

	dfd = SAFE_OPEN(MNTPOINT, O_RDONLY);
	fd = SAFE_CREAT(MNTPOINT "/" FILENAME, 0777);

	SAFE_IOCTL(fd, FS_IOC_FSGETXATTR, &xattr);

	xattr.fsx_xflags |= FS_XFLAG_EXTSIZE;
	xattr.fsx_xflags |= FS_XFLAG_COWEXTSIZE;
	xattr.fsx_extsize = BLOCKS * block_size;
	xattr.fsx_cowextsize = BLOCKS * block_size;
	xattr.fsx_projid = PROJID;

	SAFE_IOCTL(fd, FS_IOC_FSSETXATTR, &xattr);

	/* this will force at least one extent to be allocated */
	SAFE_WRITE(SAFE_WRITE_ALL, fd, "a", 1);

	SAFE_IOCTL(fd, FS_IOC_FSGETXATTR, &xattr);
	SAFE_CLOSE(fd);
}

static void cleanup(void)
{
	if (fd != -1)
		SAFE_CLOSE(fd);

	if (dfd != -1)
		SAFE_CLOSE(dfd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.mntpoint = MNTPOINT,
	.needs_root = 1,
	.mount_device = 1,
	.filesystems = (struct tst_fs []) {
		{.type = "xfs"},
		{}
	},
	.bufs = (struct tst_buffers []) {
		{&attr, .size = sizeof(struct file_attr)},
		{}
	}
};
