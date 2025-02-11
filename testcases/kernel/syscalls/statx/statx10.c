// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * It is a basic test for STATX_DIOALIGN mask on ext4 and xfs filesystem.
 *
 * - STATX_DIOALIGN   Want stx_dio_mem_align and stx_dio_offset_align value
 *
 * Check these two values are nonzero under dio situation when STATX_DIOALIGN
 * in the request mask.
 *
 * On ext4, files that use certain filesystem features (data journaling,
 * encryption, and verity) fall back to buffered I/O. But ltp creates own
 * filesystem by enabling mount_device in tst_test struct. If we set block
 * device to LTP_DEV environment, we use this block device to mount by using
 * default mount option. Otherwise, use loop device to simuate it. So it can
 * avoid these above situations and don't fall back to buffered I/O.
 *
 * Minimum Linux version required is v6.1.
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include "tst_test.h"
#include "lapi/stat.h"
#include "lapi/fcntl.h"

#define MNTPOINT "mnt_point"
#define TESTFILE MNTPOINT"/testfile"

static void verify_statx(void)
{
	struct statx buf;

	TST_EXP_PASS_SILENT(statx(AT_FDCWD, TESTFILE, 0, STATX_DIOALIGN, &buf),
		"statx(AT_FDCWD, %s, 0, STATX_DIOALIGN, &buf)", TESTFILE);

	if (!(buf.stx_mask & STATX_DIOALIGN)) {
		tst_res(TCONF, "Filesystem does not support STATX_DIOALIGN");
		return;
	}

#ifdef HAVE_STRUCT_STATX_STX_DIO_MEM_ALIGN
	if (buf.stx_dio_mem_align != 0)
		tst_res(TPASS, "stx_dio_mem_align:%u", buf.stx_dio_mem_align);
	else
		tst_res(TFAIL, "stx_dio_mem_align was 0, but DIO should be supported");

	if (buf.stx_dio_offset_align != 0)
		tst_res(TPASS, "stx_dio_offset_align:%u", buf.stx_dio_offset_align);
	else
		tst_res(TFAIL, "stx_dio_offset_align was 0, but DIO should be supported");
#else
	tst_res(TCONF, "glibc statx struct miss stx_dio_mem_align field");
#endif
}

static void setup(void)
{
	int fd = -1;

	if (strcmp(tst_device->fs_type, "xfs") && strcmp(tst_device->fs_type, "ext4"))
		tst_brk(TCONF, "This test only supports ext4 and xfs");

	SAFE_FILE_PRINTF(TESTFILE, "AAAA");
	fd = open(TESTFILE, O_RDWR | O_DIRECT);
	if (fd == -1) {
		if (errno == EINVAL)
			tst_brk(TCONF,
				"The regular file is not on a filesystem that support DIO");
		else
			tst_brk(TBROK | TERRNO,
				"The regular file is open with O_RDWR | O_DIRECT failed");
	}
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = verify_statx,
	.setup = setup,
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.mount_device = 1,
	.all_filesystems = 1,
};
