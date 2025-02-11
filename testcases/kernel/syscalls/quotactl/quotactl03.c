// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Fujitsu Ltd.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*\
 * quotactl(2) with XGETNEXTQUOTA looks for the next active quota for an user
 * equal or higher to a given ID, in this test the ID is specified to a value
 * close to UINT_MAX(max value of unsigned int). When reaching the upper limit
 * and finding no active quota, it should return -1 and set errno to ENOENT.
 * Actually, quotactl(2) overflows and and return 0 as the "next" active id.
 *
 * This kernel bug of xfs has been fixed in:
 *
 *  commit 657bdfb7f5e68ca5e2ed009ab473c429b0d6af85
 *  Author: Eric Sandeen <sandeen@redhat.com>
 *  Date:   Tue Jan 17 11:43:38 2017 -0800
 *
 *  xfs: don't wrap ID in xfs_dq_get_next_id
 */

#define _GNU_SOURCE
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/quota.h>

#include "tst_test.h"
#include "quotactl_syscall_var.h"

#ifdef HAVE_XFS_XQM_H
# include <xfs/xqm.h>

static uint32_t test_id = 0xfffffffc;

static void verify_quota(void)
{
	struct fs_disk_quota res_dquota;

	res_dquota.d_id = 1;

	TEST(do_quotactl(fd, QCMD(Q_XGETNEXTQUOTA, USRQUOTA), tst_device->dev,
		test_id, (void *)&res_dquota));
	if (TST_RET != -1) {
		tst_res(TFAIL, "quotactl() found the next active ID: %u unexpectedly",
				res_dquota.d_id);
		return;
	}

	if (TST_ERR == EINVAL)
		tst_brk(TCONF | TTERRNO,
			"Q_XGETNEXTQUOTA wasn't supported in quotactl()");

	if (TST_ERR != ENOENT)
		tst_res(TFAIL | TTERRNO, "quotactl() failed unexpectedly with %s expected ENOENT",
				tst_strerrno(TST_ERR));
	else
		tst_res(TPASS, "quotactl() failed with ENOENT as expected");
}

static void setup(void)
{
	quotactl_info();
	fd = SAFE_OPEN(MNTPOINT, O_RDONLY);
}

static void cleanup(void)
{
	if (fd > -1)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_XFS_QUOTA",
		NULL
	},
	.test_all = verify_quota,
	.mount_device = 1,
	.filesystems = (struct tst_fs []) {
		{
			.type = "xfs",
			.mnt_data = "usrquota",
		},
		{}
	},
	.mntpoint = MNTPOINT,
	.test_variants = QUOTACTL_SYSCALL_VARIANTS,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "657bdfb7f5e6"},
		{}
	}
};

#else
	TST_TEST_TCONF("System doesn't have <xfs/xqm.h>");
#endif
