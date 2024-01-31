// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 * Author: Stanislav Kholmanskikh <stanislav.kholmanskikh@oracle.com>
 */

#include <errno.h>

#define TST_NO_DEFAULT_MAIN

#include "tst_test.h"
#include "libswap.h"
#include "lapi/syscalls.h"

static const char *const swap_supported_fs[] = {
	"ext2",
	"ext3",
	"ext4",
	"xfs",
	"vfat",
	"exfat",
	"ntfs",
	NULL
};

/*
 * Make a swap file
 */
int make_swapfile(const char *swapfile, int safe)
{
	if (!tst_fs_has_free(".", sysconf(_SC_PAGESIZE) * 10, TST_BYTES))
		tst_brk(TBROK, "Insufficient disk space to create swap file");

	/* create file */
	if (tst_fill_file(swapfile, 0, sysconf(_SC_PAGESIZE), 10) != 0)
		tst_brk(TBROK, "Failed to create swapfile");

	/* make the file swapfile */
	const char *argv[2 + 1];

	argv[0] = "mkswap";
	argv[1] = swapfile;
	argv[2] = NULL;

	return tst_cmd(argv, "/dev/null", "/dev/null", safe ?
				   TST_CMD_PASS_RETVAL | TST_CMD_TCONF_ON_MISSING : 0);
}

/*
 * Check swapon/swapoff support status of filesystems or files
 * we are testing on.
 */
void is_swap_supported(const char *filename)
{
	int i, sw_support = 0;
	int fibmap = tst_fibmap(filename);
	long fs_type = tst_fs_type(filename);
	const char *fstype = tst_fs_type_name(fs_type);

	for (i = 0; swap_supported_fs[i]; i++) {
		if (strstr(fstype, swap_supported_fs[i])) {
			sw_support = 1;
			break;
		}
	}

	int ret = make_swapfile(filename, 1);

	if (ret != 0) {
		if (fibmap == 1 && sw_support == 0)
			tst_brk(TCONF, "mkswap on %s not supported", fstype);
		else
			tst_brk(TFAIL, "mkswap on %s failed", fstype);
	}

	TEST(tst_syscall(__NR_swapon, filename, 0));
	if (TST_RET == -1) {
		if (errno == EPERM)
			tst_brk(TCONF, "Permission denied for swapon()");
		else if (fibmap == 1 && errno == EINVAL && sw_support == 0)
			tst_brk(TCONF, "Swapfile on %s not implemented", fstype);
		else
			tst_brk(TFAIL | TTERRNO, "swapon() on %s failed", fstype);
	}

	TEST(tst_syscall(__NR_swapoff, filename, 0));
	if (TST_RET == -1)
		tst_brk(TFAIL | TTERRNO, "swapoff on %s failed", fstype);
}
