/*
 * Copyright (c) 2017 Fujitsu Ltd.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program, if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Test Name: quotactl03
 *
 * Description:
 * quotactl(2) with XGETNEXTQUOTA looks for the next active quota for an user
 * equal or higher to a given ID, in this test the ID is specified to a value
 * close to UINT_MAX(max value of unsigned int). When reaching the upper limit
 * and finding no active quota, it should return -1 and set errno to ENOENT.
 * Actually, quotactl(2) overflows and and return 0 as the "next" active id.
 *
 * This kernel bug of xfs has been fixed in:
 *
 * commit 657bdfb7f5e68ca5e2ed009ab473c429b0d6af85
 * Author: Eric Sandeen <sandeen@redhat.com>
 * Date:   Tue Jan 17 11:43:38 2017 -0800
 *
 *     xfs: don't wrap ID in xfs_dq_get_next_id
 */

#define _GNU_SOURCE
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/quota.h>
#include "config.h"

#if defined(HAVE_QUOTAV2) || defined(HAVE_QUOTAV1)
# include <sys/quota.h>
#endif

#if defined(HAVE_XFS_QUOTA)
# include <xfs/xqm.h>
#endif

#include "tst_test.h"
#include "lapi/quotactl.h"

#if defined(HAVE_XFS_QUOTA) && (defined(HAVE_QUOTAV2) || defined(HAVE_QUOTAV1))

static const char mntpoint[] = "mnt_point";
static uint32_t test_id = 0xfffffffc;

static void verify_quota(void)
{
	struct fs_disk_quota res_dquota;

	res_dquota.d_id = 1;

	TEST(quotactl(QCMD(Q_XGETNEXTQUOTA, USRQUOTA), tst_device->dev,
		test_id, (void *)&res_dquota));
	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "quotactl() found the next active ID:"
			" %u unexpectedly", res_dquota.d_id);
		return;
	}

	if (TEST_ERRNO == EINVAL) {
		tst_brk(TCONF | TTERRNO,
			"Q_XGETNEXTQUOTA wasn't supported in quotactl()");
	}

	if (TEST_ERRNO != ENOENT) {
		tst_res(TFAIL | TTERRNO, "quotaclt() failed unexpectedly with"
			" %s expected ENOENT", tst_strerrno(TEST_ERRNO));
	} else {
		tst_res(TPASS, "quotaclt() failed with ENOENT as expected");
	}
}

static struct tst_test test = {
	.tid = "quotactl03",
	.needs_tmpdir = 1,
	.needs_root = 1,
	.test_all = verify_quota,
	.mount_device = 1,
	.dev_fs_type = "xfs",
	.mntpoint = mntpoint,
	.mnt_data = "usrquota",
};

#else
	TST_TEST_TCONF("This system didn't support quota or xfs quota");
#endif
