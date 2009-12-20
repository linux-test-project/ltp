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
AC_DEFUN([LTP_CHECK_SYSCALL_QUOTACTL],
dnl Backstory: this autoconf test is bogus because it (and the quotactl test)
dnl uses Q_GETINFO which only exists in the quota 2.x code. Thus, it's
dnl commented out now until a proper 2.4.x test can be written, if necessary...
dnl[AC_TRY_COMPILE([
dnl #include <linux/quota.h>
dnl #include <unistd.h>],[
dnl int main(void) {
dnl	return quotactl(Q_GETINFO, (const char *)NULL, geteuid(), (caddr_t)NULL);
dnl}],[has_24_quotactl="yes"],
[AC_TRY_COMPILE([
#define _LINUX_QUOTA_VERSION 2
#include <sys/types.h>
#include <sys/quota.h>
#include <unistd.h>],[
int main(void) {
	return quotactl(Q_GETINFO, (const char *)NULL, geteuid(), (caddr_t)NULL);
}],[has_quotav2="yes"],[AC_MSG_ERROR([Couldn't find functional copy of quota v2 - are you running 2.4.x?])])
dnl if test "x$has_24_quotactl" = "xyes"; then
dnl	AC_DEFINE(HAS_24_QUOTACTL,1,[Define to 1 if you have the 2.4.x version of quotactl, e.g. require linux/quota.h instead of sys/quota.h])
dnl elif test "x$has_new_26_quotactl" = "xyes"; then
if test "x$has_quotav2" = "xyes"; then
	AC_DEFINE(HAS_QUOTAV2,1,[Define to 1 if you have quota v2 code, e.g. are running 2.6.x])
fi
])
