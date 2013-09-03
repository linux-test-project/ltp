/*
 * Copyright (c) 2013 Fujitsu Ltd.
 * Author: DAN LI <li.dan@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * Description:
 *	This tests basic flags of quotactl() syscall:
 *	1) Q_XQUOTAOFF - Turn off quotas for an XFS file system.
 *	2) Q_XQUOTAON - Turn on quotas for an XFS file system.
 *	3) Q_XGETQUOTA - Get disk quota limits and current usage for user id.
 *	4) Q_XSETQLIM - Set disk quota limits for user id.
 *	5) Q_XGETQSTAT - Get XFS file system specific quota information.
 */

#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mount.h>
#include <linux/fs.h>
#include <sys/types.h>

#include "config.h"
#if defined(HAVE_XFS_QUOTA)
#include <xfs/xqm.h>
#endif
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"
#include "safe_macros.h"

#define USRQCMD(cmd)	((cmd) << 8)
#define RTBLIMIT	2000

char *TCID = "quotactl02";
int TST_TOTAL = 5;

#if defined(HAVE_XFS_QUOTA)
static void check_qoff(void);
static void check_qon(void);
static void check_getq(void);
static void setup_setqlim(void), check_setqlim(void);
static void check_getqstat(void);

static void setup(void);
static void cleanup(void);
static void help(void);

static int i;
static int uid;
static int dflag;
static char *block_dev;
static struct fs_disk_quota dquota;
static struct fs_quota_stat qstat;
static unsigned int qflag = XFS_QUOTA_UDQ_ENFD;
static const char mntpoint[] = "mnt_point";

static option_t options[] = {
	{"D:", &dflag, &block_dev},
	{NULL, NULL, NULL},
};

static struct test_case_t {
	int cmd;
	void *addr;
	void (*func_test) ();
	void (*func_setup) ();
} TC[] = {
	{Q_XQUOTAOFF, &qflag, check_qoff, NULL},
	{Q_XQUOTAON, &qflag, check_qon, NULL},
	{Q_XGETQUOTA, &dquota, check_getq, NULL},
	{Q_XSETQLIM, &dquota, check_setqlim, setup_setqlim},
	{Q_XGETQSTAT, &qstat, check_getqstat, NULL},
};

int main(int argc, char *argv[])
{
	int lc;
	char *msg;

	msg = parse_opts(argc, argv, options, &help);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	if (!dflag)
		tst_brkm(TBROK, NULL,
			 "you must specify the device used for mounting with "
			 "the -D option");

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			if (TC[i].func_setup != NULL)
				(*TC[i].func_setup) ();

			TEST(ltp_syscall(__NR_quotactl,
					 USRQCMD(TC[i].cmd), block_dev,
					 uid, TC[i].addr));

			if (TEST_RETURN != 0)
				tst_resm(TFAIL | TERRNO,
					 "cmd=0x%x failed", TC[i].cmd);

			if (STD_FUNCTIONAL_TEST)
				(*TC[i].func_test) ();
			else
				tst_resm(TPASS, "quotactl call succeeded");

		}
	}
	cleanup();
	tst_exit();
}

static void check_qoff(void)
{
	int ret;

	ret = ltp_syscall(__NR_quotactl, USRQCMD(Q_XGETQSTAT),
			  block_dev, uid, &qstat);
	if (ret != 0)
		tst_brkm(TBROK | TERRNO, cleanup, "fail to get quota stat");

	if (qstat.qs_flags & XFS_QUOTA_UDQ_ENFD) {
		tst_resm(TFAIL, "enforcement is not off");
		return;
	}

	tst_resm(TPASS, "enforcement is off");
}

static void check_qon(void)
{
	int ret;
	ret = ltp_syscall(__NR_quotactl, USRQCMD(Q_XGETQSTAT),
			  block_dev, uid, &qstat);
	if (ret != 0)
		tst_brkm(TBROK | TERRNO, cleanup, "fail to get quota stat");

	if (!(qstat.qs_flags & XFS_QUOTA_UDQ_ENFD)) {
		tst_resm(TFAIL, "enforcement is off");
		return;
	}

	tst_resm(TPASS, "enforcement is on");
}

static void check_getq(void)
{
	if (!(dquota.d_flags & XFS_USER_QUOTA)) {
		tst_resm(TFAIL, "get incorrect quota type");
		return;
	}

	tst_resm(TPASS, "get the right quota type");
}

static void setup_setqlim(void)
{
	dquota.d_rtb_hardlimit = RTBLIMIT;
	dquota.d_fieldmask = FS_DQ_LIMIT_MASK;
}

static void check_setqlim(void)
{
	int ret;
	ret = ltp_syscall(__NR_quotactl, USRQCMD(Q_XGETQUOTA),
			  block_dev, uid, &dquota);
	if (ret != 0)
		tst_brkm(TFAIL | TERRNO, NULL,
			 "fail to get quota information");

	if (dquota.d_rtb_hardlimit != RTBLIMIT) {
		tst_resm(TFAIL, "limit on RTB, except %lu get %lu",
			 (uint64_t)RTBLIMIT,
			 (uint64_t)dquota.d_rtb_hardlimit);
		return;
	}

	tst_resm(TPASS, "quotactl works fine with Q_XSETQLIM");
}

static void check_getqstat(void)
{
	if (qstat.qs_version != FS_QSTAT_VERSION) {
		tst_resm(TFAIL, "get incorrect qstat version");
		return;
	}

	tst_resm(TPASS, "get correct qstat version");
}

static void setup(void)
{

	tst_require_root(NULL);

	TEST_PAUSE;

	tst_tmpdir();

	SAFE_MKDIR(cleanup, mntpoint, 0755);

	tst_mkfs(NULL, block_dev, "xfs", NULL);

	if (mount(block_dev, mntpoint, "xfs", 0, "uquota") < 0)
		tst_brkm(TFAIL | TERRNO, NULL, "mount(2) fail");

}

static void cleanup(void)
{
	if (umount(mntpoint) < 0)
		tst_resm(TFAIL | TERRNO, "umount(2) fail");

	TEST_CLEANUP;
	tst_rmdir();
}

static void help(void)
{
	printf("-D device : device used for mounting.\n");
}
#else
int main(void)
{
	tst_brkm(TCONF, NULL, "This system doesn't support xfs quota");
}
#endif
