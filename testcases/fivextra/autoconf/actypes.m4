# This file is part of Autoconf.                       -*- Autoconf -*-
# Type related macros: existence, sizeof, and structure members.
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


## ---------------- ##
## Type existence.  ##
## ---------------- ##

# ---------------- #
# General checks.  #
# ---------------- #

# Up to 2.13 included, Autoconf used to provide the macro
#
#    AC_CHECK_TYPE(TYPE, DEFAULT)
#
# Since, it provides another version which fits better with the other
# AC_CHECK_ families:
#
#    AC_CHECK_TYPE(TYPE,
#	           [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#	           [INCLUDES])
#
# In order to provide backward compatibility, the new scheme is
# implemented as _AC_CHECK_TYPE_NEW, the old scheme as _AC_CHECK_TYPE_OLD,
# and AC_CHECK_TYPE branches to one or the other, depending upon its
# arguments.



# _AC_CHECK_TYPE_NEW(TYPE,
#		     [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#		     [INCLUDES])
# ------------------------------------------------------------
# Check whether the type TYPE is supported by the system, maybe via the
# the provided includes.  This macro implements the former task of
# AC_CHECK_TYPE, with one big difference though: AC_CHECK_TYPE was
# grepping in the headers, which, BTW, led to many problems until
# the egrep expression was correct and did not given false positives.
# It turned out there are even portability issues with egrep...
#
# The most obvious way to check for a TYPE is just to compile a variable
# definition:
#
# 	  TYPE my_var;
#
# Unfortunately this does not work for const qualified types in C++,
# where you need an initializer.  So you think of
#
# 	  TYPE my_var = (TYPE) 0;
#
# Unfortunately, again, this is not valid for some C++ classes.
#
# Then you look for another scheme.  For instance you think of declaring
# a function which uses a parameter of type TYPE:
#
# 	  int foo (TYPE param);
#
# but of course you soon realize this does not make it with K&R
# compilers.  And by no ways you want to
#
# 	  int foo (param)
# 	    TYPE param
# 	  { ; }
#
# since this time it's C++ who is not happy.
#
# Don't even think of the return type of a function, since K&R cries
# there too.  So you start thinking of declaring a *pointer* to this TYPE:
#
# 	  TYPE *p;
#
# but you know fairly well that this is legal in C for aggregates which
# are unknown (TYPE = struct does-not-exist).
#
# Then you think of using sizeof to make sure the TYPE is really
# defined:
#
# 	  sizeof (TYPE);
#
# But this succeeds if TYPE is a variable: you get the size of the
# variable's type!!!
#
# This time you tell yourself the last two options *together* will make
# it.  And indeed this is the solution invented by Alexandre Oliva.
#
# Also note that we use
#
# 	  if (sizeof (TYPE))
#
# to `read' sizeof (to avoid warnings), while not depending on its type
# (not necessarily size_t etc.).  Equally, instead of defining an unused
# variable, we just use a cast to avoid warnings from the compiler.
# Suggested by Paul Eggert.
m4_define([_AC_CHECK_TYPE_NEW],
[AS_VAR_PUSHDEF([ac_Type], [ac_cv_type_$1])dnl
AC_CACHE_CHECK([for $1], ac_Type,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT([$4])],
[if (($1 *) 0)
  return 0;
if (sizeof ($1))
  return 0;])],
                   [AS_VAR_SET(ac_Type, yes)],
                   [AS_VAR_SET(ac_Type, no)])])
AS_IF([test AS_VAR_GET(ac_Type) = yes], [$2], [$3])[]dnl
AS_VAR_POPDEF([ac_Type])dnl
])# _AC_CHECK_TYPE_NEW


# AC_CHECK_TYPES(TYPES,
#                [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#                [INCLUDES])
# --------------------------------------------------------
# TYPES is an m4 list.  There are no ambiguities here, we mean the newer
# AC_CHECK_TYPE.
AC_DEFUN([AC_CHECK_TYPES],
[m4_foreach([AC_Type], [$1],
  [_AC_CHECK_TYPE_NEW(AC_Type,
                      [AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_[]AC_Type), 1,
                                          [Define if the system has the type
                                          `]AC_Type['.])
$2],
                      [$3],
                      [$4])])])


# _AC_CHECK_TYPE_OLD(TYPE, DEFAULT)
# ---------------------------------
# FIXME: This is an extremely badly chosen name, since this
# macro actually performs an AC_REPLACE_TYPE.  Some day we
# have to clean this up.
m4_define([_AC_CHECK_TYPE_OLD],
[_AC_CHECK_TYPE_NEW([$1],,
   [AC_DEFINE_UNQUOTED([$1], [$2],
                       [Define to `$2' if <sys/types.h> does not define.])])dnl
])# _AC_CHECK_TYPE_OLD


# _AC_CHECK_TYPE_REPLACEMENT_TYPE_P(STRING)
# -----------------------------------------
# Return `1' if STRING seems to be a builtin C/C++ type, i.e., if it
# starts with `_Bool', `bool', `char', `double', `float', `int',
# `long', `short', `signed', or `unsigned' followed by characters
# that are defining types.
# Because many people have used `off_t' and `size_t' too, they are added
# for better common-useward backward compatibility.
m4_define([_AC_CHECK_TYPE_REPLACEMENT_TYPE_P],
[m4_if(m4_regexp([$1],
                 [^\(_Bool\|bool\|char\|double\|float\|int\|long\|short\|\(un\)?signed\|[_a-zA-Z][_a-zA-Z0-9]*_t\)[][_a-zA-Z0-9() *]*$]),
       0, 1, 0)dnl
])# _AC_CHECK_TYPE_REPLACEMENT_TYPE_P


# _AC_CHECK_TYPE_MAYBE_TYPE_P(STRING)
# -----------------------------------
# Return `1' if STRING looks like a C/C++ type.
m4_define([_AC_CHECK_TYPE_MAYBE_TYPE_P],
[m4_if(m4_regexp([$1], [^[_a-zA-Z0-9 ]+\([_a-zA-Z0-9() *]\|\[\|\]\)*$]),
       0, 1, 0)dnl
])# _AC_CHECK_TYPE_MAYBE_TYPE_P


# AC_CHECK_TYPE(TYPE, DEFAULT)
#  or
# AC_CHECK_TYPE(TYPE,
#	        [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#	        [INCLUDES])
# -------------------------------------------------------
#
# Dispatch respectively to _AC_CHECK_TYPE_OLD or _AC_CHECK_TYPE_NEW.
# 1. More than two arguments         => NEW
# 2. $2 seems to be replacement type => OLD
#    See _AC_CHECK_TYPE_REPLACEMENT_TYPE_P for `replacement type'.
# 3. $2 seems to be a type           => NEW plus a warning
# 4. default                         => NEW
AC_DEFUN([AC_CHECK_TYPE],
[m4_if($#, 3,
         [_AC_CHECK_TYPE_NEW($@)],
       $#, 4,
         [_AC_CHECK_TYPE_NEW($@)],
       _AC_CHECK_TYPE_REPLACEMENT_TYPE_P([$2]), 1,
         [_AC_CHECK_TYPE_OLD($@)],
       _AC_CHECK_TYPE_MAYBE_TYPE_P([$2]), 1,
         [AC_DIAGNOSE([syntax],
                    [$0: assuming `$2' is not a type])_AC_CHECK_TYPE_NEW($@)],
       [_AC_CHECK_TYPE_NEW($@)])[]dnl
])# AC_CHECK_TYPE



# ----------------- #
# Specific checks.  #
# ----------------- #

# AC_TYPE_GETGROUPS
# -----------------
AC_DEFUN([AC_TYPE_GETGROUPS],
[AC_REQUIRE([AC_TYPE_UID_T])dnl
AC_CACHE_CHECK(type of array argument to getgroups, ac_cv_type_getgroups,
[AC_RUN_IFELSE([AC_LANG_SOURCE(
[[/* Thanks to Mike Rendell for this test.  */
#include <sys/types.h>
#define NGID 256
#undef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))

int
main ()
{
  gid_t gidset[NGID];
  int i, n;
  union { gid_t gval; long lval; }  val;

  val.lval = -1;
  for (i = 0; i < NGID; i++)
    gidset[i] = val.gval;
  n = getgroups (sizeof (gidset) / MAX (sizeof (int), sizeof (gid_t)) - 1,
                 gidset);
  /* Exit non-zero if getgroups seems to require an array of ints.  This
     happens when gid_t is short but getgroups modifies an array of ints.  */
  exit ((n > 0 && gidset[n] != val.gval) ? 1 : 0);
}]])],
               [ac_cv_type_getgroups=gid_t],
               [ac_cv_type_getgroups=int],
               [ac_cv_type_getgroups=cross])
if test $ac_cv_type_getgroups = cross; then
  dnl When we can't run the test program (we are cross compiling), presume
  dnl that <unistd.h> has either an accurate prototype for getgroups or none.
  dnl Old systems without prototypes probably use int.
  AC_EGREP_HEADER([getgroups.*int.*gid_t], unistd.h,
		  ac_cv_type_getgroups=gid_t, ac_cv_type_getgroups=int)
fi])
AC_DEFINE_UNQUOTED(GETGROUPS_T, $ac_cv_type_getgroups,
                   [Define to the type of elements in the array set by
                    `getgroups'. Usually this is either `int' or `gid_t'.])
])# AC_TYPE_GETGROUPS


# AU::AM_TYPE_PTRDIFF_T
AU_DEFUN([AM_TYPE_PTRDIFF_T],
[AC_CHECK_TYPES(ptrdiff_t)])


# AC_TYPE_UID_T
# -------------
# FIXME: Rewrite using AC_CHECK_TYPE.
AC_DEFUN([AC_TYPE_UID_T],
[AC_CACHE_CHECK(for uid_t in sys/types.h, ac_cv_type_uid_t,
[AC_EGREP_HEADER(uid_t, sys/types.h,
  ac_cv_type_uid_t=yes, ac_cv_type_uid_t=no)])
if test $ac_cv_type_uid_t = no; then
  AC_DEFINE(uid_t, int, [Define to `int' if <sys/types.h> doesn't define.])
  AC_DEFINE(gid_t, int, [Define to `int' if <sys/types.h> doesn't define.])
fi
])


AC_DEFUN([AC_TYPE_SIZE_T], [AC_CHECK_TYPE(size_t, unsigned)])
AC_DEFUN([AC_TYPE_PID_T],  [AC_CHECK_TYPE(pid_t,  int)])
AC_DEFUN([AC_TYPE_OFF_T],  [AC_CHECK_TYPE(off_t,  long)])
AC_DEFUN([AC_TYPE_MODE_T], [AC_CHECK_TYPE(mode_t, int)])


# AC_TYPE_SIGNAL
# --------------
# Note that identifiers starting with SIG are reserved by ANSI C.
AC_DEFUN([AC_TYPE_SIGNAL],
[AC_CACHE_CHECK([return type of signal handlers], ac_cv_type_signal,
[AC_COMPILE_IFELSE(
[AC_LANG_PROGRAM([#include <sys/types.h>
#include <signal.h>
#ifdef signal
# undef signal
#endif
#ifdef __cplusplus
extern "C" void (*signal (int, void (*)(int)))(int);
#else
void (*signal ()) ();
#endif
],
                 [int i;])],
                   [ac_cv_type_signal=void],
                   [ac_cv_type_signal=int])])
AC_DEFINE_UNQUOTED(RETSIGTYPE, $ac_cv_type_signal,
                   [Define as the return type of signal handlers
                    (`int' or `void').])
])


## ------------------------ ##
## Checking size of types.  ##
## ------------------------ ##

# ---------------- #
# Generic checks.  #
# ---------------- #


# AC_CHECK_SIZEOF(TYPE, [IGNORED], [INCLUDES])
# --------------------------------------------
AC_DEFUN([AC_CHECK_SIZEOF],
[AS_LITERAL_IF([$1], [],
               [AC_FATAL([$0: requires literal arguments])])dnl
AC_CHECK_TYPE([$1], [], [], [$3])
AC_CACHE_CHECK([size of $1], AS_TR_SH([ac_cv_sizeof_$1]),
[if test "$AS_TR_SH([ac_cv_type_$1])" = yes; then
  _AC_COMPUTE_INT([sizeof ($1)],
                  [AS_TR_SH([ac_cv_sizeof_$1])],
                  [AC_INCLUDES_DEFAULT([$3])])
else
  AS_TR_SH([ac_cv_sizeof_$1])=0
fi])dnl
AC_DEFINE_UNQUOTED(AS_TR_CPP(sizeof_$1), $AS_TR_SH([ac_cv_sizeof_$1]),
                   [The size of a `$1', as computed by sizeof.])
])# AC_CHECK_SIZEOF



# ---------------- #
# Generic checks.  #
# ---------------- #

# AU::AC_INT_16_BITS
# ------------------
# What a great name :)
AU_DEFUN([AC_INT_16_BITS],
[AC_CHECK_SIZEOF([int])
AC_DIAGNOSE([obsolete], [$0:
        your code should no longer depend upon `INT_16_BITS', but upon
        `SIZEOF_INT'.  Remove this warning and the `AC_DEFINE' when you
        adjust the code.])dnl
test $ac_cv_sizeof_int = 2 &&
  AC_DEFINE(INT_16_BITS, 1,
            [Define if `sizeof (int)' = 2.  Obsolete, use `SIZEOF_INT'.])
])


# AU::AC_LONG_64_BITS
# -------------------
AU_DEFUN([AC_LONG_64_BITS],
[AC_CHECK_SIZEOF([long int])
AC_DIAGNOSE([obsolete], [$0:
        your code should no longer depend upon `LONG_64_BITS', but upon
        `SIZEOF_LONG_INT'.  Remove this warning and the `AC_DEFINE' when
        you adjust the code.])dnl
test $ac_cv_sizeof_long_int = 8 &&
  AC_DEFINE(LONG_64_BITS, 1,
            [Define if `sizeof (long int)' = 8.  Obsolete, use
             `SIZEOF_LONG_INT'.])
])



## -------------------------- ##
## Generic structure checks.  ##
## -------------------------- ##


# ---------------- #
# Generic checks.  #
# ---------------- #

# AC_CHECK_MEMBER(AGGREGATE.MEMBER,
#                 [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#                 [INCLUDES])
# ---------------------------------------------------------
# AGGREGATE.MEMBER is for instance `struct passwd.pw_gecos', shell
# variables are not a valid argument.
AC_DEFUN([AC_CHECK_MEMBER],
[AS_LITERAL_IF([$1], [],
               [AC_FATAL([$0: requires literal arguments])])dnl
m4_if(m4_regexp([$1], [\.]), -1,
      [AC_FATAL([$0: Did not see any dot in `$1'])])dnl
AS_VAR_PUSHDEF([ac_Member], [ac_cv_member_$1])dnl
dnl Extract the aggregate name, and the member name
AC_CACHE_CHECK([for $1], ac_Member,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT([$4])],
[dnl AGGREGATE ac_aggr;
static m4_patsubst([$1], [\..*]) ac_aggr;
dnl ac_aggr.MEMBER;
if (ac_aggr.m4_patsubst([$1], [^[^.]*\.]))
return 0;])],
                [AS_VAR_SET(ac_Member, yes)],
                [AS_VAR_SET(ac_Member, no)])])
AS_IF([test AS_VAR_GET(ac_Member) = yes], [$2], [$3])dnl
AS_VAR_POPDEF([ac_Member])dnl
])# AC_CHECK_MEMBER


# AC_CHECK_MEMBERS([AGGREGATE.MEMBER, ...],
#                  [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND]
#                  [INCLUDES])
# ---------------------------------------------------------
# The first argument is an m4 list.
AC_DEFUN([AC_CHECK_MEMBERS],
[m4_foreach([AC_Member], [$1],
  [AC_CHECK_MEMBER(AC_Member,
         [AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_[]AC_Member), 1,
                            [Define if `]m4_patsubst(AC_Member,
                                                     [^[^.]*\.])[' is
                             member of `]m4_patsubst(AC_Member, [\..*])['.])
$2],
                 [$3],
                 [$4])])])


# ----------------- #
# Specific checks.  #
# ----------------- #

# Alphabetic order, please.

# AC_STRUCT_ST_BLKSIZE
# --------------------
AU_DEFUN([AC_STRUCT_ST_BLKSIZE],
[AC_DIAGNOSE([obsolete], [$0:
        your code should no longer depend upon `HAVE_ST_BLKSIZE', but
        `HAVE_STRUCT_STAT_ST_BLKSIZE'.  Remove this warning and
        the `AC_DEFINE' when you adjust the code.])
AC_CHECK_MEMBERS([struct stat.st_blksize],
                 [AC_DEFINE(HAVE_ST_BLKSIZE, 1,
                            [Define if your `struct stat' has
                             `st_blksize'.  Deprecated, use
                             `HAVE_STRUCT_STAT_ST_BLKSIZE' instead.])])
])# AC_STRUCT_ST_BLKSIZE


# AC_STRUCT_ST_BLOCKS
# -------------------
# If `struct stat' contains an `st_blocks' member, define
# HAVE_STRUCT_STAT_ST_BLOCKS.  Otherwise, add `fileblocks.o' to the
# output variable LIBOBJS.  We still define HAVE_ST_BLOCKS for backward
# compatibility.  In the future, we will activate specializations for
# this macro, so don't obsolete it right now.
#
# AC_OBSOLETE([$0], [; replace it with
#   AC_CHECK_MEMBERS([struct stat.st_blocks],
#                     [AC_LIBOBJ([fileblocks])])
# Please note that it will define `HAVE_STRUCT_STAT_ST_BLOCKS',
# and not `HAVE_ST_BLOCKS'.])dnl
#
AC_DEFUN([AC_STRUCT_ST_BLOCKS],
[AC_CHECK_MEMBERS([struct stat.st_blocks],
                  [AC_DEFINE(HAVE_ST_BLOCKS, 1,
                             [Define if your `struct stat' has
                              `st_blocks'.  Deprecated, use
                              `HAVE_STRUCT_STAT_ST_BLOCKS' instead.])],
                  [AC_LIBOBJ([fileblocks])])
])# AC_STRUCT_ST_BLOCKS


# AC_STRUCT_ST_RDEV
# -----------------
AU_DEFUN([AC_STRUCT_ST_RDEV],
[AC_DIAGNOSE([obsolete], [$0:
        your code should no longer depend upon `HAVE_ST_RDEV', but
        `HAVE_STRUCT_STAT_ST_RDEV'.  Remove this warning and
        the `AC_DEFINE' when you adjust the code.])
AC_CHECK_MEMBERS([struct stat.st_rdev],
                 [AC_DEFINE(HAVE_ST_RDEV, 1,
                            [Define if your `struct stat' has `st_rdev'.
                             Deprecated, use `HAVE_STRUCT_STAT_ST_RDEV'
                             instead.])])
])# AC_STRUCT_ST_RDEV


# AC_STRUCT_TM
# ------------
# FIXME: This macro is badly named, it should be AC_CHECK_TYPE_STRUCT_TM.
# Or something else, but what? AC_CHECK_TYPE_STRUCT_TM_IN_SYS_TIME?
AC_DEFUN([AC_STRUCT_TM],
[AC_CACHE_CHECK([whether struct tm is in sys/time.h or time.h],
  ac_cv_struct_tm,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <sys/types.h>
#include <time.h>
],
                                    [struct tm *tp; tp->tm_sec;])],
                   [ac_cv_struct_tm=time.h],
                   [ac_cv_struct_tm=sys/time.h])])
if test $ac_cv_struct_tm = sys/time.h; then
  AC_DEFINE(TM_IN_SYS_TIME, 1,
            [Define if your <sys/time.h> declares `struct tm'.])
fi
])# AC_STRUCT_TM


# AC_STRUCT_TIMEZONE
# ------------------
# Figure out how to get the current timezone.  If `struct tm' has a
# `tm_zone' member, define `HAVE_TM_ZONE'.  Otherwise, if the
# external array `tzname' is found, define `HAVE_TZNAME'.
AC_DEFUN([AC_STRUCT_TIMEZONE],
[AC_REQUIRE([AC_STRUCT_TM])dnl
AC_CHECK_MEMBERS([struct tm.tm_zone],,,[#include <sys/types.h>
#include <$ac_cv_struct_tm>
])
if test "$ac_cv_member_struct_tm_tm_zone" = yes; then
  AC_DEFINE(HAVE_TM_ZONE, 1,
            [Define if your `struct tm' has `tm_zone'. Deprecated, use
             `HAVE_STRUCT_TM_TM_ZONE' instead.])
else
  AC_CACHE_CHECK(for tzname, ac_cv_var_tzname,
[AC_TRY_LINK(
[#include <time.h>
#ifndef tzname /* For SGI.  */
extern char *tzname[]; /* RS6000 and others reject char **tzname.  */
#endif
],
[atoi(*tzname);], ac_cv_var_tzname=yes, ac_cv_var_tzname=no)])
  if test $ac_cv_var_tzname = yes; then
    AC_DEFINE(HAVE_TZNAME, 1,
              [Define if you don't have `tm_zone' but do have the external
               array `tzname'.])
  fi
fi
])# AC_STRUCT_TIMEZONE
