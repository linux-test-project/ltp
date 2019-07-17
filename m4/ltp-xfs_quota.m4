dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2013 Fujitsu Ltd.
dnl Author: DAN LI <li.dan@cn.fujitsu.com>

AC_DEFUN([LTP_CHECK_XFS_QUOTACTL],[
	AC_MSG_CHECKING([for XFS quota (xfs/xqm.h)])
	AC_LINK_IFELSE([AC_LANG_SOURCE([
#define _GNU_SOURCE
#include <xfs/xqm.h>
#include <sys/quota.h>
int main(void) {
	struct fs_quota_stat qstat;
	return quotactl(QCMD(Q_XGETQSTAT, USRQUOTA), "/dev/null", geteuid(),
			(caddr_t) &qstat);
}])],[has_xfs_quota="yes"])

if test "x$has_xfs_quota" = xyes; then
	AC_DEFINE(HAVE_XFS_QUOTA,1,[Define to 1 if you have xfs quota])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
])
