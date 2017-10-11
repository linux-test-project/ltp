dnl
dnl Copyright (c) Red Hat Inc., 2008
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
dnl Author: Masatake YAMATO <yamato@redhat.com>
dnl Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
dnl

dnl
dnl LTP_CHECK_SYSCALL_EVENTFD
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_SYSCALL_EVENTFD], [
	AC_CHECK_HEADERS(libaio.h, [have_libaio=yes])
	AC_CHECK_LIB(aio, io_setup, [have_aio=yes])

	if test "x$have_libaio" = "xyes" -a "x$have_aio" = "xyes"; then
		AC_DEFINE(HAVE_LIBAIO, 1, [Define to 1 if you have libaio and it's headers installed.])
		AC_SUBST(AIO_LIBS, "-laio")

		AC_MSG_CHECKING([io_set_eventfd is defined in aio library or aio header])
		AC_TRY_LINK([#include <stdio.h>
                             #include <libaio.h>
		            ],
		            [io_set_eventfd(NULL, 0); return 0;],
		            [AC_DEFINE(HAVE_IO_SET_EVENTFD, 1, [Define to 1 if you have `io_set_eventfd' function.])
						AC_MSG_RESULT(yes)],
		            [AC_MSG_RESULT(no)])
	fi
])
