dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Jiri Palecek 2009

AC_DEFUN([LTP_CHECK_TASKSTATS],
_LTP_CHECK_TASKSTATS_FREEPAGES
)

dnl Check for taskstat.freepages_* members, introduced to the kernel
dnl in commit 016ae219 in July 2008

AC_DEFUN([_LTP_CHECK_TASKSTATS_FREEPAGES],[
AC_CHECK_HEADERS([linux/taskstats.h],[
    AC_CHECK_MEMBERS([struct taskstats.freepages_count, struct taskstats.nvcsw, struct taskstats.read_bytes],
                     [],[],[
#include <linux/taskstats.h>
])
],[],[
#include <linux/types.h>
])
])
