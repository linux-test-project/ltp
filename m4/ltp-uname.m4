dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2017 Richard Palethorpe <rpalethorpe@suse.com>

AC_DEFUN([LTP_CHECK_UNAME_DOMAINNAME],[
AC_CHECK_MEMBERS([struct utsname.domainname],,,[
#define _GNU_SOURCE
#include <sys/utsname.h>
])])
