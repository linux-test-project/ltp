dnl
dnl Copyright (c) 2015 Fujitsu Ltd.
dnl Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
dnl
dnl This program is free software; you can redistribute it and/or modify it
dnl under the terms of version 2 of the GNU General Public License as
dnl published by the Free Software Foundation.
dnl
dnl This program is distributed in the hope that it would be useful, but
dnl WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
dnl
dnl You should have received a copy of the GNU General Public License
dnl alone with this program.
dnl

dnl
dnl LTP_CHECK_PREADV
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_PREADV],[
AC_CHECK_FUNCS(preadv,,)
])
