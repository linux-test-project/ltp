dnl
dnl Copyright (c) 2013 Fujitsu Ltd.
dnl Author: DAN LI <li.dan@cn.fujitsu.com>
dnl
dnl This program is free software;  you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY;  without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
dnl the GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program;  if not, write to the Free Software
dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
dnl

dnl
dnl LTP_CHECK_XFS_QUOTACTL
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_XFS_QUOTACTL],[dnl
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
