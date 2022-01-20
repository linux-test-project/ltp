// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * [Description]
 *
 * Test whether the kernel properly advertises support for statx() attributes:
 *
 * - STATX_ATTR_COMPRESSED: The file is compressed by the filesystem.
 * - STATX_ATTR_IMMUTABLE: The file cannot be modified.
 * - STATX_ATTR_APPEND: The file can only be opened in append mode for writing.
 * - STATX_ATTR_NODUMP: File is not a candidate for backup when a backup
 *                        program such as dump(8) is run.
 *
 * xfs filesystem doesn't support STATX_ATTR_COMPRESSED flag, so we only test
 * three other flags.
 *
 * ext2, ext4, btrfs and xfs support statx syscall since the following commit
 *
 *  commit 93bc420ed41df63a18ae794101f7cbf45226a6ef
 *  Author: yangerkun <yangerkun@huawei.com>
 *  Date:   Mon Feb 18 09:07:02 2019 +0800
 *
 *  ext2: support statx syscall
 *
 *  commit 99652ea56a4186bc5bf8a3721c5353f41b35ebcb
 *  Author: David Howells <dhowells@redhat.com>
 *  Date:   Fri Mar 31 18:31:56 2017 +0100
 *
 *  ext4: Add statx support
 *
 *  commit 04a87e3472828f769a93655d7c64a27573bdbc2c
 *  Author: Yonghong Song <yhs@fb.com>
 *  Date:   Fri May 12 15:07:43 2017 -0700
 *
 *  Btrfs: add statx support
 *
 *  commit 5f955f26f3d42d04aba65590a32eb70eedb7f37d
 *  Author: Darrick J. Wong <darrick.wong@oracle.com>
 *  Date:   Fri Mar 31 18:32:03 2017 +0100
 *
 *  xfs: report crtime and attribute flags to statx
 */

#define _GNU_SOURCE
#include "tst_test.h"
#include "lapi/fs.h"
#include <stdlib.h>
#include "lapi/stat.h"

#define MOUNT_POINT "mntpoint"
#define TESTDIR MOUNT_POINT "/testdir"

#define ATTR(x) {.attr = x, .name = #x}

static struct {
	uint64_t attr;
	const char *name;
} attr_list[] = {
	ATTR(STATX_ATTR_COMPRESSED),
	ATTR(STATX_ATTR_APPEND),
	ATTR(STATX_ATTR_IMMUTABLE),
	ATTR(STATX_ATTR_NODUMP)
};

static uint64_t expected_mask;

static void setup(void)
{
	size_t i;
	int fd;

	SAFE_MKDIR(TESTDIR, 0777);

	/* Check general inode attribute support */
	fd = SAFE_OPEN(TESTDIR, O_RDONLY | O_DIRECTORY);
	TEST(ioctl(fd, FS_IOC_GETFLAGS, &i));
	SAFE_CLOSE(fd);

	if (TST_RET == -1 && TST_ERR == ENOTTY)
		tst_brk(TCONF | TTERRNO, "Inode attributes not supported");

	if (TST_RET)
		tst_brk(TBROK | TTERRNO, "Unexpected ioctl() error");

	for (i = 0, expected_mask = 0; i < ARRAY_SIZE(attr_list); i++)
		expected_mask |= attr_list[i].attr;

	/* STATX_ATTR_COMPRESSED not supported on XFS */
	if (!strcmp(tst_device->fs_type, "xfs"))
		expected_mask &= ~STATX_ATTR_COMPRESSED;

	/* Attribute support was added to Btrfs statx() in kernel v4.13 */
	if (!strcmp(tst_device->fs_type, "btrfs") && tst_kvercmp(4, 13, 0) < 0)
		tst_brk(TCONF, "statx() attributes not supported on Btrfs");
}

static void run(void)
{
	size_t i;
	struct statx buf;

	TST_EXP_PASS_SILENT(statx(AT_FDCWD, TESTDIR, 0, 0, &buf));

	for (i = 0; i < ARRAY_SIZE(attr_list); i++) {
		if (!(expected_mask & attr_list[i].attr))
			continue;

		if (buf.stx_attributes_mask & attr_list[i].attr)
			tst_res(TPASS, "%s is supported", attr_list[i].name);
		else
			tst_res(TFAIL, "%s not supported", attr_list[i].name);
	}
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_root = 1,
	.all_filesystems = 1,
	.mount_device = 1,
	.mntpoint = MOUNT_POINT,
	.min_kver = "4.11",
	.skip_filesystems = (const char *const[]) {
		"fuse",
		NULL
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "93bc420ed41d"},
		{"linux-git", "99652ea56a41"},
		{"linux-git", "04a87e347282"},
		{"linux-git", "5f955f26f3d4"},
		{}
	},
};
