dnl
dnl Copyright (c) Red Hat Inc., 2009
dnl
dnl This program is free software;  you can redistribute it and/or
dnl modify it under the terms of the GNU General Public License as
dnl published by the Free Software Foundation; either version 2 of
dnl the License, or (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY;  without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
dnl the GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program;  if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
dnl USA

dnl
dnl LTP_CHECK_SELINUX
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_SELINUX],
[dnl
AH_TEMPLATE(HAVE_LIBSELINUX_DEVEL,
[Define to 1 if you have both SELinux libraries and headers.])
AC_CHECK_HEADERS(selinux/selinux.h,[
        AC_CHECK_LIB(selinux,is_selinux_enabled,[
                AC_DEFINE(HAVE_LIBSELINUX_DEVEL) SELINUX_LIBS="-lselinux"],[
                SELINUX_LIBS=""])],[
        SELINUX_LIBS=""])
AC_SUBST(SELINUX_LIBS)])
