# This file is part of Autoconf.                       -*- Autoconf -*-
# Checking for headers.
# Copyright 2000, 2001
# Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.
#
# As a special exception, the Free Software Foundation gives unlimited
# permission to copy, distribute and modify the configure scripts that
# are the output of Autoconf.  You need not follow the terms of the GNU
# General Public License when using or distributing such scripts, even
# though portions of the text of Autoconf appear in them.  The GNU
# General Public License (GPL) does govern all other use of the material
# that constitutes the Autoconf program.
#
# Certain portions of the Autoconf source text are designed to be copied
# (in certain cases, depending on the input) into the output of
# Autoconf.  We call these the "data" portions.  The rest of the Autoconf
# source text consists of comments plus executable code that decides which
# of the data portions to output in any given case.  We call these
# comments and executable code the "non-data" portions.  Autoconf never
# copies any of the non-data portions into its output.
#
# This special exception to the GPL applies to versions of Autoconf
# released by the Free Software Foundation.  When you make and
# distribute a modified version of Autoconf, you may extend this special
# exception to the GPL to apply to your modified version as well, *unless*
# your modified version has the potential to copy into its output some
# of the text that was the non-data portion of the version that you started
# with.  (In other words, unless your change moves or copies text from
# the non-data portions to the data portions.)  If your modification has
# such potential, you must delete any notice of this special exception
# to the GPL from your modified version.
#
# Written by David MacKenzie, with help from
# Franc,ois Pinard, Karl Berry, Richard Pixley, Ian Lance Taylor,
# Roland McGrath, Noah Friedman, david d zuhn, and many others.


# Table of contents
#
# 1. Generic tests for headers
# 2. Tests for specific headers


## ------------------------------ ##
## 1. Generic tests for headers.  ##
## ------------------------------ ##


# AC_CHECK_HEADER(HEADER-FILE,
#                 [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#                 [INCLUDES])
# ---------------------------------------------------------
# If INCLUDES is empty and strictly empty, use the preprocessor to
# check whether HEADER-FILE exists.  If INCLUDES is set, then use the
# compiler to check whether INCLUDES followed by HEADER-FILE compiles
# with success.
AC_DEFUN([AC_CHECK_HEADER],
[AS_VAR_PUSHDEF([ac_Header], [ac_cv_header_$1])dnl
AC_CACHE_CHECK([for $1], ac_Header,
            [m4_ifval([$4],
                      [AC_COMPILE_IFELSE([AC_LANG_SOURCE([$4
@%:@include <$1>])],
                                         [AS_VAR_SET(ac_Header, yes)],
                                         [AS_VAR_SET(ac_Header, no)])],
                      [AC_PREPROC_IFELSE([AC_LANG_SOURCE([@%:@include <$1>])],
                                         [AS_VAR_SET(ac_Header, yes)],
                                         [AS_VAR_SET(ac_Header, no)])])])
AS_IF([test AS_VAR_GET(ac_Header) = yes], [$2], [$3])[]dnl
AS_VAR_POPDEF([ac_Header])dnl
])# AC_CHECK_HEADER


# AH_CHECK_HEADERS(HEADER-FILE...)
# --------------------------------
m4_define([AH_CHECK_HEADERS],
[AC_FOREACH([AC_Header], [$1],
  [AH_TEMPLATE(AS_TR_CPP(HAVE_[]AC_Header),
               [Define if you have the <]AC_Header[> header file.])])])


# AC_CHECK_HEADERS(HEADER-FILE...
#                  [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#                  [INCLUDES])
# ----------------------------------------------------------
AC_DEFUN([AC_CHECK_HEADERS],
[AH_CHECK_HEADERS([$1])dnl
for ac_header in $1
do
AC_CHECK_HEADER($ac_header,
                [AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_$ac_header)) $2],
                [$3],
                [$4])dnl
done
])# AC_CHECK_HEADERS





## ------------------------------- ##
## 2. Tests for specific headers.  ##
## ------------------------------- ##


# _AC_CHECK_HEADER_DIRENT(HEADER-FILE,
#                         [ACTION-IF-FOUND], [ACTION-IF-NOT_FOUND])
# -----------------------------------------------------------------
# Like AC_CHECK_HEADER, except also make sure that HEADER-FILE
# defines the type `DIR'.  dirent.h on NextStep 3.2 doesn't.
m4_define([_AC_CHECK_HEADER_DIRENT],
[AS_VAR_PUSHDEF([ac_Header], [ac_cv_header_dirent_$1])dnl
AC_CACHE_CHECK([for $1 that defines DIR], ac_Header,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <sys/types.h>
#include <$1>
],
                                    [if ((DIR *) 0)
return 0;])],
                   [AS_VAR_SET(ac_Header, yes)],
                   [AS_VAR_SET(ac_Header, no)])])
AS_IF([test AS_VAR_GET(ac_Header) = yes], [$2], [$3])[]dnl
AS_VAR_POPDEF([ac_Header])dnl
])# _AC_CHECK_HEADER_DIRENT


# AH_CHECK_HEADERS_DIRENT(HEADERS...)
# -----------------------------------
m4_define([AH_CHECK_HEADERS_DIRENT],
[AC_FOREACH([AC_Header], [$1],
  [AH_TEMPLATE(AS_TR_CPP(HAVE_[]AC_Header),
               [Define if you have the <]AC_Header[> header file, and
                it defines `DIR'.])])])


# AC_HEADER_DIRENT
# ----------------
AC_DEFUN([AC_HEADER_DIRENT],
[AH_CHECK_HEADERS_DIRENT(dirent.h sys/ndir.h sys/dir.h ndir.h)
ac_header_dirent=no
for ac_hdr in dirent.h sys/ndir.h sys/dir.h ndir.h; do
  _AC_CHECK_HEADER_DIRENT($ac_hdr,
                          [AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_$ac_hdr), 1)
ac_header_dirent=$ac_hdr; break])
done
# Two versions of opendir et al. are in -ldir and -lx on SCO Xenix.
if test $ac_header_dirent = dirent.h; then
  AC_CHECK_LIB(dir, opendir, LIBS="$LIBS -ldir")
else
  AC_CHECK_LIB(x, opendir, LIBS="$LIBS -lx")
fi
])# AC_HEADER_DIRENT


# AC_HEADER_MAJOR
# ---------------
AC_DEFUN([AC_HEADER_MAJOR],
[AC_CACHE_CHECK(whether sys/types.h defines makedev,
  ac_cv_header_sys_types_h_makedev,
[AC_TRY_LINK([#include <sys/types.h>
], [return makedev(0, 0);],
  ac_cv_header_sys_types_h_makedev=yes, ac_cv_header_sys_types_h_makedev=no)
])

if test $ac_cv_header_sys_types_h_makedev = no; then
AC_CHECK_HEADER(sys/mkdev.h,
                [AC_DEFINE(MAJOR_IN_MKDEV, 1,
                           [Define if `major', `minor', and `makedev' are
                            declared in <mkdev.h>.])])

  if test $ac_cv_header_sys_mkdev_h = no; then
    AC_CHECK_HEADER(sys/sysmacros.h,
                    [AC_DEFINE(MAJOR_IN_SYSMACROS, 1,
                               [Define if `major', `minor', and `makedev' are
                                declared in <sysmacros.h>.])])
  fi
fi
])# AC_HEADER_MAJOR


# AC_HEADER_STAT
# --------------
# FIXME: Shouldn't this be named AC_HEADER_SYS_STAT?
AC_DEFUN([AC_HEADER_STAT],
[AC_CACHE_CHECK(whether stat file-mode macros are broken,
  ac_cv_header_stat_broken,
[AC_EGREP_CPP([You lose], [#include <sys/types.h>
#include <sys/stat.h>

#if defined(S_ISBLK) && defined(S_IFDIR)
# if S_ISBLK (S_IFDIR)
You lose.
# endif
#endif

#if defined(S_ISBLK) && defined(S_IFCHR)
# if S_ISBLK (S_IFCHR)
You lose.
# endif
#endif

#if defined(S_ISLNK) && defined(S_IFREG)
# if S_ISLNK (S_IFREG)
You lose.
# endif
#endif

#if defined(S_ISSOCK) && defined(S_IFREG)
# if S_ISSOCK (S_IFREG)
You lose.
# endif
#endif
], ac_cv_header_stat_broken=yes, ac_cv_header_stat_broken=no)])
if test $ac_cv_header_stat_broken = yes; then
  AC_DEFINE(STAT_MACROS_BROKEN, 1,
            [Define if the `S_IS*' macros in <sys/stat.h> do not
             work properly.])
fi
])# AC_HEADER_STAT


# AC_HEADER_STDC
# --------------
AC_DEFUN([AC_HEADER_STDC],
[AC_CACHE_CHECK(for ANSI C header files, ac_cv_header_stdc,
[AC_TRY_CPP([#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <float.h>
], ac_cv_header_stdc=yes, ac_cv_header_stdc=no)

if test $ac_cv_header_stdc = yes; then
  # SunOS 4.x string.h does not declare mem*, contrary to ANSI.
  AC_EGREP_HEADER(memchr, string.h, , ac_cv_header_stdc=no)
fi

if test $ac_cv_header_stdc = yes; then
  # ISC 2.0.2 stdlib.h does not declare free, contrary to ANSI.
  AC_EGREP_HEADER(free, stdlib.h, , ac_cv_header_stdc=no)
fi

if test $ac_cv_header_stdc = yes; then
  # /bin/cc in Irix-4.0.5 gets non-ANSI ctype macros unless using -ansi.
  AC_TRY_RUN(
[#include <ctype.h>
#if ((' ' & 0x0FF) == 0x020)
# define ISLOWER(c) ('a' <= (c) && (c) <= 'z')
# define TOUPPER(c) (ISLOWER(c) ? 'A' + ((c) - 'a') : (c))
#else
# define ISLOWER(c) (('a' <= (c) && (c) <= 'i') \
                     || ('j' <= (c) && (c) <= 'r') \
                     || ('s' <= (c) && (c) <= 'z'))
# define TOUPPER(c) (ISLOWER(c) ? ((c) | 0x40) : (c))
#endif

#define XOR(e, f) (((e) && !(f)) || (!(e) && (f)))
int
main ()
{
  int i;
  for (i = 0; i < 256; i++)
    if (XOR (islower (i), ISLOWER (i))
        || toupper (i) != TOUPPER (i))
      exit(2);
  exit (0);
}], , ac_cv_header_stdc=no, :)
fi])
if test $ac_cv_header_stdc = yes; then
  AC_DEFINE(STDC_HEADERS, 1, [Define if you have the ANSI C header files.])
fi
])# AC_HEADER_STDC


# AC_HEADER_SYS_WAIT
# ------------------
AC_DEFUN([AC_HEADER_SYS_WAIT],
[AC_CACHE_CHECK([for sys/wait.h that is POSIX.1 compatible],
  ac_cv_header_sys_wait_h,
[AC_COMPILE_IFELSE(
[AC_LANG_PROGRAM([#include <sys/types.h>
#include <sys/wait.h>
#ifndef WEXITSTATUS
# define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
# define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif
],
[  int s;
  wait (&s);
  s = WIFEXITED (s) ? WEXITSTATUS (s) : 1;])],
                 [ac_cv_header_sys_wait_h=yes],
                 [ac_cv_header_sys_wait_h=no])])
if test $ac_cv_header_sys_wait_h = yes; then
  AC_DEFINE(HAVE_SYS_WAIT_H, 1,
            [Define if you have <sys/wait.h> that is POSIX.1 compatible.])
fi
])# AC_HEADER_SYS_WAIT


# AC_HEADER_TIME
# --------------
AC_DEFUN([AC_HEADER_TIME],
[AC_CACHE_CHECK([whether time.h and sys/time.h may both be included],
  ac_cv_header_time,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
],
[if ((struct tm *) 0)
return 0;])],
                   [ac_cv_header_time=yes],
                   [ac_cv_header_time=no])])
if test $ac_cv_header_time = yes; then
  AC_DEFINE(TIME_WITH_SYS_TIME, 1,
            [Define if you can safely include both <sys/time.h> and <time.h>.])
fi
])# AC_HEADER_TIME


# _AC_HEADER_TIOCGWINSZ_IN_TERMIOS_H
# ----------------------------------
m4_define([_AC_HEADER_TIOCGWINSZ_IN_TERMIOS_H],
[AC_CACHE_CHECK([whether termios.h defines TIOCGWINSZ],
                ac_cv_sys_tiocgwinsz_in_termios_h,
[AC_EGREP_CPP([yes],
              [#include <sys/types.h>
#include <termios.h>
#ifdef TIOCGWINSZ
  yes
#endif
],
                ac_cv_sys_tiocgwinsz_in_termios_h=yes,
                ac_cv_sys_tiocgwinsz_in_termios_h=no)])
])# _AC_HEADER_TIOCGWINSZ_IN_TERMIOS_H


# _AC_HEADER_TIOCGWINSZ_IN_SYS_IOCTL
# ----------------------------------
m4_define([_AC_HEADER_TIOCGWINSZ_IN_SYS_IOCTL],
[AC_CACHE_CHECK([whether sys/ioctl.h defines TIOCGWINSZ],
                ac_cv_sys_tiocgwinsz_in_sys_ioctl_h,
[AC_EGREP_CPP([yes],
              [#include <sys/types.h>
#include <sys/ioctl.h>
#ifdef TIOCGWINSZ
  yes
#endif
],
                ac_cv_sys_tiocgwinsz_in_sys_ioctl_h=yes,
                ac_cv_sys_tiocgwinsz_in_sys_ioctl_h=no)])
])# _AC_HEADER_TIOCGWINSZ_IN_SYS_IOCTL


# AC_HEADER_TIOCGWINSZ
# --------------------
# Look for a header that defines TIOCGWINSZ.
# FIXME: Is this the proper name?  Is this the proper implementation?
# I need more help.
AC_DEFUN([AC_HEADER_TIOCGWINSZ],
[AC_REQUIRE([AC_SYS_POSIX_TERMIOS])dnl
if test $ac_cv_sys_posix_termios = yes; then
  _AC_HEADER_TIOCGWINSZ_IN_TERMIOS_H
fi
if test $ac_cv_sys_tiocgwinsz_in_termios_h != yes; then
  _AC_HEADER_TIOCGWINSZ_IN_SYS_IOCTL
  if test $ac_cv_sys_tiocgwinsz_in_sys_ioctl_h = yes; then
    AC_DEFINE(GWINSZ_IN_SYS_IOCTL,1,
              [Define if `TIOCGWINSZ' requires <sys/ioctl.h>])
  fi
fi
])# AC_HEADER_TIOCGWINSZ


# AU::AC_UNISTD_H
# ---------------
AU_DEFUN([AC_UNISTD_H],
[AC_CHECK_HEADERS(unistd.h)])


# AU::AC_USG
# ----------
# Define `USG' if string functions are in strings.h.
AU_DEFUN([AC_USG],
[AC_DIAGNOSE([obsolete],
[$0: Remove `AC_MSG_CHECKING', `AC_TRY_LINK' and this `AC_WARNING'
when you ajust your code to use HAVE_STRING_H.])dnl
AC_MSG_CHECKING([for BSD string and memory functions])
AC_TRY_LINK([@%:@include <strings.h>], [rindex(0, 0); bzero(0, 0);],
  [AC_MSG_RESULT(yes)],
  [AC_MSG_RESULT(no)
   AC_DEFINE(USG, 1,
       [Define if you do not have <strings.h>, index, bzero, etc...
        This symbol is obsolete, you should not depend upon it.])])
AC_CHECK_HEADERS(string.h)])


# AU::AC_MEMORY_H
# ---------------
# To be precise this macro used to be:
#
#   | AC_MSG_CHECKING(whether string.h declares mem functions)
#   | AC_EGREP_HEADER(memchr, string.h, ac_found=yes, ac_found=no)
#   | AC_MSG_RESULT($ac_found)
#   | if test $ac_found = no; then
#   | 	AC_CHECK_HEADER(memory.h, [AC_DEFINE(NEED_MEMORY_H)])
#   | fi
#
# But it is better to check for both headers, and alias NEED_MEMORY_H to
# HAVE_MEMORY_H.
AU_DEFUN([AC_MEMORY_H],
[AC_DIAGNOSE([obsolete], [$0: Remove this warning and
`AC_CHECK_HEADER(memory.h, AC_DEFINE(...))' when you ajust your code to
use and HAVE_STRING_H and HAVE_MEMORY_H, not NEED_MEMORY_H.])dnl
AC_CHECK_HEADER(memory.h,
                [AC_DEFINE([NEED_MEMORY_H], 1,
                           [Same as `HAVE_MEMORY_H', don't depend on me.])])
AC_CHECK_HEADERS(string.h memory.h)
])


# AU::AC_DIR_HEADER
# -----------------
# Like calling `AC_HEADER_DIRENT' and `AC_FUNC_CLOSEDIR_VOID', but
# defines a different set of C preprocessor macros to indicate which
# header file is found.
AU_DEFUN([AC_DIR_HEADER],
[AC_HEADER_DIRENT
AC_FUNC_CLOSEDIR_VOID
AC_DIAGNOSE([obsolete],
[$0: Remove this warning and the four `AC_DEFINE' when you
ajust your code to use `AC_HEADER_DIRENT'.])
test ac_cv_header_dirent_dirent_h &&
  AC_DEFINE([DIRENT], 1, [Same as `HAVE_DIRENT_H', don't depend on me.])
test ac_cv_header_dirent_sys_ndir_h &&
  AC_DEFINE([SYSNDIR], 1, [Same as `HAVE_SYS_NDIR_H', don't depend on me.])
test ac_cv_header_dirent_sys_dir_h &&
  AC_DEFINE([SYSDIR], 1, [Same as `HAVE_SYS_DIR_H', don't depend on me.])
test ac_cv_header_dirent_ndir_h &&
  AC_DEFINE([NDIR], 1, [Same as `HAVE_NDIR_H', don't depend on me.])
])
