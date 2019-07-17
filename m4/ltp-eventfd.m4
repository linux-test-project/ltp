dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Red Hat Inc., 2008
dnl Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
dnl Author: Masatake YAMATO <yamato@redhat.com>

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
