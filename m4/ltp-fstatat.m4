dnl
dnl Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
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
dnl along with this program. If not, see <http://www.gnu.org/licenses/>.
dnl
dnl Author: Alexey Kodanev <alexey.kodanev@oracle.com>
dnl

dnl
dnl LTP_CHECK_FSTATAT
dnl -----------------
dnl
AC_DEFUN([LTP_CHECK_FSTATAT],[
AC_CHECK_FUNCS(fstatat,,)
])
