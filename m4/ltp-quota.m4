dnl
dnl Copyright (c) Cisco Systems, Inc, 2008
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
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
dnl

dnl
dnl LTP_CHECK_SYSCALL_QUOTACTL
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_SYSCALL_QUOTACTL],[AC_TRY_COMPILE([
#define _LINUX_QUOTA_VERSION 2 
#include <sys/types.h>
#include <sys/quota.h>
#include <unistd.h>],[
	struct dqblk dq;
	return quotactl(QCMD(Q_GETINFO, USRQUOTA), (const char *) "/dev/null",
			geteuid(), (caddr_t) &dq);
],[has_quotav2="yes"],[dnl END quota v2 check
AC_TRY_COMPILE([
#define _LINUX_QUOTA_VERSION 1
#include <sys/types.h>
#include <sys/quota.h>
#include <unistd.h>],[
	struct dqblk dq;
	return quotactl(QCMD(Q_GETQUOTA, USRQUOTA), (const char *) "/dev/null",
			geteuid(), (caddr_t) &dq);
],[has_quotav1="yes"],AC_MSG_WARN(Couldn't determine quota version (please submit config.log and manpage details for inspectionto ltp-list@lists.sourceforge.net))) dnl END quota v1 check
]) 
if test "x$has_quotav1" = "xyes"; then
	AC_DEFINE(HAVE_QUOTAV1,1,[Define to 1 if you have quota v1])
elif test "x$has_quotav2" = "xyes"; then
	AC_DEFINE(HAVE_QUOTAV2,1,[Define to 1 if you have quota v2])
fi
])
