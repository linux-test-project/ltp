dnl
dnl Copyright (c) 2016 Fujitsu Ltd.
dnl Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
dnl the GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program.
dnl

dnl
dnl LTP_CHECK_EPOLL_PWAIT
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_EPOLL_PWAIT],[
AC_CHECK_FUNCS(epoll_pwait,,)
])
