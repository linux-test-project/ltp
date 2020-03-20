// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Copyright (c) 2016-2019 FUJITSU LIMITED. All rights reserved
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 *
 * This testcase checks the basic flag of quotactl(2) for non-XFS filesystems:
 * 1) quotactl(2) succeeds to turn on quota with Q_QUOTAON flag for user.
 * 2) quotactl(2) succeeds to set disk quota limits with Q_SETQUOTA flag
 *    for user.
 * 3) quotactl(2) succeeds to get disk quota limits with Q_GETQUOTA flag
 *    for user.
 * 4) quotactl(2) succeeds to set information about quotafile with Q_SETINFO
 *    flag for user.
 * 5) quotactl(2) succeeds to get information about quotafile with Q_GETINFO
 *    flag for user.
 * 6) quotactl(2) succeeds to get quota format with Q_GETFMT flag for user.
 * 7) quotactl(2) succeeds to update quota usages with Q_SYNC flag for user.
 * 8) quotactl(2) succeeds to get disk quota limit greater than or equal to
 *    ID with Q_GETNEXTQUOTA flag for user.
 * 9) quotactl(2) succeeds to turn off quota with Q_QUOTAOFF flag for user.
 * 10) quotactl(2) succeeds to turn on quota with Q_QUOTAON flag for group.
 * 11) quotactl(2) succeeds to set disk quota limits with Q_SETQUOTA flag
 *     for group.
 * 12) quotactl(2) succeeds to get disk quota limits with Q_GETQUOTA flag
 *     for group.
 * 13) quotactl(2) succeeds to set information about quotafile with Q_SETINFO
 *     flag for group.
 * 14) quotactl(2) succeeds to get information about quotafile with Q_GETINFO
 *     flag for group.
 * 15) quotactl(2) succeeds to get quota format with Q_GETFMT flag for group.
 * 16) quotactl(2) succeeds to update quota usages with Q_SYNC flag for group.
 * 17) quotactl(2) succeeds to get disk quota limit greater than or equal to
 *     ID with Q_GETNEXTQUOTA flag for group.
 * 18) quotactl(2) succeeds to turn off quota with Q_QUOTAOFF flag for group.
 */

#include "config.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "lapi/quotactl.h"
#include "tst_test.h"

#ifndef QFMT_VFS_V0
# define QFMT_VFS_V0	2
#endif
#define USRPATH MNTPOINT "/aquota.user"
#define GRPPATH MNTPOINT "/aquota.group"
#define FMTID	QFMT_VFS_V0
#define MNTPOINT	"mntpoint"

static int32_t fmt_id = FMTID;
static int test_id;
static struct dqblk set_dq = {
	.dqb_bsoftlimit = 100,
	.dqb_valid = QIF_BLIMITS
};
static struct dqblk res_dq;

static struct dqinfo set_qf = {
	.dqi_bgrace = 80,
	.dqi_valid = IIF_BGRACE
};
static struct dqinfo res_qf;
static int32_t fmt_buf;
static int getnextquota_nsup;

static struct if_nextdqblk res_ndq;

static struct tcase {
	int cmd;
	int *id;
	void *addr;
	void *set_data;
	void *res_data;
	int sz;
	char *des;
	char *tname;
} tcases[] = {
	{QCMD(Q_QUOTAON, USRQUOTA), &fmt_id, USRPATH,
	NULL, NULL, 0, "turn on quota for user",
	"QCMD(Q_QUOTAON, USRQUOTA)"},

	{QCMD(Q_SETQUOTA, USRQUOTA), &test_id, &set_dq,
	NULL, NULL, 0, "set disk quota limit for user",
	"QCMD(Q_SETQUOTA, USRQUOTA)"},

	{QCMD(Q_GETQUOTA, USRQUOTA), &test_id, &res_dq,
	&set_dq.dqb_bsoftlimit, &res_dq.dqb_bsoftlimit,
	sizeof(res_dq.dqb_bsoftlimit), "get disk quota limit for user",
	"QCMD(Q_GETQUOTA, USRQUOTA)"},

	{QCMD(Q_SETINFO, USRQUOTA), &test_id, &set_qf,
	NULL, NULL, 0, "set information about quotafile for user",
	"QCMD(Q_SETINFO, USRQUOTA)"},

	{QCMD(Q_GETINFO, USRQUOTA), &test_id, &res_qf,
	&set_qf.dqi_bgrace, &res_qf.dqi_bgrace, sizeof(res_qf.dqi_bgrace),
	"get information about quotafile for user",
	"QCMD(Q_GETINFO, USRQUOTA)"},

	{QCMD(Q_GETFMT, USRQUOTA), &test_id, &fmt_buf,
	&fmt_id, &fmt_buf, sizeof(fmt_buf),
	"get quota format for user",
	"QCMD(Q_GETFMT, USRQUOTA)"},

	{QCMD(Q_SYNC, USRQUOTA), &test_id, &res_dq,
	NULL, NULL, 0, "update quota usages for user",
	"QCMD(Q_SYNC, USRQUOTA)"},

	{QCMD(Q_GETNEXTQUOTA, USRQUOTA), &test_id, &res_ndq,
	&test_id, &res_ndq.dqb_id, sizeof(res_ndq.dqb_id),
	"get next disk quota limit for user",
	"QCMD(Q_GETNEXTQUOTA, USRQUOTA)"},

	{QCMD(Q_QUOTAOFF, USRQUOTA), &test_id, USRPATH,
	NULL, NULL, 0, "turn off quota for user",
	"QCMD(Q_QUOTAOFF, USRQUOTA)"},

	{QCMD(Q_QUOTAON, GRPQUOTA), &fmt_id, GRPPATH,
	NULL, NULL, 0, "turn on quota for group",
	"QCMD(Q_QUOTAON, GRPQUOTA)"},

	{QCMD(Q_SETQUOTA, GRPQUOTA), &test_id, &set_dq,
	NULL, NULL, 0, "set disk quota limit for group",
	"QCMD(Q_SETQUOTA, GRPQUOTA)"},

	{QCMD(Q_GETQUOTA, GRPQUOTA), &test_id, &res_dq, &set_dq.dqb_bsoftlimit,
	&res_dq.dqb_bsoftlimit, sizeof(res_dq.dqb_bsoftlimit),
	"set disk quota limit for group",
	"QCMD(Q_GETQUOTA, GRPQUOTA)"},

	{QCMD(Q_SETINFO, GRPQUOTA), &test_id, &set_qf,
	NULL, NULL, 0, "set information about quotafile for group",
	"QCMD(Q_SETINFO, GRPQUOTA)"},

	{QCMD(Q_GETINFO, GRPQUOTA), &test_id, &res_qf, &set_qf.dqi_bgrace,
	&res_qf.dqi_bgrace, sizeof(res_qf.dqi_bgrace),
	"get information about quotafile for group",
	"QCMD(Q_GETINFO, GRPQUOTA)"},

	{QCMD(Q_GETFMT, GRPQUOTA), &test_id, &fmt_buf,
	&fmt_id, &fmt_buf, sizeof(fmt_buf), "get quota format for group",
	"QCMD(Q_GETFMT, GRPQUOTA)"},

	{QCMD(Q_SYNC, GRPQUOTA), &test_id, &res_dq,
	NULL, NULL, 0, "update quota usages for group",
	"QCMD(Q_SYNC, GRPQUOTA)"},

	{QCMD(Q_GETNEXTQUOTA, GRPQUOTA), &test_id, &res_ndq,
	&test_id, &res_ndq.dqb_id, sizeof(res_ndq.dqb_id),
	"get next disk quota limit for group",
	"QCMD(Q_GETNEXTQUOTA, GRPQUOTA)"},

	{QCMD(Q_QUOTAOFF, GRPQUOTA), &test_id, GRPPATH,
	NULL, NULL, 0, "turn off quota for group",
	"QCMD(Q_QUOTAOFF, GRPQUOTA)"},
};

static void setup(void)
{
	const char *const cmd[] = {"quotacheck", "-ugF", "vfsv0", MNTPOINT, NULL};

	SAFE_CMD(cmd, NULL, NULL);

	test_id = geteuid();
	if (access(USRPATH, F_OK) == -1)
		tst_brk(TFAIL | TERRNO, "user quotafile didn't exist");

	if (access(GRPPATH, F_OK) == -1)
		tst_brk(TFAIL | TERRNO, "group quotafile didn't exist");

	TEST(quotactl(QCMD(Q_GETNEXTQUOTA, USRQUOTA), tst_device->dev,
		test_id, (void *) &res_ndq));
	if (TST_ERR == EINVAL || TST_ERR == ENOSYS)
		getnextquota_nsup = 1;
}

static void verify_quota(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	res_dq.dqb_bsoftlimit = 0;
	res_qf.dqi_igrace = 0;
	fmt_buf = 0;
	res_ndq.dqb_id = -1;

	tst_res(TINFO, "Test #%d: %s", n, tc->tname);
	if ((tc->cmd == QCMD(Q_GETNEXTQUOTA, USRQUOTA) ||
		tc->cmd == QCMD(Q_GETNEXTQUOTA, GRPQUOTA)) &&
		getnextquota_nsup) {
		tst_res(TCONF, "current system doesn't support this cmd");
		return;
	}
	TEST(quotactl(tc->cmd, tst_device->dev, *tc->id, tc->addr));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "quotactl failed to %s", tc->des);
		return;
	}

	if (memcmp(tc->res_data, tc->set_data, tc->sz)) {
		tst_res(TFAIL, "quotactl failed to %s", tc->des);
		tst_res_hexd(TINFO, tc->res_data, tc->sz, "retval:   ");
		tst_res_hexd(TINFO, tc->set_data, tc->sz, "expected: ");
		return;
	}

	tst_res(TPASS, "quotactl succeeded to %s", tc->des);
}

static const char *kconfigs[] = {
	"CONFIG_QFMT_V2",
	NULL
};

static struct tst_test test = {
	.needs_root = 1,
	.needs_kconfigs = kconfigs,
	.test = verify_quota,
	.tcnt = ARRAY_SIZE(tcases),
	.mount_device = 1,
	.dev_fs_type = "ext4",
	.mntpoint = MNTPOINT,
	.mnt_data = "usrquota,grpquota",
	.needs_cmds = (const char *const []) {
		"quotacheck",
		NULL
	},
	.setup = setup,
};
