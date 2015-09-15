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
dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
dnl

dnl
dnl LTP_CHECK_SYSCALL_QUOTACTL
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_SYSCALL_QUOTACTL],[dnl
	AC_LINK_IFELSE([AC_LANG_SOURCE([
#define _LINUX_QUOTA_VERSION 2
#include <sys/types.h>
#include <sys/quota.h>
#include <unistd.h>
int main(void) {
	struct dqblk dq;
	return quotactl(QCMD(Q_GETINFO, USRQUOTA), (const char *) "/dev/null",
			geteuid(), (caddr_t) &dq);
}])],[has_quotav2="yes"])

if test "x$has_quotav2" = xyes; then
	AC_DEFINE(HAVE_QUOTAV2,1,[Define to 1 if you have quota v2])
else

	# got quota v1?
	AC_LINK_IFELSE([AC_LANG_SOURCE([
#define _LINUX_QUOTA_VERSION 1
#include <sys/types.h>
#include <sys/quota.h>
#include <unistd.h>
int main(void) {
	struct dqblk dq;
	return quotactl(QCMD(Q_GETQUOTA, USRQUOTA), (const char *) "/dev/null",
			geteuid(), (caddr_t) &dq);
}])],[has_quotav1="yes"])

	if test "x$has_quotav1" = xyes; then
		AC_DEFINE(HAVE_QUOTAV1,1,[Define to 1 if you have quota v1])
	else
		AC_MSG_WARN(Couldn't determine quota version (please submit config.log and manpage to ltp@lists.linux.it))
	fi

fi
])
