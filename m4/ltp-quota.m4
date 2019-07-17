dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Cisco Systems, Inc, 2008

AC_DEFUN([LTP_CHECK_SYSCALL_QUOTACTL],[
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
