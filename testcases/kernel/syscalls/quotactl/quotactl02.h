// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

#ifndef QUOTACTL02_H
#define QUOTACTL02_H

#define _GNU_SOURCE
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include "tst_test.h"
#include "quotactl_syscall_var.h"

#ifdef HAVE_XFS_XQM_H
# include <xfs/xqm.h>

static struct fs_disk_quota set_dquota = {
	.d_rtb_softlimit = 1000,
	.d_fieldmask = FS_DQ_RTBSOFT
};
static uint32_t test_id;
static int x_getnextquota_nsup;
static int x_getstatv_nsup;

static void check_support_cmd(int quotatype)
{
	struct fs_disk_quota resfs_dquota;
	struct fs_quota_statv resfs_qstatv = {
		.qs_version = FS_QSTATV_VERSION1
	};

	x_getnextquota_nsup = 0;
	x_getstatv_nsup = 0;

	TEST(do_quotactl(fd, QCMD(Q_XGETNEXTQUOTA, quotatype), tst_device->dev,
		      test_id, (void *) &resfs_dquota));
	if (TST_ERR == EINVAL || TST_ERR == ENOSYS)
		x_getnextquota_nsup = 1;

	TEST(do_quotactl(fd, QCMD(Q_XGETQSTATV, quotatype), tst_device->dev, test_id,
		      (void *) &resfs_qstatv));
	if (TST_ERR == EINVAL || TST_ERR == ENOSYS)
		x_getstatv_nsup = 1;

}

static void check_qoff(int subcmd, char *desp, int flag)
{
	int res;
	struct fs_quota_stat res_qstat;

	res = do_quotactl(fd, subcmd, tst_device->dev, test_id, (void *) &res_qstat);
	if (res == -1) {
		tst_res(TFAIL | TERRNO,
			"quotactl() failed to get xfs quota off status");
		return;
	}

	if (res_qstat.qs_flags & flag) {
		tst_res(TFAIL, "xfs quota enforcement was on unexpectedly");
		return;
	}

	tst_res(TPASS, "quotactl() succeeded to %s", desp);
}

static void check_qon(int subcmd, char *desp, int flag)
{
	int res;
	struct fs_quota_stat res_qstat;

	res = do_quotactl(fd, subcmd, tst_device->dev, test_id, (void *) &res_qstat);
	if (res == -1) {
		tst_res(TFAIL | TERRNO,
			"quotactl() failed to get xfs quota on status");
		return;
	}

	if (!(res_qstat.qs_flags & flag)) {
		tst_res(TFAIL, "xfs quota enforcement was off unexpectedly");
		return;
	}

	tst_res(TPASS, "quotactl() succeeded to %s", desp);
}

static void check_qoffv(int subcmd, char *desp, int flag)
{
	int res;
	struct fs_quota_statv res_qstatv = {
		.qs_version = FS_QSTATV_VERSION1,
	};

	res = do_quotactl(fd, subcmd, tst_device->dev, test_id, (void *) &res_qstatv);
	if (res == -1) {
		tst_res(TFAIL | TERRNO,
			"quotactl() failed to get xfs quota off stav");
		return;
	}

	if (res_qstatv.qs_flags & flag) {
		tst_res(TFAIL, "xfs quota enforcement was on unexpectedly");
		return;
	}

	tst_res(TPASS, "quotactl() succeeded to %s", desp);
}

static void check_qonv(int subcmd, char *desp, int flag)
{
	int res;
	struct fs_quota_statv res_qstatv = {
		.qs_version = FS_QSTATV_VERSION1
	};

	res = do_quotactl(fd, subcmd, tst_device->dev, test_id, (void *) &res_qstatv);
	if (res == -1) {
		tst_res(TFAIL | TERRNO,
			"quotactl() failed to get xfs quota on statv");
		return;
	}

	if (!(res_qstatv.qs_flags & flag)) {
		tst_res(TFAIL, "xfs quota enforcement was off unexpectedly");
		return;
	}

	tst_res(TPASS, "quotactl() succeeded to %s", desp);
}

static void check_qlim(int subcmd, char *desp)
{
	int res;
	static struct fs_disk_quota res_dquota;

	res_dquota.d_rtb_softlimit = 0;

	res = do_quotactl(fd, subcmd, tst_device->dev, test_id, (void *) &res_dquota);
	if (res == -1) {
		tst_res(TFAIL | TERRNO,
			"quotactl() failed to get xfs disk quota limits");
		return;
	}

	if (res_dquota.d_id != test_id) {
		tst_res(TFAIL, "quotactl() got unexpected user id %u, expected %u",
			res_dquota.d_id, test_id);
		return;
	}

	if (res_dquota.d_rtb_hardlimit != set_dquota.d_rtb_hardlimit) {
		tst_res(TFAIL, "quotactl() got unexpected rtb soft limit %llu, expected %llu",
				res_dquota.d_rtb_hardlimit, set_dquota.d_rtb_hardlimit);
		return;
	}

	tst_res(TPASS, "quotactl() succeeded to set and use %s to get xfs disk quota limits",
			desp);
}
#endif /* HAVE_XFS_XQM_H */
#endif /* QUOTACTL02_H */
