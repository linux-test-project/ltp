dnl
dnl Copyright (c) Linux Test Project, 2011
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
dnl Author: Cyril Hrubis <chrubis@suse.cz>
dnl

dnl
dnl LTP_CHECK_MADVISE
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_MADVISE],
[dnl
AC_CHECK_DECLS([MADV_MERGEABLE],[have_madv_mergeable="yes"],,[#include <sys/mman.h>])
if test "x$have_madv_mergeable" = "xyes"; then
	AC_DEFINE(HAVE_MADV_MERGEABLE,1,[Define to 1 if you have MADV_MERGEABLE])
fi
])
