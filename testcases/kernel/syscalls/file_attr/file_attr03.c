// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2025 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that `file_setattr` is correctly setting filesystems additional
 * attributes. We are running test on XFS only, since it's the only filesystem
 * currently implementing the features we need.
 */

#include "tst_test.h"
#include "lapi/fs.h"

#define MNTPOINT "mntpoint"
#define FILEPATH (MNTPOINT "/ltp_file")
#define BLOCKS 1024
#define PROJID 16

static int fd = -1;
static int block_size;
static struct fsxattr xattr;
static struct file_attr *attr;

static void run(void)
{
	fd = SAFE_CREAT(FILEPATH, 0777);

	TST_EXP_PASS(file_setattr(AT_FDCWD, FILEPATH,
			   attr, FILE_ATTR_SIZE_LATEST, 0));

	SAFE_IOCTL(fd, FS_IOC_FSGETXATTR, &xattr);
	SAFE_CLOSE(fd);

	TST_EXP_EQ_LI(xattr.fsx_xflags & FS_XFLAG_EXTSIZE, FS_XFLAG_EXTSIZE);
	TST_EXP_EQ_LI(xattr.fsx_xflags & FS_XFLAG_COWEXTSIZE, FS_XFLAG_COWEXTSIZE);
	TST_EXP_EQ_LI(xattr.fsx_extsize, BLOCKS * block_size);
	TST_EXP_EQ_LI(xattr.fsx_cowextsize, BLOCKS * block_size);
	TST_EXP_EQ_LI(xattr.fsx_projid, PROJID);

	SAFE_UNLINK(FILEPATH);
}

static void setup(void)
{
	block_size = tst_dev_block_size(MNTPOINT);

	attr->fa_xflags |= FS_XFLAG_EXTSIZE;
	attr->fa_xflags |= FS_XFLAG_COWEXTSIZE;
	attr->fa_extsize = BLOCKS * block_size;
	attr->fa_cowextsize = BLOCKS * block_size;
	attr->fa_projid = PROJID;
}

static void cleanup(void)
{
	if (fd != -1)
		SAFE_CLOSE(fd);
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
