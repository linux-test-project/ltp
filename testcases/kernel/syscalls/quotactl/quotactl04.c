// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019-2021 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * This testcase checks that quotactl(2) on ext4 filesystem succeeds to:
 *
 * - turn on quota with Q_QUOTAON flag for project
 * - set disk quota limits with Q_SETQUOTA flag for project
 * - get disk quota limits with Q_GETQUOTA flag for project
 * - set information about quotafile with Q_SETINFO flag for project
 * - get information about quotafile with Q_GETINFO flag for project
 * - get quota format with Q_GETFMT flag for project
 * - get disk quota limit greater than or equal to ID with Q_GETNEXTQUOTA flag for project
 * - turn off quota with Q_QUOTAOFF flag for project
 *
 * Minimum e2fsprogs version required is 1.43.
 */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include "tst_test.h"
#include "quotactl_syscall_var.h"

#define FMTID QFMT_VFS_V1

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

static struct if_nextdqblk res_ndq;
static int getnextquota_nsup;

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
	{QCMD(Q_QUOTAON, PRJQUOTA), &fmt_id, NULL,
	NULL, NULL, 0, "turn on quota for project",
	"QCMD(Q_QUOTAON, PRJQUOTA)"},

	{QCMD(Q_SETQUOTA, PRJQUOTA), &test_id, &set_dq,
	NULL, NULL, 0, "set disk quota limit for project",
	"QCMD(Q_SETQUOTA, PRJQUOTA)"},

	{QCMD(Q_GETQUOTA, PRJQUOTA), &test_id, &res_dq,
	&set_dq.dqb_bsoftlimit, &res_dq.dqb_bsoftlimit,
	sizeof(res_dq.dqb_bsoftlimit), "get disk quota limit for project",
	"QCMD(Q_GETQUOTA, PRJQUOTA)"},

	{QCMD(Q_SETINFO, PRJQUOTA), &test_id, &set_qf,
	NULL, NULL, 0, "set information about quotafile for project",
	"QCMD(Q_SETINFO, PRJQUOTA"},

	{QCMD(Q_GETINFO, PRJQUOTA), &test_id, &res_qf,
	&set_qf.dqi_bgrace, &res_qf.dqi_bgrace, sizeof(res_qf.dqi_bgrace),
	"get information about quotafile for project",
	"QCMD(Q_GETINFO, PRJQUOTA"},

	{QCMD(Q_GETFMT, PRJQUOTA), &test_id, &fmt_buf,
	&fmt_id, &fmt_buf, sizeof(fmt_buf),
	"get quota format for project", "QCMD(Q_GETFMT, PRJQUOTA)"},

	{QCMD(Q_GETNEXTQUOTA, PRJQUOTA), &test_id, &res_ndq,
	&test_id, &res_ndq.dqb_id, sizeof(res_ndq.dqb_id),
	"get next disk quota limit for project",
	"QCMD(Q_GETNEXTQUOTA, PRJQUOTA)"},

	{QCMD(Q_QUOTAOFF, PRJQUOTA), &test_id, NULL,
	NULL, NULL, 0, "turn off quota for project",
	"QCMD(Q_QUOTAOFF, PRJQUOTA)"},

};

static void setup(void)
{
	quotactl_info();

	fd = SAFE_OPEN(MNTPOINT, O_RDONLY);

	TEST(do_quotactl(fd, QCMD(Q_GETNEXTQUOTA, PRJQUOTA), tst_device->dev,
		test_id, (void *) &res_ndq));
	if (TST_ERR == EINVAL || TST_ERR == ENOSYS)
		getnextquota_nsup = 1;
}

static void cleanup(void)
{
	if (fd > -1)
		SAFE_CLOSE(fd);
}

static void verify_quota(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	res_dq.dqb_bsoftlimit = 0;
	res_qf.dqi_igrace = 0;
	fmt_buf = 0;

	tst_res(TINFO, "Test #%d: %s", n, tc->tname);

	if (tc->cmd == QCMD(Q_GETNEXTQUOTA, PRJQUOTA) && getnextquota_nsup) {
		tst_res(TCONF, "current system doesn't support this cmd");
		return;
	}

	TST_EXP_PASS_SILENT(do_quotactl(fd, tc->cmd, tst_device->dev, *tc->id, tc->addr),
			"do_quotactl to %s", tc->des);
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
	.min_kver = "4.5", /* commit 689c958cbe6b (ext4: add project quota support) */
	.test = verify_quota,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.mount_device = 1,
	.filesystems = (struct tst_fs []) {
		{
			.type = "ext4",
			.mkfs_opts = (const char *const[]) {
				"-I 256", "-O quota,project", NULL
			},
		},
		{}
	},
	.mntpoint = MNTPOINT,
	.test_variants = QUOTACTL_SYSCALL_VARIANTS,
	.needs_cmds = (struct tst_cmd[]) {
		{.cmd = "mkfs.ext4 >= 1.43.0"},
		{}
	}
};
