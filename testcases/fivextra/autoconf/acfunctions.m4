# This file is part of Autoconf.                       -*- Autoconf -*-
# Checking for functions.
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
# 1. Generic tests for functions.
# 2. Tests for specific functions.


## -------------------------------- ##
## 1. Generic tests for functions.  ##
## -------------------------------- ##


# AC_CHECK_FUNC(FUNCTION, [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# -----------------------------------------------------------------
AC_DEFUN([AC_CHECK_FUNC],
[AS_VAR_PUSHDEF([ac_var], [ac_cv_func_$1])dnl
AC_CACHE_CHECK([for $1], ac_var,
[AC_LINK_IFELSE([AC_LANG_FUNC_LINK_TRY([$1])],
                [AS_VAR_SET(ac_var, yes)],
                [AS_VAR_SET(ac_var, no)])])
AS_IF([test AS_VAR_GET(ac_var) = yes], [$2], [$3])dnl
AS_VAR_POPDEF([ac_var])dnl
])# AC_CHECK_FUNC


# AC_CHECK_FUNCS(FUNCTION..., [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# ---------------------------------------------------------------------
AC_DEFUN([AC_CHECK_FUNCS],
[AC_FOREACH([AC_Func], [$1],
  [AH_TEMPLATE(AS_TR_CPP(HAVE_[]AC_Func),
               [Define if you have the `]AC_Func[' function.])])dnl
for ac_func in $1
do
AC_CHECK_FUNC($ac_func,
              [AC_DEFINE_UNQUOTED([AS_TR_CPP([HAVE_$ac_func])]) $2],
              [$3])dnl
done
])


# AC_REPLACE_FUNCS(FUNCTION...)
# -----------------------------
AC_DEFUN([AC_REPLACE_FUNCS],
[AC_FOREACH([AC_Func], [$1], [AC_LIBSOURCE(AC_Func.c)])dnl
AC_CHECK_FUNCS([$1], , [_AC_LIBOBJ($ac_func)])
])


# AU::AC_FUNC_CHECK
# -----------------
AU_ALIAS([AC_FUNC_CHECK], [AC_CHECK_FUNC])


# AU::AC_HAVE_FUNCS
# -----------------
AU_ALIAS([AC_HAVE_FUNCS], [AC_CHECK_FUNCS])




## --------------------------------- ##
## 2. Tests for specific functions.  ##
## --------------------------------- ##


# The macros are sorted:
#
# 1. AC_FUNC_* macros are sorted by alphabetical order.
#
# 2. Helping macros such as _AC_LIBOBJ_* are before the macro that
#    uses it.
#
# 3. Obsolete macros are right after the modern macro.



# _AC_LIBOBJ_ALLOCA
# -----------------
# Set up the LIBOBJ replacement of `alloca'.  Well, not exactly
# AC_LIBOBJ since we actually set the output variable `ALLOCA'.
# Nevertheless, for Automake, AC_LIBSOURCES it.
m4_define([_AC_LIBOBJ_ALLOCA],
[# The SVR3 libPW and SVR4 libucb both contain incompatible functions
# that cause trouble.  Some versions do not even contain alloca or
# contain a buggy version.  If you still want to use their alloca,
# use ar to extract alloca.o from them instead of compiling alloca.c.
AC_LIBSOURCES(alloca.c)
AC_SUBST(ALLOCA, alloca.$ac_objext)dnl
AC_DEFINE(C_ALLOCA, 1, [Define if using `alloca.c'.])

AC_CACHE_CHECK(whether `alloca.c' needs Cray hooks, ac_cv_os_cray,
[AC_EGREP_CPP(webecray,
[#if defined(CRAY) && ! defined(CRAY2)
webecray
#else
wenotbecray
#endif
], ac_cv_os_cray=yes, ac_cv_os_cray=no)])
if test $ac_cv_os_cray = yes; then
  for ac_func in _getb67 GETB67 getb67; do
    AC_CHECK_FUNC($ac_func,
  		  [AC_DEFINE_UNQUOTED(CRAY_STACKSEG_END, $ac_func,
  				      [Define to one of `_getb67', `GETB67',
  				       `getb67' for Cray-2 and Cray-YMP
                                       systems. This function is required for
                                       `alloca.c' support on those systems.])
    break])
  done
fi

AC_CACHE_CHECK([stack direction for C alloca],
               [ac_cv_c_stack_direction],
[AC_RUN_IFELSE([AC_LANG_SOURCE(
[int
find_stack_direction ()
{
  static char *addr = 0;
  auto char dummy;
  if (addr == 0)
    {
      addr = &dummy;
      return find_stack_direction ();
    }
  else
    return (&dummy > addr) ? 1 : -1;
}

int
main ()
{
  exit (find_stack_direction () < 0);
}])],
               [ac_cv_c_stack_direction=1],
               [ac_cv_c_stack_direction=-1],
               [ac_cv_c_stack_direction=0])])
AH_VERBATIM([STACK_DIRECTION],
[/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
        STACK_DIRECTION > 0 => grows toward higher addresses
        STACK_DIRECTION < 0 => grows toward lower addresses
        STACK_DIRECTION = 0 => direction of growth unknown */
@%:@undef STACK_DIRECTION])dnl
AC_DEFINE_UNQUOTED(STACK_DIRECTION, $ac_cv_c_stack_direction)
])# _AC_LIBOBJ_ALLOCA


# AC_FUNC_ALLOCA
# --------------
AC_DEFUN([AC_FUNC_ALLOCA],
[# The Ultrix 4.2 mips builtin alloca declared by alloca.h only works
# for constant arguments.  Useless!
AC_CACHE_CHECK([for working alloca.h], ac_cv_working_alloca_h,
[AC_TRY_LINK([@%:@include <alloca.h>],
  [char *p = (char *) alloca (2 * sizeof (int));],
  ac_cv_working_alloca_h=yes, ac_cv_working_alloca_h=no)])
if test $ac_cv_working_alloca_h = yes; then
  AC_DEFINE(HAVE_ALLOCA_H, 1,
            [Define if you have <alloca.h> and it should be used
             (not on Ultrix).])
fi

AC_CACHE_CHECK([for alloca], ac_cv_func_alloca_works,
[AC_TRY_LINK(
[#ifdef __GNUC__
# define alloca __builtin_alloca
#else
# ifdef _MSC_VER
#  include <malloc.h>
#  define alloca _alloca
# else
#  if HAVE_ALLOCA_H
#   include <alloca.h>
#  else
#   ifdef _AIX
 #pragma alloca
#   else
#    ifndef alloca /* predefined by HP cc +Olibcalls */
char *alloca ();
#    endif
#   endif
#  endif
# endif
#endif
], [char *p = (char *) alloca (1);],
  ac_cv_func_alloca_works=yes, ac_cv_func_alloca_works=no)])

if test $ac_cv_func_alloca_works = yes; then
  AC_DEFINE(HAVE_ALLOCA, 1,
            [Define if you have `alloca', as a function or macro.])
else
  _AC_LIBOBJ_ALLOCA
fi
])# AC_FUNC_ALLOCA


# AU::AC_ALLOCA
# -------------
AU_ALIAS([AC_ALLOCA], [AC_FUNC_ALLOCA])


# AC_FUNC_CHOWN
# -------------
# Determine whether chown accepts arguments of -1 for uid and gid.
AC_DEFUN([AC_FUNC_CHOWN],
[AC_REQUIRE([AC_TYPE_UID_T])dnl
AC_CHECK_HEADERS(unistd.h)
AC_CACHE_CHECK([for working chown], ac_cv_func_chown_works,
[AC_RUN_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT
#include <fcntl.h>
],
[[  char *f = "conftest.chown";
  struct stat before, after;

  if (creat (f, 0600) < 0)
    exit (1);
  if (stat (f, &before) < 0)
    exit (1);
  if (chown (f, (uid_t) -1, (gid_t) -1) == -1)
    exit (1);
  if (stat (f, &after) < 0)
    exit (1);
  exit ((before.st_uid == after.st_uid
         && before.st_gid == after.st_gid) ? 0 : 1);
]])],
               [ac_cv_func_chown_works=yes],
               [ac_cv_func_chown_works=no],
               [ac_cv_func_chown_works=no])
rm -f conftest.chown
])
if test $ac_cv_func_chown_works = yes; then
  AC_DEFINE(HAVE_CHOWN, 1,
            [Define if your system has a working `chown' function.])
fi
])# AC_FUNC_CHOWN


# AC_FUNC_CLOSEDIR_VOID
# ---------------------
# Check whether closedir returns void, and #define CLOSEDIR_VOID in
# that case.
AC_DEFUN([AC_FUNC_CLOSEDIR_VOID],
[AC_REQUIRE([AC_HEADER_DIRENT])dnl
AC_CACHE_CHECK([whether closedir returns void],
               [ac_cv_func_closedir_void],
[AC_RUN_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT
#include <$ac_header_dirent>
#ifndef __cplusplus
int closedir ();
#endif
],
                                [[exit (closedir (opendir (".")) != 0);]])],
               [ac_cv_func_closedir_void=no],
               [ac_cv_func_closedir_void=yes],
               [ac_cv_func_closedir_void=yes])])
if test $ac_cv_func_closedir_void = yes; then
  AC_DEFINE(CLOSEDIR_VOID, 1,
            [Define if the `closedir' function returns void instead of `int'.])
fi
])


# AC_FUNC_ERROR_AT_LINE
# ---------------------
AC_DEFUN([AC_FUNC_ERROR_AT_LINE],
[AC_LIBSOURCES([error.h, error.c])dnl
AC_CACHE_CHECK([for error_at_line], ac_cv_lib_error_at_line,
[AC_TRY_LINK([],[error_at_line (0, 0, "", 0, "");],
             [ac_cv_lib_error_at_line=yes],
             [ac_cv_lib_error_at_line=no])])
if test $ac_cv_lib_error_at_line = no; then
  AC_LIBOBJ(error)
fi
])


# AU::AM_FUNC_ERROR_AT_LINE
# -------------------------
AU_ALIAS([AM_FUNC_ERROR_AT_LINE], [AC_FUNC_ERROR_AT_LINE])


# AC_FUNC_FNMATCH
# ---------------
# We look for fnmatch.h to avoid that the test fails in C++.
AC_DEFUN([AC_FUNC_FNMATCH],
[AC_CACHE_CHECK([for working GNU-style fnmatch],
                [ac_cv_func_fnmatch_works],
# Some versions of Solaris, SCO, and the GNU C Library
# have a broken or incompatible fnmatch.
# So we run a test program.  If we are cross-compiling, take no chance.
# Thanks to John Oleynick, Franc,ois Pinard, and Paul Eggert for this test.
[AC_RUN_IFELSE([AC_LANG_PROGRAM([@%:@include <fnmatch.h>],
 [exit (fnmatch ("a*", "abc", 0) != 0
	|| fnmatch ("d*/*1", "d/s/1", FNM_FILE_NAME) != FNM_NOMATCH
	|| fnmatch ("*", "x", FNM_FILE_NAME | FNM_LEADING_DIR) != 0
	|| fnmatch ("x*", "x/y/z", FNM_FILE_NAME | FNM_LEADING_DIR) != 0
	|| fnmatch ("*c*", "c/x", FNM_FILE_NAME | FNM_LEADING_DIR) != 0);])],
               [ac_cv_func_fnmatch_works=yes],
               [ac_cv_func_fnmatch_works=no],
               [ac_cv_func_fnmatch_works=no])])
if test $ac_cv_func_fnmatch_works = yes; then
  AC_DEFINE(HAVE_FNMATCH, 1,
            [Define if your system has a working `fnmatch' function.])
fi
])# AC_FUNC_FNMATCH


# AU::AM_FUNC_FNMATCH
# AU::fp_FUNC_FNMATCH
# -------------------
AU_ALIAS([AM_FUNC_FNMATCH], [AC_FUNC_FNMATCH])
AU_ALIAS([fp_FUNC_FNMATCH], [AC_FUNC_FNMATCH])


# AC_FUNC_FSEEKO
# --------------
AC_DEFUN([AC_FUNC_FSEEKO],
[_AC_SYS_LARGEFILE_MACRO_VALUE(_LARGEFILE_SOURCE, 1,
   [ac_cv_sys_largefile_source],
   [Define to make fseeko visible on some hosts (e.g. glibc 2.2).],
   [@%:@include <stdio.h>], [return !fseeko;])

# We used to try defining _XOPEN_SOURCE=500 too, to work around a bug
# in glibc 2.1.3, but that breaks too many other things.
# If you want fseeko and ftello with glibc, upgrade to a fixed glibc.
AC_CACHE_CHECK([for fseeko], [ac_cv_func_fseeko],
               [AC_TRY_LINK([@%:@include <stdio.h>],
                            [return fseeko && fseeko (stdin, 0, 0);],
                            [ac_cv_func_fseeko=yes],
                            [ac_cv_func_fseeko=no])])
if test $ac_cv_func_fseeko = yes; then
  AC_DEFINE(HAVE_FSEEKO, 1,
    [Define if fseeko (and presumably ftello) exists and is declared.])
fi
])# AC_FUNC_FSEEKO


# AC_FUNC_GETGROUPS
# -----------------
# Try to find `getgroups', and check that it works.
# When crosscompiling, assume getgroups is broken.
AC_DEFUN([AC_FUNC_GETGROUPS],
[AC_REQUIRE([AC_TYPE_GETGROUPS])dnl
AC_REQUIRE([AC_TYPE_SIZE_T])dnl
AC_CHECK_FUNC(getgroups)

# If we don't yet have getgroups, see if it's in -lbsd.
# This is reported to be necessary on an ITOS 3000WS running SEIUX 3.1.
ac_save_LIBS=$LIBS
if test $ac_cv_func_getgroups = no; then
  AC_CHECK_LIB(bsd, getgroups, [GETGROUPS_LIB=-lbsd])
fi

# Run the program to test the functionality of the system-supplied
# getgroups function only if there is such a function.
if test $ac_cv_func_getgroups = yes; then
  AC_CACHE_CHECK([for working getgroups], ac_cv_func_getgroups_works,
   [AC_RUN_IFELSE([AC_LANG_PROGRAM([],
      [[/* On Ultrix 4.3, getgroups (0, 0) always fails.  */
       exit (getgroups (0, 0) == -1 ? 1 : 0);]])],
                  [ac_cv_func_getgroups_works=yes],
                  [ac_cv_func_getgroups_works=no],
                  [ac_cv_func_getgroups_works=no])
   ])
  if test $ac_cv_func_getgroups_works = yes; then
    AC_DEFINE(HAVE_GETGROUPS, 1,
              [Define if your system has a working `getgroups' function.])
  fi
fi
LIBS=$ac_save_LIBS
])# AC_FUNC_GETGROUPS


# _AC_LIBOBJ_GETLOADAVG
# ---------------------
# Set up the AC_LIBOBJ replacement of `getloadavg'.
m4_define([_AC_LIBOBJ_GETLOADAVG],
[AC_LIBOBJ(getloadavg)
AC_DEFINE(C_GETLOADAVG, 1, [Define if using `getloadavg.c'.])
# Figure out what our getloadavg.c needs.
ac_have_func=no
AC_CHECK_HEADER(sys/dg_sys_info.h,
[ac_have_func=yes
 AC_DEFINE(DGUX, 1, [Define for DGUX with <sys/dg_sys_info.h>.])
 AC_CHECK_LIB(dgc, dg_sys_info)])

AC_CHECK_HEADER(locale.h)
AC_CHECK_FUNCS(setlocale)

# We cannot check for <dwarf.h>, because Solaris 2 does not use dwarf (it
# uses stabs), but it is still SVR4.  We cannot check for <elf.h> because
# Irix 4.0.5F has the header but not the library.
if test $ac_have_func = no && test "$ac_cv_lib_elf_elf_begin" = yes; then
  ac_have_func=yes
  AC_DEFINE(SVR4, 1, [Define on System V Release 4.])
fi

if test $ac_have_func = no; then
  AC_CHECK_HEADER(inq_stats/cpustats.h,
  [ac_have_func=yes
   AC_DEFINE(UMAX, 1, [Define for Encore UMAX.])
   AC_DEFINE(UMAX4_3, 1,
             [Define for Encore UMAX 4.3 that has <inq_status/cpustats.h>
              instead of <sys/cpustats.h>.])])
fi

if test $ac_have_func = no; then
  AC_CHECK_HEADER(sys/cpustats.h,
  [ac_have_func=yes; AC_DEFINE(UMAX)])
fi

if test $ac_have_func = no; then
  AC_CHECK_HEADERS(mach/mach.h)
fi

AC_CHECK_HEADERS(nlist.h,
[AC_CHECK_MEMBERS([struct nlist.n_un.n_name],
                  [AC_DEFINE(NLIST_NAME_UNION, 1,
                             [Define if your `struct nlist' has an
                              `n_un' member.  Obsolete, depend on
                              `HAVE_STRUCT_NLIST_N_UN_N_NAME])], [],
                  [@%:@include <nlist.h>])
])dnl
])# _AC_LIBOBJ_GETLOADAVG


# AC_FUNC_GETLOADAVG
# ------------------
AC_DEFUN([AC_FUNC_GETLOADAVG],
[ac_have_func=no # yes means we've found a way to get the load average.

ac_save_LIBS=$LIBS

# Check for getloadavg, but be sure not to touch the cache variable.
(AC_CHECK_FUNC(getloadavg, exit 0, exit 1)) && ac_have_func=yes

# On HPUX9, an unprivileged user can get load averages through this function.
AC_CHECK_FUNCS(pstat_getdynamic)

# Solaris has libkstat which does not require root.
AC_CHECK_LIB(kstat, kstat_open)
test $ac_cv_lib_kstat_kstat_open = yes && ac_have_func=yes

# Some systems with -lutil have (and need) -lkvm as well, some do not.
# On Solaris, -lkvm requires nlist from -lelf, so check that first
# to get the right answer into the cache.
# For kstat on solaris, we need libelf to force the definition of SVR4 below.
if test $ac_have_func = no; then
  AC_CHECK_LIB(elf, elf_begin, LIBS="-lelf $LIBS")
fi
if test $ac_have_func = no; then
  AC_CHECK_LIB(kvm, kvm_open, LIBS="-lkvm $LIBS")
  # Check for the 4.4BSD definition of getloadavg.
  AC_CHECK_LIB(util, getloadavg,
    [LIBS="-lutil $LIBS" ac_have_func=yes ac_cv_func_getloadavg_setgid=yes])
fi

if test $ac_have_func = no; then
  # There is a commonly available library for RS/6000 AIX.
  # Since it is not a standard part of AIX, it might be installed locally.
  ac_getloadavg_LIBS=$LIBS
  LIBS="-L/usr/local/lib $LIBS"
  AC_CHECK_LIB(getloadavg, getloadavg,
               [LIBS="-lgetloadavg $LIBS"], [LIBS=$ac_getloadavg_LIBS])
fi

# Make sure it is really in the library, if we think we found it,
# otherwise set up the replacement function.
AC_CHECK_FUNCS(getloadavg, [],
               [_AC_LIBOBJ_GETLOADAVG])

# Some definitions of getloadavg require that the program be installed setgid.
dnl FIXME: Don't hardwire the path of getloadavg.c in the top-level directory.
AC_CACHE_CHECK(whether getloadavg requires setgid,
               ac_cv_func_getloadavg_setgid,
[AC_EGREP_CPP([Yowza Am I SETGID yet],
[#include "$srcdir/getloadavg.c"
#ifdef LDAV_PRIVILEGED
Yowza Am I SETGID yet
@%:@endif],
              ac_cv_func_getloadavg_setgid=yes,
              ac_cv_func_getloadavg_setgid=no)])
if test $ac_cv_func_getloadavg_setgid = yes; then
  NEED_SETGID=true
  AC_DEFINE(GETLOADAVG_PRIVILEGED, 1,
            [Define if the `getloadavg' function needs to be run setuid
             or setgid.])
else
  NEED_SETGID=false
fi
AC_SUBST(NEED_SETGID)dnl

if test $ac_cv_func_getloadavg_setgid = yes; then
  AC_CACHE_CHECK(group of /dev/kmem, ac_cv_group_kmem,
[ # On Solaris, /dev/kmem is a symlink.  Get info on the real file.
  ac_ls_output=`ls -lgL /dev/kmem 2>/dev/null`
  # If we got an error (system does not support symlinks), try without -L.
  test -z "$ac_ls_output" && ac_ls_output=`ls -lg /dev/kmem`
  ac_cv_group_kmem=`echo $ac_ls_output \
    | sed -ne ['s/[ 	][ 	]*/ /g;
	       s/^.[sSrwx-]* *[0-9]* *\([^0-9]*\)  *.*/\1/;
	       / /s/.* //;p;']`
])
  AC_SUBST(KMEM_GROUP, $ac_cv_group_kmem)dnl
fi
if test "x$ac_save_LIBS" = x; then
  GETLOADAVG_LIBS=$LIBS
else
  GETLOADAVG_LIBS=`echo "$LIBS" | sed "s!$ac_save_LIBS!!"`
fi
LIBS=$ac_save_LIBS

AC_SUBST(GETLOADAVG_LIBS)dnl
])# AC_FUNC_GETLOADAVG


# AU::AC_GETLOADAVG
# -----------------
AU_ALIAS([AC_GETLOADAVG], [AC_FUNC_GETLOADAVG])


# AC_FUNC_GETMNTENT
# -----------------
AC_DEFUN([AC_FUNC_GETMNTENT],
[# getmntent is in -lsun on Irix 4, -lseq on Dynix/PTX, -lgen on Unixware.
AC_CHECK_LIB(sun, getmntent, LIBS="-lsun $LIBS",
  [AC_CHECK_LIB(seq, getmntent, LIBS="-lseq $LIBS",
    [AC_CHECK_LIB(gen, getmntent, LIBS="-lgen $LIBS")])])
AC_CHECK_FUNC(getmntent,
              [AC_DEFINE(HAVE_GETMNTENT, 1,
                         [Define if you have the `getmntent' function.])])])


# _AC_FUNC_GETPGRP_TEST
# ---------------------
# A program that exits with success iff `getpgrp' seems to ignore its
# argument.
m4_define([_AC_FUNC_GETPGRP_TEST],
[AC_LANG_SOURCE([AC_INCLUDES_DEFAULT]
[[
/*
 * If this system has a BSD-style getpgrp(),
 * which takes a pid argument, exit unsuccessfully.
 *
 * Snarfed from Chet Ramey's bash pgrp.c test program
 */

int     pid;
int     pg1, pg2, pg3, pg4;
int     ng, np, s, child;

int
main ()
{
  pid = getpid ();
  pg1 = getpgrp (0);
  pg2 = getpgrp ();
  pg3 = getpgrp (pid);
  pg4 = getpgrp (1);

  /* If all of these values are the same, it's pretty sure that we're
     on a system that ignores getpgrp's first argument.  */
  if (pg2 == pg4 && pg1 == pg3 && pg2 == pg3)
    exit (0);

  child = fork ();
  if (child < 0)
    exit(1);
  else if (child == 0)
    {
      np = getpid ();
      /*  If this is Sys V, this will not work; pgrp will be set to np
	 because setpgrp just changes a pgrp to be the same as the
	 pid.  */
      setpgrp (np, pg1);
      ng = getpgrp (0);        /* Same result for Sys V and BSD */
      if (ng == pg1)
  	exit (1);
      else
  	exit (0);
    }
  else
    {
      wait (&s);
      exit (s>>8);
    }
}]])
])# _AC_FUNC_GETPGRP_TEST


# AC_FUNC_GETPGRP
# ---------------
# Figure out whether getpgrp takes an argument or not.  Try first using
# prototypes (AC_COMPILE), and if the compiler is of no help, try a runtime
# test.
AC_DEFUN([AC_FUNC_GETPGRP],
[AC_CACHE_CHECK(whether getpgrp takes no argument, ac_cv_func_getpgrp_void,
[# Use it with a single arg.
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT], [getpgrp (0);])],
                  [ac_func_getpgrp_1=yes],
                  [ac_func_getpgrp_1=no])
# Use it with no arg.
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT], [getpgrp ();])],
                  [ac_func_getpgrp_0=yes],
                  [ac_func_getpgrp_0=no])
# If both static checks agree, we are done.
case $ac_func_getpgrp_0:$ac_func_getpgrp_1 in
  yes:no) ac_cv_func_getpgrp_void=yes;;
  no:yes) ac_cv_func_getpgrp_void=false;;
  *) AC_RUN_IFELSE([_AC_FUNC_GETPGRP_TEST],
                   [ac_cv_func_getpgrp_void=yes],
                   [ac_cv_func_getpgrp_void=no],
                   [AC_MSG_ERROR([cannot check getpgrp if cross compiling])]);;
esac # $ac_func_getpgrp_0:$ac_func_getpgrp_1
])
if test $ac_cv_func_getpgrp_void = yes; then
  AC_DEFINE(GETPGRP_VOID, 1,
            [Define if the `getpgrp' function takes no argument.])
fi
])# AC_FUNC_GETPGRP


# AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
# -------------------------------------
# When crosscompiling, be pessimistic so we will end up using the
# replacement version of lstat that checkes for trailing slashes and
# calls lstat a second time when necessary.
AC_DEFUN([AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK],
[AC_CACHE_CHECK(
       [whether lstat dereferences a symlink specified with a trailing slash],
       [ac_cv_func_lstat_dereferences_slashed_symlink],
[rm -f conftest.sym conftest.file
echo >conftest.file
if ln -s conftest.file conftest.sym; then
  AC_RUN_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT],
    [struct stat sbuf;
     /* Linux will dereference the symlink and fail.
        That is better in the sense that it means we will not
        have to compile and use the lstat wrapper.  */
     exit (lstat ("conftest.sym/", &sbuf) ? 0 : 1);])],
                [ac_cv_func_lstat_dereferences_slashed_symlink=yes],
                [ac_cv_func_lstat_dereferences_slashed_symlink=no],
                [ac_cv_func_lstat_dereferences_slashed_symlink=no])
else
  # If the `ln -s' command failed, then we probably don't even
  # have an lstat function.
  ac_cv_func_lstat_dereferences_slashed_symlink=no
fi
rm -f conftest.sym conftest.file
])

test $ac_cv_func_lstat_dereferences_slashed_symlink = yes &&
  AC_DEFINE_UNQUOTED(LSTAT_FOLLOWS_SLASHED_SYMLINK, 1,
                     [Define if `lstat' dereferences a symlink specified
                      with a trailing slash.])

if test $ac_cv_func_lstat_dereferences_slashed_symlink = no; then
  AC_LIBOBJ(lstat)
fi
])


# AC_FUNC_MALLOC
# --------------
# Is `malloc (0)' properly handled?
AC_DEFUN([AC_FUNC_MALLOC],
[AC_REQUIRE([AC_HEADER_STDC])dnl
AC_CHECK_HEADERS(stdlib.h)
AC_CACHE_CHECK([for working malloc], ac_cv_func_malloc_works,
[AC_RUN_IFELSE(
[AC_LANG_PROGRAM(
[[#if STDC_HEADERS || HAVE_STDLIB_H
# include <stdlib.h>
#else
char *malloc ();
#endif
]],
                 [exit (malloc (0) ? 0 : 1);])],
               [ac_cv_func_malloc_works=yes],
               [ac_cv_func_malloc_works=no],
               [ac_cv_func_malloc_works=no])])
if test $ac_cv_func_malloc_works = yes; then
  AC_DEFINE(HAVE_MALLOC, 1,
            [Define if your system has a working `malloc' function.])
fi
])# AC_FUNC_MALLOC


# AC_FUNC_MEMCMP
# --------------
AC_DEFUN([AC_FUNC_MEMCMP],
[AC_CACHE_CHECK([for working memcmp], ac_cv_func_memcmp_working,
[AC_RUN_IFELSE([AC_LANG_PROGRAM([], [[
  /* Some versions of memcmp are not 8-bit clean.  */
  char c0 = 0x40, c1 = 0x80, c2 = 0x81;
  if (memcmp(&c0, &c2, 1) >= 0 || memcmp(&c1, &c2, 1) >= 0)
    exit (1);

  /* The Next x86 OpenStep bug shows up only when comparing 16 bytes
     or more and with at least one buffer not starting on a 4-byte boundary.
     William Lewis provided this test program.   */
  {
    char foo[21];
    char bar[21];
    int i;
    for (i = 0; i < 4; i++)
      {
        char *a = foo + i;
        char *b = bar + i;
        strcpy (a, "--------01111111");
        strcpy (b, "--------10000000");
        if (memcmp (a, b, 16) >= 0)
          exit (1);
      }
    exit (0);
  }
]])],
               [ac_cv_func_memcmp_working=yes],
               [ac_cv_func_memcmp_working=no],
               [ac_cv_func_memcmp_working=no])])
test $ac_cv_func_memcmp_working = no && AC_LIBOBJ([memcmp])
])# AC_FUNC_MEMCMP


# AC_FUNC_MKTIME
# --------------
AC_DEFUN([AC_FUNC_MKTIME],
[AC_REQUIRE([AC_HEADER_TIME])dnl
AC_CHECK_HEADERS(sys/time.h unistd.h)
AC_CHECK_FUNCS(alarm)
AC_CACHE_CHECK([for working mktime], ac_cv_func_working_mktime,
[AC_RUN_IFELSE([AC_LANG_SOURCE(
[[/* Test program from Paul Eggert and Tony Leneis.  */
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#if !HAVE_ALARM
# define alarm(X) /* empty */
#endif

/* Work around redefinition to rpl_putenv by other config tests.  */
#undef putenv

static time_t time_t_max;

/* Values we'll use to set the TZ environment variable.  */
static const char *const tz_strings[] = {
  (const char *) 0, "TZ=GMT0", "TZ=JST-9",
  "TZ=EST+3EDT+2,M10.1.0/00:00:00,M2.3.0/00:00:00"
};
#define N_STRINGS (sizeof (tz_strings) / sizeof (tz_strings[0]))

/* Fail if mktime fails to convert a date in the spring-forward gap.
   Based on a problem report from Andreas Jaeger.  */
static void
spring_forward_gap ()
{
  /* glibc (up to about 1998-10-07) failed this test. */
  struct tm tm;

  /* Use the portable POSIX.1 specification "TZ=PST8PDT,M4.1.0,M10.5.0"
     instead of "TZ=America/Vancouver" in order to detect the bug even
     on systems that don't support the Olson extension, or don't have the
     full zoneinfo tables installed.  */
  putenv ("TZ=PST8PDT,M4.1.0,M10.5.0");

  tm.tm_year = 98;
  tm.tm_mon = 3;
  tm.tm_mday = 5;
  tm.tm_hour = 2;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  tm.tm_isdst = -1;
  if (mktime (&tm) == (time_t)-1)
    exit (1);
}

static void
mktime_test (now)
     time_t now;
{
  struct tm *lt;
  if ((lt = localtime (&now)) && mktime (lt) != now)
    exit (1);
  now = time_t_max - now;
  if ((lt = localtime (&now)) && mktime (lt) != now)
    exit (1);
}

static void
irix_6_4_bug ()
{
  /* Based on code from Ariel Faigon.  */
  struct tm tm;
  tm.tm_year = 96;
  tm.tm_mon = 3;
  tm.tm_mday = 0;
  tm.tm_hour = 0;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  tm.tm_isdst = -1;
  mktime (&tm);
  if (tm.tm_mon != 2 || tm.tm_mday != 31)
    exit (1);
}

static void
bigtime_test (j)
     int j;
{
  struct tm tm;
  time_t now;
  tm.tm_year = tm.tm_mon = tm.tm_mday = tm.tm_hour = tm.tm_min = tm.tm_sec = j;
  now = mktime (&tm);
  if (now != (time_t) -1)
    {
      struct tm *lt = localtime (&now);
      if (! (lt
	     && lt->tm_year == tm.tm_year
	     && lt->tm_mon == tm.tm_mon
	     && lt->tm_mday == tm.tm_mday
	     && lt->tm_hour == tm.tm_hour
	     && lt->tm_min == tm.tm_min
	     && lt->tm_sec == tm.tm_sec
	     && lt->tm_yday == tm.tm_yday
	     && lt->tm_wday == tm.tm_wday
	     && ((lt->tm_isdst < 0 ? -1 : 0 < lt->tm_isdst)
		  == (tm.tm_isdst < 0 ? -1 : 0 < tm.tm_isdst))))
	exit (1);
    }
}

int
main ()
{
  time_t t, delta;
  int i, j;

  /* This test makes some buggy mktime implementations loop.
     Give up after 60 seconds; a mktime slower than that
     isn't worth using anyway.  */
  alarm (60);

  for (time_t_max = 1; 0 < time_t_max; time_t_max *= 2)
    continue;
  time_t_max--;
  delta = time_t_max / 997; /* a suitable prime number */
  for (i = 0; i < N_STRINGS; i++)
    {
      if (tz_strings[i])
	putenv (tz_strings[i]);

      for (t = 0; t <= time_t_max - delta; t += delta)
	mktime_test (t);
      mktime_test ((time_t) 60 * 60);
      mktime_test ((time_t) 60 * 60 * 24);

      for (j = 1; 0 < j; j *= 2)
        bigtime_test (j);
      bigtime_test (j - 1);
    }
  irix_6_4_bug ();
  spring_forward_gap ();
  exit (0);
}]])],
               [ac_cv_func_working_mktime=yes],
               [ac_cv_func_working_mktime=no],
               [ac_cv_func_working_mktime=no])])
if test $ac_cv_func_working_mktime = no; then
  AC_LIBOBJ([mktime])
fi
])# AC_FUNC_MKTIME


# AU::AM_FUNC_MKTIME
# ------------------
AU_ALIAS([AM_FUNC_MKTIME], [AC_FUNC_MKTIME])


# AC_FUNC_MMAP
# ------------
AC_DEFUN([AC_FUNC_MMAP],
[AC_CHECK_HEADERS(stdlib.h unistd.h)
AC_CHECK_FUNCS(getpagesize)
AC_CACHE_CHECK(for working mmap, ac_cv_func_mmap_fixed_mapped,
[AC_RUN_IFELSE([AC_LANG_SOURCE([AC_INCLUDES_DEFAULT]
[[/* Thanks to Mike Haertel and Jim Avera for this test.
   Here is a matrix of mmap possibilities:
	mmap private not fixed
	mmap private fixed at somewhere currently unmapped
	mmap private fixed at somewhere already mapped
	mmap shared not fixed
	mmap shared fixed at somewhere currently unmapped
	mmap shared fixed at somewhere already mapped
   For private mappings, we should verify that changes cannot be read()
   back from the file, nor mmap's back from the file at a different
   address.  (There have been systems where private was not correctly
   implemented like the infamous i386 svr4.0, and systems where the
   VM page cache was not coherent with the file system buffer cache
   like early versions of FreeBSD and possibly contemporary NetBSD.)
   For shared mappings, we should conversely verify that changes get
   propogated back to all the places they're supposed to be.

   Grep wants private fixed already mapped.
   The main things grep needs to know about mmap are:
   * does it exist and is it safe to write into the mmap'd area
   * how to use it (BSD variants)  */

#include <fcntl.h>
#include <sys/mman.h>

#if !STDC_HEADERS && !HAVE_STDLIB_H
char *malloc ();
#endif

/* This mess was copied from the GNU getpagesize.h.  */
#if !HAVE_GETPAGESIZE
/* Assume that all systems that can run configure have sys/param.h.  */
# if !HAVE_SYS_PARAM_H
#  define HAVE_SYS_PARAM_H 1
# endif

# ifdef _SC_PAGESIZE
#  define getpagesize() sysconf(_SC_PAGESIZE)
# else /* no _SC_PAGESIZE */
#  if HAVE_SYS_PARAM_H
#   include <sys/param.h>
#   ifdef EXEC_PAGESIZE
#    define getpagesize() EXEC_PAGESIZE
#   else /* no EXEC_PAGESIZE */
#    ifdef NBPG
#     define getpagesize() NBPG * CLSIZE
#     ifndef CLSIZE
#      define CLSIZE 1
#     endif /* no CLSIZE */
#    else /* no NBPG */
#     ifdef NBPC
#      define getpagesize() NBPC
#     else /* no NBPC */
#      ifdef PAGESIZE
#       define getpagesize() PAGESIZE
#      endif /* PAGESIZE */
#     endif /* no NBPC */
#    endif /* no NBPG */
#   endif /* no EXEC_PAGESIZE */
#  else /* no HAVE_SYS_PARAM_H */
#   define getpagesize() 8192	/* punt totally */
#  endif /* no HAVE_SYS_PARAM_H */
# endif /* no _SC_PAGESIZE */

#endif /* no HAVE_GETPAGESIZE */

int
main ()
{
  char *data, *data2, *data3;
  int i, pagesize;
  int fd;

  pagesize = getpagesize ();

  /* First, make a file with some known garbage in it. */
  data = (char *) malloc (pagesize);
  if (!data)
    exit (1);
  for (i = 0; i < pagesize; ++i)
    *(data + i) = rand ();
  umask (0);
  fd = creat ("conftest.mmap", 0600);
  if (fd < 0)
    exit (1);
  if (write (fd, data, pagesize) != pagesize)
    exit (1);
  close (fd);

  /* Next, try to mmap the file at a fixed address which already has
     something else allocated at it.  If we can, also make sure that
     we see the same garbage.  */
  fd = open ("conftest.mmap", O_RDWR);
  if (fd < 0)
    exit (1);
  data2 = (char *) malloc (2 * pagesize);
  if (!data2)
    exit (1);
  data2 += (pagesize - ((int) data2 & (pagesize - 1))) & (pagesize - 1);
  if (data2 != mmap (data2, pagesize, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_FIXED, fd, 0L))
    exit (1);
  for (i = 0; i < pagesize; ++i)
    if (*(data + i) != *(data2 + i))
      exit (1);

  /* Finally, make sure that changes to the mapped area do not
     percolate back to the file as seen by read().  (This is a bug on
     some variants of i386 svr4.0.)  */
  for (i = 0; i < pagesize; ++i)
    *(data2 + i) = *(data2 + i) + 1;
  data3 = (char *) malloc (pagesize);
  if (!data3)
    exit (1);
  if (read (fd, data3, pagesize) != pagesize)
    exit (1);
  for (i = 0; i < pagesize; ++i)
    if (*(data + i) != *(data3 + i))
      exit (1);
  close (fd);
  exit (0);
}]])],
               [ac_cv_func_mmap_fixed_mapped=yes],
               [ac_cv_func_mmap_fixed_mapped=no],
               [ac_cv_func_mmap_fixed_mapped=no])])
if test $ac_cv_func_mmap_fixed_mapped = yes; then
  AC_DEFINE(HAVE_MMAP, 1,
            [Define if you have a working `mmap' system call.])
fi
rm -f conftest.mmap
])# AC_FUNC_MMAP


# AU::AC_MMAP
# -----------
AU_ALIAS([AC_MMAP], [AC_FUNC_MMAP])


# AC_FUNC_OBSTACK
# ---------------
# Ensure obstack support.  Yeah, this is not exactly a `FUNC' check.
AC_DEFUN([AC_FUNC_OBSTACK],
[AC_LIBSOURCES([obstack.h, obstack.c])dnl
AC_CACHE_CHECK([for obstacks], ac_cv_func_obstack,
[AC_TRY_LINK([@%:@include "obstack.h"],
             [struct obstack *mem; obstack_free(mem,(char *) 0)],
             [ac_cv_func_obstack=yes],
             [ac_cv_func_obstack=no])])
if test $ac_cv_func_obstack = yes; then
  AC_DEFINE(HAVE_OBSTACK, 1, [Define if libc includes obstacks.])
else
  AC_LIBOBJ(obstack)
fi
])# AC_FUNC_OBSTACK


# AU::AM_FUNC_OBSTACK
# -------------------
AU_ALIAS([AM_FUNC_OBSTACK], [AC_FUNC_OBSTACK])


# AC_FUNC_SELECT_ARGTYPES
# -----------------------
# Determine the correct type to be passed to each of the `select'
# function's arguments, and define those types in `SELECT_TYPE_ARG1',
# `SELECT_TYPE_ARG234', and `SELECT_TYPE_ARG5'.
AC_DEFUN([AC_FUNC_SELECT_ARGTYPES],
[AC_CHECK_HEADERS(sys/select.h sys/socket.h)
AC_CACHE_CHECK([types of arguments for select],
[ac_cv_func_select_args],
[for ac_arg234 in 'fd_set *' 'int *' 'void *'; do
 for ac_arg1 in 'int' 'size_t' 'unsigned long' 'unsigned'; do
  for ac_arg5 in 'struct timeval *' 'const struct timeval *'; do
   AC_COMPILE_IFELSE(
       [AC_LANG_PROGRAM(
[AC_INCLUDES_DEFAULT
#if HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
],
                        [extern int select ($ac_arg1,
                                            $ac_arg234, $ac_arg234, $ac_arg234,
                                            $ac_arg5);])],
              [ac_cv_func_select_args="$ac_arg1,$ac_arg234,$ac_arg5"; break 3])
  done
 done
done
# Provide a safe default value.
: ${ac_cv_func_select_args='int,int *,struct timeval *'}
])
ac_save_IFS=$IFS; IFS=','
set dummy `echo "$ac_cv_func_select_args" | sed 's/\*/\*/g'`
IFS=$ac_save_IFS
shift
AC_DEFINE_UNQUOTED(SELECT_TYPE_ARG1, $[1],
                   [Define to the type of arg 1 for `select'.])
AC_DEFINE_UNQUOTED(SELECT_TYPE_ARG234, ($[2]),
                   [Define to the type of args 2, 3 and 4 for `select'.])
AC_DEFINE_UNQUOTED(SELECT_TYPE_ARG5, ($[3]),
                   [Define to the type of arg 5 for `select'.])
rm -f conftest*
])# AC_FUNC_SELECT_ARGTYPES


# AC_FUNC_SETPGRP
# ---------------
AC_DEFUN([AC_FUNC_SETPGRP],
[AC_CACHE_CHECK(whether setpgrp takes no argument, ac_cv_func_setpgrp_void,
[AC_RUN_IFELSE(
[AC_LANG_PROGRAM(
[#if HAVE_UNISTD_H
# include <unistd.h>
#endif
],
[/* If this system has a BSD-style setpgrp, which takes arguments, exit
   successfully.  */
  exit (setpgrp (1,1) == -1);])],
               [ac_cv_func_setpgrp_void=no],
               [ac_cv_func_setpgrp_void=yes],
               [AC_MSG_ERROR([cannot check setpgrp if cross compiling])])])
if test $ac_cv_func_setpgrp_void = yes; then
  AC_DEFINE(SETPGRP_VOID, 1,
            [Define if the `setpgrp' function takes no argument.])
fi
])# AC_FUNC_SETPGRP


# _AC_FUNC_STAT(STAT | LSTAT)
# ---------------------------
# Determine whether stat or lstat have the bug that it succeeds when
# given the zero-length file name argument.  The stat and lstat from
# SunOS4.1.4 and the Hurd (as of 1998-11-01) do this.
#
# If it does, then define HAVE_STAT_EMPTY_STRING_BUG (or
# HAVE_LSTAT_EMPTY_STRING_BUG) and arrange to compile the wrapper
# function.
m4_define([_AC_FUNC_STAT],
[AC_REQUIRE([AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK])dnl
AC_CACHE_CHECK([whether $1 accepts an empty string],
               [ac_cv_func_$1_empty_string_bug],
[AC_RUN_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT],
[[struct stat sbuf;
  exit ($1 ("", &sbuf) ? 1 : 0);]])],
            [ac_cv_func_$1_empty_string_bug=yes],
            [ac_cv_func_$1_empty_string_bug=no],
            [ac_cv_func_$1_empty_string_bug=yes])])
if test $ac_cv_func_$1_empty_string_bug = yes; then
  AC_LIBOBJ([$1])
  AC_DEFINE_UNQUOTED(AS_TR_CPP([HAVE_$1_EMPTY_STRING_BUG]), 1,
                     [Define if `$1' has the bug that it succeeds when
                      given the zero-length file name argument.])
fi
])# _AC_FUNC_STAT


# AC_FUNC_STAT & AC_FUNC_LSTAT
# ----------------------------
AC_DEFUN([AC_FUNC_STAT],  [_AC_FUNC_STAT(stat)])
AC_DEFUN([AC_FUNC_LSTAT], [_AC_FUNC_STAT(lstat)])


# _AC_LIBOBJ_STRTOD
# -----------------
m4_define([_AC_LIBOBJ_STRTOD],
[AC_LIBOBJ(strtod)
AC_CHECK_FUNC(pow)
if test $ac_cv_func_pow = no; then
  AC_CHECK_LIB(m, pow,
               [POW_LIB=-lm],
               [AC_MSG_WARN([can't find library containing definition of pow])])
fi
])# _AC_LIBOBJ_STRTOD


# AC_FUNC_STRTOD
# --------------
AC_DEFUN([AC_FUNC_STRTOD],
[AC_CACHE_CHECK(for working strtod, ac_cv_func_strtod,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
double strtod ();
int
main()
{
  {
    /* Some versions of Linux strtod mis-parse strings with leading '+'.  */
    char *string = " +69";
    char *term;
    double value;
    value = strtod (string, &term);
    if (value != 69 || term != (string + 4))
      exit (1);
  }

  {
    /* Under Solaris 2.4, strtod returns the wrong value for the
       terminating character under some conditions.  */
    char *string = "NaN";
    char *term;
    strtod (string, &term);
    if (term != string && *(term - 1) == 0)
      exit (1);
  }
  exit (0);
}
]])],
               ac_cv_func_strtod=yes,
               ac_cv_func_strtod=no,
               ac_cv_func_strtod=no)])
if test $ac_cv_func_strtod = no; then
  _AC_LIBOBJ_STRTOD
fi
])


# AU::AM_FUNC_STRTOD
# ------------------
AU_ALIAS([AM_FUNC_STRTOD], [AC_FUNC_STRTOD])


# AC_FUNC_STRERROR_R
# ------------------
AC_DEFUN([AC_FUNC_STRERROR_R],
[AC_CHECK_DECLS([strerror_r])
AC_CHECK_FUNCS([strerror_r])
if test $ac_cv_func_strerror_r = yes; then
  AC_CACHE_CHECK([for working strerror_r],
                 ac_cv_func_strerror_r_works,
   [
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT],
     [[
       char buf[100];
       char x = *strerror_r (0, buf, sizeof buf);
     ]])],
                      ac_cv_func_strerror_r_works=yes,
                      ac_cv_func_strerror_r_works=no)
    if test $ac_cv_func_strerror_r_works = no; then
      # strerror_r seems not to work, but now we have to choose between
      # systems that have relatively inaccessible declarations for the
      # function.  BeOS and DEC UNIX 4.0 fall in this category, but the
      # former has a strerror_r that returns char*, while the latter
      # has a strerror_r that returns `int'.
      # This test should segfault on the DEC system.
      AC_RUN_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT
	extern char *strerror_r ();],
	[[char buf[100];
	  char x = *strerror_r (0, buf, sizeof buf);
	  exit (!isalpha (x));]])],
                    ac_cv_func_strerror_r_works=yes,
                    ac_cv_func_strerror_r_works=no,
                    ac_cv_func_strerror_r_works=no)
    fi
  ])
  if test $ac_cv_func_strerror_r_works = yes; then
    AC_DEFINE_UNQUOTED([HAVE_WORKING_STRERROR_R], 1,
                       [Define to 1 if `strerror_r' returns a string.])
  fi
fi
])# AC_FUNC_STRERROR_R


# AC_FUNC_STRFTIME
# ----------------
AC_DEFUN([AC_FUNC_STRFTIME],
[AC_CHECK_FUNCS(strftime, [],
[# strftime is in -lintl on SCO UNIX.
AC_CHECK_LIB(intl, strftime,
             [AC_DEFINE(HAVE_STRFTIME)
LIBS="-lintl $LIBS"])])dnl
])# AC_FUNC_STRFTIME


# AC_FUNC_SETVBUF_REVERSED
# ------------------------
AC_DEFUN([AC_FUNC_SETVBUF_REVERSED],
[AC_CACHE_CHECK(whether setvbuf arguments are reversed,
  ac_cv_func_setvbuf_reversed,
[AC_TRY_RUN([#include <stdio.h>
/* If setvbuf has the reversed format, exit 0. */
int
main ()
{
  /* This call has the arguments reversed.
     A reversed system may check and see that the address of main
     is not _IOLBF, _IONBF, or _IOFBF, and return nonzero.  */
  if (setvbuf(stdout, _IOLBF, (char *) main, BUFSIZ) != 0)
    exit(1);
  putc('\r', stdout);
  exit(0);			/* Non-reversed systems segv here.  */
}], ac_cv_func_setvbuf_reversed=yes, ac_cv_func_setvbuf_reversed=no)
rm -f core core.* *.core])
if test $ac_cv_func_setvbuf_reversed = yes; then
  AC_DEFINE(SETVBUF_REVERSED, 1,
            [Define if the `setvbuf' function takes the buffering type as
             its second argument and the buffer pointer as the third, as on
             System V before release 3.])
fi
])# AC_FUNC_SETVBUF_REVERSED


# AU::AC_SETVBUF_REVERSED
# -----------------------
AU_ALIAS([AC_SETVBUF_REVERSED], [AC_FUNC_SETVBUF_REVERSED])


# AC_FUNC_STRCOLL
# ---------------
AC_DEFUN([AC_FUNC_STRCOLL],
[AC_CACHE_CHECK(for working strcoll, ac_cv_func_strcoll_works,
[AC_RUN_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT],
  [[exit (strcoll ("abc", "def") >= 0 ||
	 strcoll ("ABC", "DEF") >= 0 ||
	 strcoll ("123", "456") >= 0)]])],
               ac_cv_func_strcoll_works=yes,
               ac_cv_func_strcoll_works=no,
               ac_cv_func_strcoll_works=no)])
if test $ac_cv_func_strcoll_works = yes; then
  AC_DEFINE(HAVE_STRCOLL, 1,
            [Define if you have the `strcoll' function and it is properly
             defined.])
fi
])# AC_FUNC_STRCOLL


# AU::AC_STRCOLL
# --------------
AU_ALIAS([AC_STRCOLL], [AC_FUNC_STRCOLL])


# AC_FUNC_UTIME_NULL
# ------------------
AC_DEFUN([AC_FUNC_UTIME_NULL],
[AC_CACHE_CHECK(whether utime accepts a null argument, ac_cv_func_utime_null,
[rm -f conftest.data; >conftest.data
# Sequent interprets utime(file, 0) to mean use start of epoch.  Wrong.
AC_RUN_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT],
[[struct stat s, t;
  exit (!(stat ("conftest.data", &s) == 0
          && utime ("conftest.data", (long *)0) == 0
          && stat ("conftest.data", &t) == 0
          && t.st_mtime >= s.st_mtime
          && t.st_mtime - s.st_mtime < 120));]])],
              ac_cv_func_utime_null=yes,
              ac_cv_func_utime_null=no,
              ac_cv_func_utime_null=no)
rm -f core core.* *.core])
if test $ac_cv_func_utime_null = yes; then
  AC_DEFINE(HAVE_UTIME_NULL, 1,
            [Define if `utime(file, NULL)' sets file's timestamp to the
             present.])
fi
rm -f conftest.data
])# AC_FUNC_UTIME_NULL


# AU::AC_UTIME_NULL
# -----------------
AU_ALIAS([AC_UTIME_NULL], [AC_FUNC_UTIME_NULL])


# AC_FUNC_FORK
# -------------
AC_DEFUN([AC_FUNC_FORK],
[AC_REQUIRE([AC_TYPE_PID_T])dnl
AC_CHECK_HEADERS(unistd.h vfork.h)
AC_CHECK_FUNCS(fork vfork)
ac_cv_func_fork_works=$ac_cv_func_fork
if test "x$ac_cv_func_fork" = xyes; then
  _AC_FUNC_FORK
fi
if test "x$ac_cv_func_fork_works" = xcross; then
  case $host in
    *-*-amigaos* | *-*-msdosdjgpp*)
      # Override, as these systems have only a dummy fork() stub
      ac_cv_func_fork_works=no
      ;;
    *)
      ac_cv_func_fork_works=yes
      ;;
  esac
  AC_MSG_WARN(CROSS: Result $ac_cv_func_fork_works guessed due to cross-compiling.)
fi
ac_cv_func_vfork_works=$ac_cv_func_vfork
if test "x$ac_cv_func_vfork" = xyes; then
  _AC_FUNC_VFORK
fi;
if test "x$ac_cv_func_fork_works" = xcross; then
  ac_cv_func_vfork_works=ac_cv_func_vfork
  AC_MSG_WARN(CROSS: Result $ac_cv_func_vfork_works guessed due to cross-compiling.)
fi

if test "x$ac_cv_func_vfork_works" = xyes; then
  AC_DEFINE(HAVE_WORKING_VFORK, 1, [Define if `vfork' works.])
else
  AC_DEFINE(vfork, fork, [Define as `fork' if `vfork' does not work.])
fi
if test "x$ac_cv_func_fork_works" = xyes; then
  AC_DEFINE(HAVE_WORKING_FORK, 1, [Define if `fork' works.])
fi
])# AC_FUNC_FORK


# _AC_FUNC_FORK
# -------------
AC_DEFUN([_AC_FUNC_FORK],
  [AC_CACHE_CHECK(for working fork, ac_cv_func_fork_works,
    [AC_RUN_IFELSE([/* By Rüdiger Kuhlmann. */
      #include <sys/types.h>
      #if HAVE_UNISTD_H
      # include <unistd.h>
      #endif
      /* Some systems only have a dummy stub for fork() */
      int main ()
      {
        if (fork() < 0)
          exit (1);
        exit (0);
      }],
    [ac_cv_func_fork_works=yes],
    [ac_cv_func_fork_works=no],
    [ac_cv_func_fork_works=cross])])]
)# _AC_FUNC_FORK


# _AC_FUNC_VFORK
# -------------
AC_DEFUN([_AC_FUNC_VFORK],
[AC_CACHE_CHECK(for working vfork, ac_cv_func_vfork_works,
[AC_TRY_RUN([/* Thanks to Paul Eggert for this test.  */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#if HAVE_VFORK_H
# include <vfork.h>
#endif
/* On some sparc systems, changes by the child to local and incoming
   argument registers are propagated back to the parent.  The compiler
   is told about this with #include <vfork.h>, but some compilers
   (e.g. gcc -O) don't grok <vfork.h>.  Test for this by using a
   static variable whose address is put into a register that is
   clobbered by the vfork.  */
static
#ifdef __cplusplus
sparc_address_test (int arg)
# else
sparc_address_test (arg) int arg;
#endif
{
  static pid_t child;
  if (!child) {
    child = vfork ();
    if (child < 0) {
      perror ("vfork");
      _exit(2);
    }
    if (!child) {
      arg = getpid();
      write(-1, "", 0);
      _exit (arg);
    }
  }
}

int
main ()
{
  pid_t parent = getpid ();
  pid_t child;

  sparc_address_test ();

  child = vfork ();

  if (child == 0) {
    /* Here is another test for sparc vfork register problems.  This
       test uses lots of local variables, at least as many local
       variables as main has allocated so far including compiler
       temporaries.  4 locals are enough for gcc 1.40.3 on a Solaris
       4.1.3 sparc, but we use 8 to be safe.  A buggy compiler should
       reuse the register of parent for one of the local variables,
       since it will think that parent can't possibly be used any more
       in this routine.  Assigning to the local variable will thus
       munge parent in the parent process.  */
    pid_t
      p = getpid(), p1 = getpid(), p2 = getpid(), p3 = getpid(),
      p4 = getpid(), p5 = getpid(), p6 = getpid(), p7 = getpid();
    /* Convince the compiler that p..p7 are live; otherwise, it might
       use the same hardware register for all 8 local variables.  */
    if (p != p1 || p != p2 || p != p3 || p != p4
	|| p != p5 || p != p6 || p != p7)
      _exit(1);

    /* On some systems (e.g. IRIX 3.3), vfork doesn't separate parent
       from child file descriptors.  If the child closes a descriptor
       before it execs or exits, this munges the parent's descriptor
       as well.  Test for this by closing stdout in the child.  */
    _exit(close(fileno(stdout)) != 0);
  } else {
    int status;
    struct stat st;

    while (wait(&status) != child)
      ;
    exit(
	 /* Was there some problem with vforking?  */
	 child < 0

	 /* Did the child fail?  (This shouldn't happen.)  */
	 || status

	 /* Did the vfork/compiler bug occur?  */
	 || parent != getpid()

	 /* Did the file descriptor bug occur?  */
	 || fstat(fileno(stdout), &st) != 0
	 );
  }
}],
            [ac_cv_func_vfork_works=yes],
            [ac_cv_func_vfork_works=no],
            [ac_cv_func_vfork_works=cross])])
])# _AC_FUNC_VFORK


# AU::AC_FUNC_VFORK
# ------------
AU_ALIAS([AC_FUNC_VFORK], [AC_FUNC_FORK])

# AU::AC_VFORK
# ------------
AU_ALIAS([AC_VFORK], [AC_FUNC_FORK])


# AC_FUNC_VPRINTF
# ---------------
# Why the heck is that _doprnt does not define HAVE__DOPRNT???
# That the logical name!
AC_DEFUN([AC_FUNC_VPRINTF],
[AC_CHECK_FUNCS(vprintf, []
[AC_CHECK_FUNC(_doprnt,
               [AC_DEFINE(HAVE_DOPRNT, 1,
                          [Define if you don't have `vprintf' but do have
                          `_doprnt.'])])])
])


# AU::AC_VPRINTF
# --------------
AU_ALIAS([AC_VPRINTF], [AC_FUNC_VPRINTF])


# AC_FUNC_WAIT3
# -------------
# Don't bother too hard maintaining this macro, as it's obsoleted.
# We don't AU define it, since we don't have any alternative to propose,
# any invocation should be removed, and the code adjusted.
AC_DEFUN([AC_FUNC_WAIT3],
[AC_DIAGNOSE([obsolete],
[$0: `wait3' is being removed from the Open Group standards.
Remove this `AC_FUNC_WAIT3' and adjust your code to use `waitpid' instead.])dnl
AC_CACHE_CHECK([for wait3 that fills in rusage],
               [ac_cv_func_wait3_rusage],
[AC_RUN_IFELSE([AC_LANG_SOURCE(
[[#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
/* HP-UX has wait3 but does not fill in rusage at all.  */
int
main ()
{
  struct rusage r;
  int i;
  /* Use a field that we can force nonzero --
     voluntary context switches.
     For systems like NeXT and OSF/1 that don't set it,
     also use the system CPU time.  And page faults (I/O) for Linux.  */
  r.ru_nvcsw = 0;
  r.ru_stime.tv_sec = 0;
  r.ru_stime.tv_usec = 0;
  r.ru_majflt = r.ru_minflt = 0;
  switch (fork ())
    {
    case 0: /* Child.  */
      sleep(1); /* Give up the CPU.  */
      _exit(0);
      break;
    case -1: /* What can we do?  */
      _exit(0);
      break;
    default: /* Parent.  */
      wait3(&i, 0, &r);
      /* Avoid "text file busy" from rm on fast HP-UX machines.  */
      sleep(2);
      exit (r.ru_nvcsw == 0 && r.ru_majflt == 0 && r.ru_minflt == 0
	    && r.ru_stime.tv_sec == 0 && r.ru_stime.tv_usec == 0);
    }
}]])],
               [ac_cv_func_wait3_rusage=yes],
               [ac_cv_func_wait3_rusage=no],
               [ac_cv_func_wait3_rusage=no])])
if test $ac_cv_func_wait3_rusage = yes; then
  AC_DEFINE(HAVE_WAIT3, 1,
            [Define if you have the `wait3' system call.
             Deprecated, you should no longer depend upon `wait3'.])
fi
])# AC_FUNC_WAIT3


# AU::AC_WAIT3
# ------------
AU_ALIAS([AC_WAIT3], [AC_FUNC_WAIT3])
