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
#include <linux/quota.h>
#include <xfs/xqm.h>
#include <linux/dqblk_v1.h>
#include <linux/dqblk_v2.h>
#include <unistd.h>
int main(void) {
    return quotactl(Q_GETINFO, (const char *)NULL, geteuid(void), (caddr_t)NULL);
}
],[has_old_quotactl="yes"],[AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/quota.h>
#include <unistd.h>
int main(void) {
    return quotactl(Q_GETINFO, (const char *)NULL, geteuid(void), (caddr_t)NULL);
}
],[has_new_quotactl="yes"],[AC_MSG_WARN([Couldn't find functional copy of quotactl])])])
if test "$has_old_quotactl" = "yes"; then
	AC_DEFINE(HAS_OLD_QUOTACTL,1,[Define to 1 if you have the old implementation of quotactl])
elif test "$has_new_quotactl" = "yes"; then
	AC_DEFINE(HAS_NEW_QUOTACTL,1,[Define to 1 if you have the new implementation of quotactl])
fi
])
