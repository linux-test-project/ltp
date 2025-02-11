// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Copyright (c) 2016-2021 FUJITSU LIMITED. All rights reserved
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * This testcases checks that quotactl(2) on ext4 filesystem succeeds to:
 *
 * - turn on quota with Q_QUOTAON flag for user
 * - set disk quota limits with Q_SETQUOTA flag for user
 * - get disk quota limits with Q_GETQUOTA flag for user
 * - set information about quotafile with Q_SETINFO flag for user
 * - get information about quotafile with Q_GETINFO flag for user
 * - get quota format with Q_GETFMT flag for user
 * - update quota usages with Q_SYNC flag for user
 * - get disk quota limit greater than or equal to ID with Q_GETNEXTQUOTA flag for user
 * - turn off quota with Q_QUOTAOFF flag for user
 * - turn on quota with Q_QUOTAON flag for group
 * - set disk quota limits with Q_SETQUOTA flag for group
 * - get disk quota limits with Q_GETQUOTA flag for group
 * - set information about quotafile with Q_SETINFO flag for group
 * - get information about quotafile with Q_GETINFO flag for group
 * - get quota format with Q_GETFMT flag for group
 * - update quota usages with Q_SYNC flag for group
 * - get disk quota limit greater than or equal to ID with Q_GETNEXTQUOTA flag for group
 * - turn off quota with Q_QUOTAOFF flag for group
 */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "tst_test.h"
#include "quotactl_fmt_var.h"

#define USRPATH MNTPOINT "/aquota.user"
#define GRPPATH MNTPOINT "/aquota.group"
#define MNTPOINT	"mntpoint"

static int32_t fmt_id;
static int test_id;
static char usrpath[] = USRPATH;
static char grppath[] = GRPPATH;
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
	{QCMD(Q_QUOTAON, USRQUOTA), &fmt_id, usrpath,
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

	{QCMD(Q_QUOTAOFF, USRQUOTA), &test_id, usrpath,
	NULL, NULL, 0, "turn off quota for user",
	"QCMD(Q_QUOTAOFF, USRQUOTA)"},

	{QCMD(Q_QUOTAON, GRPQUOTA), &fmt_id, grppath,
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

	{QCMD(Q_QUOTAOFF, GRPQUOTA), &test_id, grppath,
	NULL, NULL, 0, "turn off quota for group",
	"QCMD(Q_QUOTAOFF, GRPQUOTA)"},
};

static void setup(void)
{
	const struct quotactl_fmt_variant *var = &fmt_variants[tst_variant];
	const char *const cmd[] = {"quotacheck", "-ugF", var->fmt_name, MNTPOINT, NULL};

	tst_res(TINFO, "quotactl() with %s format", var->fmt_name);
	SAFE_CMD(cmd, NULL, NULL);
	fmt_id = var->fmt_id;

	SAFE_ACCESS(USRPATH, F_OK);

	SAFE_ACCESS(GRPPATH, F_OK);

	TEST(quotactl(QCMD(Q_GETNEXTQUOTA, USRQUOTA), tst_device->dev,
		test_id, (void *) &res_ndq));
	if (TST_ERR == EINVAL || TST_ERR == ENOSYS)
		getnextquota_nsup = 1;
}

static void cleanup(void)
{
	SAFE_UNLINK(USRPATH);
	SAFE_UNLINK(GRPPATH);
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
	TST_EXP_PASS_SILENT(quotactl(tc->cmd, tst_device->dev, *tc->id, tc->addr),
			"quotactl to %s", tc->des);
	if (!TST_PASS)
		return;

	if (memcmp(tc->res_data, tc->set_data, tc->sz)) {
		tst_res(TFAIL, "quotactl failed to %s", tc->des);
		tst_res_hexd(TINFO, tc->res_data, tc->sz, "retval:   ");
		tst_res_hexd(TINFO, tc->set_data, tc->sz, "expected: ");
		return;
	}

	tst_res(TPASS, "quotactl succeeded to %s", tc->des);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_drivers = (const char *const []) {
		"quota_v2",
		NULL
	},
	.test = verify_quota,
	.tcnt = ARRAY_SIZE(tcases),
	.mount_device = 1,
	.filesystems = (struct tst_fs []) {
		{
			.type = "ext4",
			.mnt_data = "usrquota,grpquota",
		},
		{}
	},
	.mntpoint = MNTPOINT,
	.needs_cmds = (const char *const []) {
		"quotacheck",
		NULL
	},
	.setup = setup,
	.cleanup = cleanup,
	.test_variants = QUOTACTL_FMT_VARIANTS,
};
