dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2019 Petr Vorel <petr.vorel@gmail.com>

AC_DEFUN([LTP_CHECK_FTS_H],[
	AC_CHECK_HEADERS(fts.h, [have_fts=1])
	AC_SUBST(HAVE_FTS_H, $have_fts)
])
