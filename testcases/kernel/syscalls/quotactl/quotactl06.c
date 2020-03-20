// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 *
 * Tests basic error handling of the quotactl syscall.
 * 1) quotactl fails with EACCES when cmd is Q_QUOTAON and addr
 * existed but not a regular file.
 * 2) quotaclt fails with ENOENT when the file specified by special
 * or addr does not exist.
 * 3) quotactl fails with EBUSTY when  cmd is Q_QUOTAON and another
 * Q_QUOTAON had already been performed.
 * 4) quotactl fails with EFAULT when addr or special is invalid.
 * 5) quotactl fails with EINVAL when cmd or type is invalid.
 * 6) quotactl fails with ENOTBLK when special is not a block device.
 * 7) quotactl fails with ESRCH when no disk quota is found for the
 * indicated user and quotas have not been turned on for this fs.
 * 8) quotactl fails with ESRCH when cmd is Q_QUOTAON, but the quota
 * format was not found.
 * 9) quotactl fails with ESRCH when cmd is Q_GETNEXTQUOTA, but there
 * is no ID greater than or equal to id that has an active quota.
 * 10) quotactl fails with ERANGE when cmd is Q_SETQUOTA, but the
 * specified limits are out of the range allowed by the quota format.
 * 11) quotactl fails with EPERM when the caller lacked the required
 * privilege (CAP_SYS_ADMIN) for the specified operation.
 */

#include <errno.h>
#include <sys/quota.h>
#include "tst_test.h"
#include "lapi/quotactl.h"
#include "tst_capability.h"

#define OPTION_INVALID 999
#define QFMT_VFS_V0     2
#define USRPATH MNTPOINT "/aquota.user"
#define FMTID QFMT_VFS_V0

#define MNTPOINT "mntpoint"
#define TESTDIR1 MNTPOINT "/testdir1"
#define TESTDIR2 MNTPOINT "/testdir2"

static int32_t fmt_id = FMTID;
static int32_t fmt_invalid = 999;
static int test_invalid;
static int test_id;
static int getnextquota_nsup;

static struct if_nextdqblk res_ndq;
static struct dqblk set_dq = {
	.dqb_bsoftlimit = 100,
	.dqb_valid = QIF_BLIMITS
};

static struct dqblk set_dqmax = {
	.dqb_bsoftlimit = 0x7fffffffffffffffLL,  /* 2^63-1 */
	.dqb_valid = QIF_BLIMITS
};

struct tst_cap dropadmin = {
	.action = TST_CAP_DROP,
	.id = CAP_SYS_ADMIN,
	.name = "CAP_SYS_ADMIN",
};

struct tst_cap needadmin = {
	.action = TST_CAP_REQ,
	.id = CAP_SYS_ADMIN,
	.name = "CAP_SYS_ADMIN",
};

static struct tcase {
	int cmd;
	int *id;
	void *addr;
	int exp_err;
	int on_flag;
} tcases[] = {
	{QCMD(Q_QUOTAON, USRQUOTA), &fmt_id, TESTDIR1, EACCES, 0},
	{QCMD(Q_QUOTAON, USRQUOTA), &fmt_id, TESTDIR2, ENOENT, 0},
	{QCMD(Q_QUOTAON, USRQUOTA), &fmt_id, USRPATH, EBUSY, 1},
	{QCMD(Q_SETQUOTA, USRQUOTA), &fmt_id, NULL, EFAULT, 1},
	{QCMD(OPTION_INVALID, USRQUOTA), &fmt_id, USRPATH, EINVAL, 0},
	{QCMD(Q_QUOTAON, USRQUOTA), &fmt_id, USRPATH, ENOTBLK, 0},
	{QCMD(Q_SETQUOTA, USRQUOTA), &test_id, &set_dq, ESRCH, 0},
	{QCMD(Q_QUOTAON, USRQUOTA), &fmt_invalid, USRPATH, ESRCH, 0},
	{QCMD(Q_GETNEXTQUOTA, USRQUOTA), &test_invalid, USRPATH, ESRCH, 0},
	{QCMD(Q_SETQUOTA, USRQUOTA), &test_id, &set_dqmax, ERANGE, 1},
	{QCMD(Q_QUOTAON, USRQUOTA), &fmt_id, USRPATH, EPERM, 0},
};

static void verify_quotactl(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int quota_on = 0;
	int drop_flag = 0;

	if (tc->cmd == QCMD(Q_GETNEXTQUOTA, USRQUOTA) && getnextquota_nsup) {
		tst_res(TCONF, "current system doesn't support Q_GETNEXTQUOTA");
		return;
	}

	if (tc->on_flag) {
		TEST(quotactl(QCMD(Q_QUOTAON, USRQUOTA), tst_device->dev, FMTID, USRPATH));
		if (TST_RET == -1)
			tst_brk(TBROK,
				"quotactl with Q_QUOTAON returned %ld", TST_RET);
		quota_on = 1;
	}

	if (tc->exp_err == EPERM) {
		tst_cap_action(&dropadmin);
		drop_flag = 1;
	}

	if (tc->exp_err == ENOTBLK)
		TEST(quotactl(tc->cmd, "/dev/null", *tc->id, tc->addr));
	else
		TEST(quotactl(tc->cmd, tst_device->dev, *tc->id, tc->addr));
	if (TST_RET == -1) {
		if (tc->exp_err == TST_ERR) {
			tst_res(TPASS | TTERRNO, "quotactl failed as expected");
		} else {
			tst_res(TFAIL | TTERRNO,
				"quotactl failed unexpectedly; expected %s, but got",
				tst_strerrno(tc->exp_err));
		}
	} else {
		tst_res(TFAIL, "quotactl returned wrong value: %ld", TST_RET);
	}

	if (quota_on) {
		TEST(quotactl(QCMD(Q_QUOTAOFF, USRQUOTA), tst_device->dev, FMTID, USRPATH));
		if (TST_RET == -1)
			tst_brk(TBROK,
				"quotactl with Q_QUOTAOFF returned %ld", TST_RET);
		quota_on = 0;
	}

	if (drop_flag) {
		tst_cap_action(&needadmin);
		drop_flag = 0;
	}
}

static void setup(void)
{
	const char *const cmd[] = {"quotacheck", "-uF", "vfsv0", MNTPOINT, NULL};
	unsigned int i;

	SAFE_CMD(cmd, NULL, NULL);

	if (access(USRPATH, F_OK) == -1)
		tst_brk(TFAIL | TERRNO, "user quotafile didn't exist");

	SAFE_MKDIR(TESTDIR1, 0666);
	test_id = geteuid();
	test_invalid = test_id + 1;

	TEST(quotactl(QCMD(Q_GETNEXTQUOTA, USRQUOTA), tst_device->dev,
		test_id, (void *) &res_ndq));
	if (TST_ERR == EINVAL || TST_ERR == ENOSYS)
		getnextquota_nsup = 1;

	for (i = 0; i < ARRAY_SIZE(tcases); i++) {
		if (!tcases[i].addr)
			tcases[i].addr = tst_get_bad_addr(NULL);
	}
}

static const char *kconfigs[] = {
	"CONFIG_QFMT_V2",
	NULL
};

static struct tst_test test = {
	.setup = setup,
	.needs_kconfigs = kconfigs,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_quotactl,
	.dev_fs_type = "ext4",
	.mntpoint = MNTPOINT,
	.mount_device = 1,
	.mnt_data = "usrquota",
	.needs_cmds = (const char *const []) {
		"quotacheck",
		NULL
	},
	.needs_root = 1,
};
