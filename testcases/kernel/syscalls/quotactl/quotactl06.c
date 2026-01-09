// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019-2021 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * Tests basic error handling of the quotactl syscall with visible quota files
 * (cover two formats, vfsv0 and vfsv1):
 *
 * - EACCES when cmd is Q_QUOTAON and addr existed but not a regular file
 * - ENOENT when the file specified by special or addr does not exist
 * - EBUSY when cmd is Q_QUOTAON and another Q_QUOTAON had already been
 *   performed
 * - EFAULT when addr or special is invalid
 * - EINVAL when cmd or type is invalid
 * - ENOTBLK when special is not a block device
 * - ESRCH when no disk quota is found for the indicated user and quotas have
 *   not been turned on for this fs
 * - ESRCH when cmd is Q_QUOTAON, but the quota format was not found
 * - ESRCH when cmd is Q_GETNEXTQUOTA, but there is no ID greater than or
 *   equal to id that has an active quota
 * - ERANGE when cmd is Q_SETQUOTA, but the specified limits are out of the
 *   range allowed by the quota format
 * - EPERM when the caller lacked the required privilege (CAP_SYS_ADMIN) for
 *   the specified operation
 *
 * For ERANGE error, the vfsv0 and vfsv1 format's maximum quota limit setting
 * have been fixed since the following kernel patch:
 *
 *  commit 7e08da50cf706151f324349f9235ebd311226997
 *  Author: Jan Kara <jack@suse.cz>
 *  Date:   Wed Mar 4 14:42:02 2015 +0100
 *
 *  quota: Fix maximum quota limit settings
 */

#include <errno.h>
#include <sys/quota.h>
#include "tst_test.h"
#include "quotactl_fmt_var.h"
#include "tst_capability.h"

#define OPTION_INVALID 999
#define USRPATH MNTPOINT "/aquota.user"
#define MNTPOINT "mntpoint"
#define TESTDIR1 MNTPOINT "/testdir1"
#define TESTDIR2 MNTPOINT "/testdir2"

static char usrpath[] = USRPATH;
static char testdir1[] = TESTDIR1;
static char testdir2[] = TESTDIR2;
static int32_t fmt_id;
static int32_t fmt_invalid = 999;
static int test_invalid = 1;
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

static struct tst_cap dropadmin = {
	.action = TST_CAP_DROP,
	.id = CAP_SYS_ADMIN,
	.name = "CAP_SYS_ADMIN",
};

static struct tst_cap needadmin = {
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
	char *des;
} tcases[] = {
	{QCMD(Q_QUOTAON, USRQUOTA), &fmt_id, testdir1, EACCES, 0,
	"EACCES when cmd is Q_QUOTAON and addr existed but not a regular file"},

	{QCMD(Q_QUOTAON, USRQUOTA), &fmt_id, testdir2, ENOENT, 0,
	"ENOENT when the file specified by special or addr does not exist"},

	{QCMD(Q_QUOTAON, USRQUOTA), &fmt_id, usrpath, EBUSY, 1,
	"EBUSY when cmd is Q_QUOTAON and another Q_QUOTAON had already been performed"},

	{QCMD(Q_SETQUOTA, USRQUOTA), &fmt_id, NULL, EFAULT, 1,
	"EFAULT when addr or special is invalid"},

	{QCMD(OPTION_INVALID, USRQUOTA), &fmt_id, usrpath, EINVAL, 0,
	"EINVAL when cmd or type is invalid"},

	{QCMD(Q_QUOTAON, USRQUOTA), &fmt_id, usrpath, ENOTBLK, 0,
	"ENOTBLK when special is not a block device"},

	{QCMD(Q_SETQUOTA, USRQUOTA), &test_id, &set_dq, ESRCH, 0,
	"ESRCH is for Q_SETQUOTA but no quota found for the user or quotas are off"},

	{QCMD(Q_QUOTAON, USRQUOTA), &fmt_invalid, usrpath, ESRCH, 0,
	"ESRCH when cmd is Q_QUOTAON, but the quota format was not found"},

	{QCMD(Q_GETNEXTQUOTA, USRQUOTA), &test_invalid, usrpath, ESRCH, 0,
	"ESRCH for Q_GETNEXTQUOTA, but the id was last one"},

	{QCMD(Q_SETQUOTA, USRQUOTA), &test_id, &set_dqmax, ERANGE, 1,
	"ERANGE for Q_SETQUOTA, but the specified limits are out of range"},

	{QCMD(Q_QUOTAON, USRQUOTA), &fmt_id, usrpath, EPERM, 0,
	"EPERM when the caller lacks the required privilege (CAP_SYS_ADMIN)"},
};

static void verify_quotactl(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int quota_on = 0;
	int drop_flag = 0;

	tst_res(TINFO, "Testing %s", tc->des);
	if (tc->cmd == QCMD(Q_GETNEXTQUOTA, USRQUOTA) && getnextquota_nsup) {
		tst_res(TCONF, "current system doesn't support Q_GETNEXTQUOTA");
		return;
	}

	if (tc->on_flag) {
		TST_EXP_PASS_SILENT(quotactl(QCMD(Q_QUOTAON, USRQUOTA),
			tst_device->dev, fmt_id, usrpath),
			"quotactl with Q_QUOTAON");

		if (!TST_PASS)
			return;

		quota_on = 1;
	}

	if (tc->exp_err == EPERM) {
		tst_cap_action(&dropadmin);
		drop_flag = 1;
	}

	if (tc->exp_err == ENOTBLK) {
		TST_EXP_FAIL(quotactl(tc->cmd, "/dev/null", *tc->id, tc->addr),
			ENOTBLK, "quotactl()");
	} else {
		TST_EXP_FAIL(quotactl(tc->cmd, tst_device->dev, *tc->id,
			tc->addr), tc->exp_err, "quotactl()");
	}

	if (quota_on) {
		TST_EXP_PASS_SILENT(quotactl(QCMD(Q_QUOTAOFF, USRQUOTA),
			tst_device->dev, fmt_id, usrpath),
			"quotactl with Q_QUOTAOFF");

		if (!TST_PASS)
			return;
	}

	if (drop_flag)
		tst_cap_action(&needadmin);
}

static void setup(void)
{
	unsigned int i;
	const struct quotactl_fmt_variant *var = &fmt_variants[tst_variant];
	const char *const cmd[] = {
		"quotacheck", "-ugF", var->fmt_name, MNTPOINT, NULL
	};

	tst_res(TINFO, "quotactl() with %s format", var->fmt_name);
	SAFE_CMD(cmd, NULL, NULL);
	fmt_id = var->fmt_id;
	/* vfsv0 block limit 2^42, vfsv1 block limit 2^63 - 1 */
	set_dqmax.dqb_bsoftlimit = tst_variant ? 0x20000000000000 : 0x100000000;

	SAFE_ACCESS(USRPATH, F_OK);

	SAFE_MKDIR(TESTDIR1, 0666);

	TEST(quotactl(QCMD(Q_GETNEXTQUOTA, USRQUOTA), tst_device->dev,
		test_id, (void *) &res_ndq));
	if (TST_ERR == EINVAL || TST_ERR == ENOSYS)
		getnextquota_nsup = 1;

	for (i = 0; i < ARRAY_SIZE(tcases); i++) {
		if (!tcases[i].addr)
			tcases[i].addr = tst_get_bad_addr(NULL);
	}
}

static void cleanup(void)
{
	if (!access(USRPATH, F_OK))
		SAFE_UNLINK(USRPATH);

	if (!access(TESTDIR1, F_OK))
		SAFE_RMDIR(TESTDIR1);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.needs_drivers = (const char *const []) {
		"quota_v2",
		NULL
	},
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_quotactl,
	.filesystems = (struct tst_fs []) {
		{
			.type = "ext4",
			.mnt_data = "usrquota",
		},
		{}
	},
	.mntpoint = MNTPOINT,
	.mount_device = 1,
	.needs_cmds = (struct tst_cmd[]) {
		{.cmd = "quotacheck"},
		{}
	},
	.needs_root = 1,
	.test_variants = QUOTACTL_FMT_VARIANTS,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "7e08da50cf70"},
		{}
	}
};
