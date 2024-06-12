// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * [Description]
 *
 * Tests basic error handling of the quotactl syscall without visible quota files
 * (use quotactl and quotactl_fd syscall):
 *
 * - EFAULT when addr or special is invalid
 * - EINVAL when cmd or type is invalid
 * - ENOTBLK when special is not a block device
 * - ERANGE when cmd is Q_SETQUOTA, but the specified limits are out of the range
 *   allowed by the quota format
 * - EPERM when the caller lacked the required privilege (CAP_SYS_ADMIN) for the
 *   specified operation
 * - ENOSYS when cmd is Q_QUOTAON, but the fd refers to a socket
 *
 * Minimum e2fsprogs version required is 1.43.
 */

#include <errno.h>
#include <sys/quota.h>
#include <sys/socket.h>
#include "tst_test.h"
#include "tst_capability.h"
#include "quotactl_syscall_var.h"

#define OPTION_INVALID 999

static int32_t fmt_id = QFMT_VFS_V1;
static int test_id;
static int getnextquota_nsup, socket_fd = -1;

static struct if_nextdqblk res_ndq;

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
	{QCMD(Q_SETQUOTA, USRQUOTA), &fmt_id, NULL, EFAULT, 1,
	"EFAULT when addr or special is invalid"},

	{QCMD(OPTION_INVALID, USRQUOTA), &fmt_id, NULL, EINVAL, 0,
	"EINVAL when cmd or type is invalid"},

	{QCMD(Q_QUOTAON, USRQUOTA), &fmt_id, NULL, ENOTBLK, 0,
	"ENOTBLK when special is not a block device"},

	{QCMD(Q_SETQUOTA, USRQUOTA), &test_id, &set_dqmax, ERANGE, 1,
	"ERANGE when cmd is Q_SETQUOTA, but the specified limits are out of the range"},

	{QCMD(Q_QUOTAON, USRQUOTA), &fmt_id, NULL, EPERM, 0,
	"EPERM when the caller lacks required privilege(CAP_SYS_ADMIN)"},

	{QCMD(Q_QUOTAON, USRQUOTA), &fmt_id, NULL, ENOSYS, 0,
	"EINVAL when cmd is Q_QUOTAON, but the fd refers to a socket"}
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
		TST_EXP_PASS_SILENT(do_quotactl(fd, QCMD(Q_QUOTAON, USRQUOTA), tst_device->dev,
			fmt_id, NULL), "do_quotactl(QCMD(Q_QUOTAON, USRQUOTA))");
		if (!TST_PASS)
			return;
		quota_on = 1;
	}

	if (tc->exp_err == EPERM) {
		tst_cap_action(&dropadmin);
		drop_flag = 1;
	}

	if (tst_variant) {
		if (tc->exp_err == ENOTBLK) {
			tst_res(TCONF, "quotactl_fd() doesn't have this error, skip");
			return;
		}
		if (tc->exp_err == ENOSYS) {
			TST_EXP_FAIL(syscall(__NR_quotactl_fd, socket_fd, tc->cmd, *tc->id,
				tc->addr), tc->exp_err, "syscall(quotactl_fd)");
			return;
		}
	} else {
		if (tc->exp_err == ENOSYS) {
			tst_res(TCONF, "quotactl() doesn't use fd, skip");
			return;
		}
	}
	if (tc->exp_err == ENOTBLK)
		TST_EXP_FAIL(do_quotactl(fd, tc->cmd, "/dev/null", *tc->id, tc->addr),
			ENOTBLK, "do_quotactl()");
	else
		TST_EXP_FAIL(do_quotactl(fd, tc->cmd, tst_device->dev, *tc->id, tc->addr),
			tc->exp_err, "do_quotactl()");

	if (quota_on) {
		TST_EXP_PASS_SILENT(do_quotactl(fd, QCMD(Q_QUOTAOFF, USRQUOTA), tst_device->dev,
			fmt_id, NULL), "do_quotactl(QCMD(Q_QUOTAOFF, USRQUOTA)");
		if (!TST_PASS)
			return;
	}

	if (drop_flag)
		tst_cap_action(&needadmin);
}

static void setup(void)
{
	unsigned int i;

	quotactl_info();

	socket_fd = SAFE_SOCKET(PF_INET, SOCK_STREAM, 0);
	fd = SAFE_OPEN(MNTPOINT, O_RDONLY);
	TEST(do_quotactl(fd, QCMD(Q_GETNEXTQUOTA, USRQUOTA), tst_device->dev,
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
	if (fd > -1)
		SAFE_CLOSE(fd);
	if (socket_fd > -1)
		SAFE_CLOSE(socket_fd);
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
			.mkfs_opts = (const char *const[]){"-O quota", NULL},
		},
		{}
	},
	.mntpoint = MNTPOINT,
	.mount_device = 1,
	.needs_root = 1,
	.test_variants = QUOTACTL_SYSCALL_VARIANTS,
	.needs_cmds = (const char *[]) {
		"mkfs.ext4 >= 1.43.0",
		NULL
	}
};
