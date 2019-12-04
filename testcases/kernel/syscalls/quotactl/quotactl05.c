// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 *
 * This testcase checks basic flags of quotactl(2) for project on an XFS file
 * system:
 * 1) quotactl(2) succeeds to turn off xfs quota and get xfs quota off status
 *    for project.
 * 2) quotactl(2) succeeds to turn on xfs quota and get xfs quota on status
 *    for project.
 * 3) quotactl(2) succeeds to set and use Q_XGETQUOTA to get xfs disk quota
 *    limits for project.
 * 4) quotactl(2) succeeds to set and use Q_XGETNEXTQUOTA to get xfs disk
 *    quota limits Cgreater than or equal to ID for project.
 * 5) quotactl(2) succeeds to turn off xfs quota and get xfs quota off statv
 *    for project.
 * 6) quotactl(2) succeeds to turn on xfs quota and get xfs quota on statv
 *    for project.
 */
#include "quotactl02.h"
#if defined(HAVE_XFS_XQM_H)

static uint32_t qflagp = XFS_QUOTA_PDQ_ENFD;
static struct t_case {
	int cmd;
	void *addr;
	void (*func_check)();
	int check_subcmd;
	int flag;
	char *des;
	char *tname;
} tcases[] = {
	{QCMD(Q_XQUOTAOFF, PRJQUOTA), &qflagp, check_qoff,
	QCMD(Q_XGETQSTAT, PRJQUOTA), 1,
	"turn off xfs quota and get xfs quota off status for project",
	"QCMD(Q_XGETQSTAT, PRJQUOTA) off"},

	{QCMD(Q_XQUOTAON, PRJQUOTA), &qflagp, check_qon,
	QCMD(Q_XGETQSTAT, PRJQUOTA), 1,
	"turn on xfs quota and get xfs quota on status for project",
	"QCMD(Q_XGETQSTAT, PRJQUOTA) on"},

	{QCMD(Q_XSETQLIM, PRJQUOTA), &set_dquota, check_qlim,
	QCMD(Q_XGETQUOTA, PRJQUOTA), 0,
	"Q_XGETQUOTA for project", "QCMD(Q_XGETQUOTA, PRJQUOTA) qlim"},

	{QCMD(Q_XSETQLIM, PRJQUOTA), &set_dquota, check_qlim,
	QCMD(Q_XGETNEXTQUOTA, PRJQUOTA), 0,
	"Q_XGETNEXTQUOTA for project", "QCMD(Q_XGETNEXTQUOTA, PRJQUOTA)"},

	{QCMD(Q_XQUOTAOFF, PRJQUOTA), &qflagp, check_qoffv,
	QCMD(Q_XGETQSTATV, PRJQUOTA), 1,
	"turn off xfs quota and get xfs quota off statv for project",
	"QCMD(Q_XGETQSTATV, PRJQUOTA) off"},

	{QCMD(Q_XQUOTAON, PRJQUOTA), &qflagp, check_qonv,
	QCMD(Q_XGETQSTATV, PRJQUOTA), 1,
	"turn on xfs quota and get xfs quota on statv for project",
	"QCMD(Q_XGETQSTATV, PRJQUOTA) on"},
};

static void setup(void)
{
	test_id = geteuid();
	check_support_cmd(PRJQUOTA);
}

static void verify_quota(unsigned int n)
{
	struct t_case *tc = &tcases[n];

	tst_res(TINFO, "Test #%d: %s", n, tc->tname);

	if ((tc->check_subcmd == QCMD(Q_XGETNEXTQUOTA, PRJQUOTA))
		&& x_getnextquota_nsup) {
		tst_res(TCONF,
			"current system doesn't support this cmd");
		return;
	}
	if ((tc->check_subcmd == QCMD(Q_XGETQSTATV, PRJQUOTA))
		&& x_getstatv_nsup) {
		tst_res(TCONF,
			"current system doesn't support this cmd");
		return;
	}

	TEST(quotactl(tc->cmd, tst_device->dev, test_id, tc->addr));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "quotactl() failed to %s", tc->des);
		return;
	}

	if (tc->flag)
		tc->func_check(tc->check_subcmd, tc->des, *(int *)(tc->addr));
	else
		tc->func_check(tc->check_subcmd, tc->des);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_kconfigs = kconfigs,
	.test = verify_quota,
	.tcnt = ARRAY_SIZE(tcases),
	.mount_device = 1,
	.dev_fs_type = "xfs",
	.mntpoint = mntpoint,
	.mnt_data = "prjquota",
	.setup = setup,
};

#else
	TST_TEST_TCONF("This system didn't have <xfs/xqm.h>");
#endif
