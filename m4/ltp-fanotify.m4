dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2019 Petr Vorel <petr.vorel@gmail.com>

AC_DEFUN([LTP_CHECK_FANOTIFY],[
AC_CHECK_TYPES([struct fanotify_event_info_header],,,[#include <sys/fanotify.h>])
AC_CHECK_TYPES([struct fanotify_event_info_fid],,,[#include <sys/fanotify.h>])
AC_CHECK_MEMBERS([struct fanotify_event_info_fid.fsid.__val],,,[#include <sys/fanotify.h>])
])
