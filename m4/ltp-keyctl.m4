dnl
dnl Copyright (c) Linux Test Project, 2010 
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
dnl LTP_CHECK_SYSCALL_KEYCTL
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_SYSCALL_KEYCTL],[AC_LINK_IFELSE([AC_LANG_SOURCE([
#include <sys/types.h>
#include <keyutils.h>
int main(void) {
	key_serial_t akey;
	(void) keyctl(KEYCTL_READ, akey);
	return 0;
}])],[has_keyctl_syscall="yes"])
if test "x$has_keyctl_syscall" = "xyes"; then
	AC_DEFINE(HAVE_KEYCTL_SYSCALL,1,[Define to 1 if you have the libkeyutils development package and keyctl syscall on your system])
else
	AC_DEFINE(HAVE_KEYCTL_SYSCALL,0,[Define to 1 if you have the libkeyutils development package and keyctl syscall on your system])
fi
])
