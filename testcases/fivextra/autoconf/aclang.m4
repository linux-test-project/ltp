# This file is part of Autoconf.                       -*- Autoconf -*-
# Programming languages support.
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


# Table of Contents:
#
# 1. Language selection
#    and routines to produce programs in a given language.
#  a. generic routines
#  b. C
#  c. C++
#  d. Fortran 77
#
# 2. Producing programs in a given language.
#  a. generic routines
#  b. C
#  c. C++
#  d. Fortran 77
#
# 3. Looking for a compiler
#    And possibly the associated preprocessor.
#  a. Generic routines.
#  b. C
#  c. C++
#  d. Fortran 77
#
# 4. Compilers' characteristics.
#  a. Generic routines.
#  b. C
#  c. C++
#  d. Fortran 77



## ----------------------- ##
## 1. Language selection.  ##
## ----------------------- ##



# -------------------------------- #
# 1a. Generic language selection.  #
# -------------------------------- #

# AC_LANG_CASE(LANG1, IF-LANG1, LANG2, IF-LANG2, ..., DEFAULT)
# ------------------------------------------------------------
# Expand into IF-LANG1 if the current language is LANG1 etc. else
# into default.
m4_define([AC_LANG_CASE],
[m4_case(_AC_LANG, $@)])


# _AC_LANG_DISPATCH(MACRO, LANG, ARGS)
# ------------------------------------
# Call the specialization of MACRO for LANG with ARGS.  Complain if
# unavailable.
m4_define([_AC_LANG_DISPATCH],
[m4_ifdef([$1($2)],
       [m4_indir([$1($2)], m4_shiftn(2, $@))],
       [AC_FATAL([$1: unknown language: $2])])])


# _AC_LANG_SET(OLD, NEW)
# ----------------------
# Output the shell code needed to switch from OLD language to NEW language.
# Do not try to optimize like this:
#
# m4_defun([_AC_LANG_SET],
# [m4_if([$1], [$2], [],
#        [_AC_LANG_DISPATCH([AC_LANG], [$2])])])
#
# as it can introduce differences between the sh-current language and the
# m4-current-language when m4_require is used.  Something more subtle
# might be possible, but at least for the time being, play it safe.
m4_defun([_AC_LANG_SET],
[_AC_LANG_DISPATCH([AC_LANG], [$2])])


# AC_LANG(LANG)
# -------------
# Set the current language to LANG.
m4_defun([AC_LANG],
[_AC_LANG_SET(m4_ifdef([_AC_LANG], [m4_defn([_AC_LANG])]),
              [$1])dnl
m4_define([_AC_LANG], [$1])])


# AC_LANG_PUSH(LANG)
# ------------------
# Save the current language, and use LANG.
m4_defun([AC_LANG_PUSH],
[_AC_LANG_SET(m4_ifdef([_AC_LANG], [m4_defn([_AC_LANG])]),
              [$1])dnl
m4_pushdef([_AC_LANG], [$1])])


# AC_LANG_POP([LANG])
# -------------------
# If given, check that the current language is LANG, and restore the
# previous language.
m4_defun([AC_LANG_POP],
[m4_ifval([$1],
 [m4_if([$1], m4_defn([_AC_LANG]), [],
  [m4_fatal([$0($1): unexpected current language: ]m4_defn([_AC_LANG]))])])dnl
m4_pushdef([$0 OLD], m4_defn([_AC_LANG]))dnl
m4_popdef([_AC_LANG])dnl
_AC_LANG_SET(m4_defn([$0 OLD]), m4_defn([_AC_LANG]))dnl
m4_popdef([$0 OLD])dnl
])


# AC_LANG_SAVE
# ------------
# Save the current language, but don't change language.
AU_DEFUN([AC_LANG_SAVE],
[AC_DIAGNOSE([obsolete],
             [instead of using `AC_LANG', `AC_LANG_SAVE',
and `AC_LANG_RESTORE', you should use `AC_LANG_PUSH' and `AC_LANG_POP'.])
m4_pushdef([_AC_LANG], _AC_LANG)])


# AC_LANG_RESTORE
# ---------------
# Restore the current language from the stack.
AU_DEFUN([AC_LANG_RESTORE], [AC_LANG_POP($@)])


# _AC_LANG_ABBREV
# ---------------
# Return a short signature of _AC_LANG which can be used in shell
# variable names, or in M4 macro names.
m4_defun([_AC_LANG_ABBREV],
[_AC_LANG_DISPATCH([$0], _AC_LANG, $@)])


# AC_LANG_ASSERT(LANG)
# --------------------
# Current language must be LANG.
m4_defun([AC_LANG_ASSERT],
[m4_if(_AC_LANG, $1, [],
       [m4_fatal([$0: current language is not $1: ] _AC_LANG)])])



# -------------------- #
# 1b. The C language.  #
# -------------------- #


# AC_LANG(C)
# ----------
# CFLAGS is not in ac_cpp because -g, -O, etc. are not valid cpp options.
m4_define([AC_LANG(C)],
[ac_ext=c
ac_cpp='$CPP $CPPFLAGS'
ac_compile='$CC -c $CFLAGS $CPPFLAGS conftest.$ac_ext >&AS_MESSAGE_LOG_FD'
ac_link='$CC -o conftest$ac_exeext $CFLAGS $CPPFLAGS $LDFLAGS conftest.$ac_ext $LIBS >&AS_MESSAGE_LOG_FD'
ac_compiler_gnu=$ac_cv_c_compiler_gnu
])


# AC_LANG_C
# ---------
AU_DEFUN([AC_LANG_C], [AC_LANG(C)])


# _AC_LANG_ABBREV(C)
# ------------------
m4_define([_AC_LANG_ABBREV(C)], [c])


# ---------------------- #
# 1c. The C++ language.  #
# ---------------------- #


# AC_LANG(C++)
# ------------
# CXXFLAGS is not in ac_cpp because -g, -O, etc. are not valid cpp options.
m4_define([AC_LANG(C++)],
[ac_ext=cc
ac_cpp='$CXXCPP $CPPFLAGS'
ac_compile='$CXX -c $CXXFLAGS $CPPFLAGS conftest.$ac_ext >&AS_MESSAGE_LOG_FD'
ac_link='$CXX -o conftest$ac_exeext $CXXFLAGS $CPPFLAGS $LDFLAGS conftest.$ac_ext $LIBS >&AS_MESSAGE_LOG_FD'
ac_compiler_gnu=$ac_cv_cxx_compiler_gnu
])


# AC_LANG_CPLUSPLUS
# -----------------
AU_DEFUN([AC_LANG_CPLUSPLUS], [AC_LANG(C++)])


# _AC_LANG_ABBREV(C++)
# --------------------
m4_define([_AC_LANG_ABBREV(C++)], [cxx])


# ----------------------------- #
# 1d. The Fortran 77 language.  #
# ----------------------------- #


# AC_LANG(Fortran 77)
# -------------------
m4_define([AC_LANG(Fortran 77)],
[ac_ext=f
ac_compile='$F77 -c $FFLAGS conftest.$ac_ext >&AS_MESSAGE_LOG_FD'
ac_link='$F77 -o conftest$ac_exeext $FFLAGS $LDFLAGS conftest.$ac_ext $LIBS >&AS_MESSAGE_LOG_FD'
ac_compiler_gnu=$ac_cv_f77_compiler_gnu
])


# AC_LANG_FORTRAN77
# -----------------
AU_DEFUN([AC_LANG_FORTRAN77], [AC_LANG(Fortran 77)])


# _AC_LANG_ABBREV(Fortran 77)
# ---------------------------
m4_define([_AC_LANG_ABBREV(Fortran 77)], [f77])



## ---------------------- ##
## 2.Producing programs.  ##
## ---------------------- ##


# ---------------------- #
# 2a. Generic routines.  #
# ---------------------- #


# AC_LANG_CONFTEST(BODY)
# ----------------------
# Save the BODY in `conftest.$ac_ext'.  Add a trailing new line.
m4_define([AC_LANG_CONFTEST],
[cat >conftest.$ac_ext <<_ACEOF
$1
_ACEOF])


# AC_LANG_SOURCE(BODY)
# --------------------
# Produce a valid source for the current language, which includes the
# BODY, and as much as possible `confdefs.h' and the `#line' sync
# lines.
AC_DEFUN([AC_LANG_SOURCE],
[_AC_LANG_DISPATCH([$0], _AC_LANG, $@)])


# AC_LANG_PROGRAM([PROLOGUE], [BODY])
# -----------------------------------
# Produce a valid source for the current language.  Prepend the
# PROLOGUE (typically CPP directives and/or declarations) to an
# execution the BODY (typically glued inside the `main' function, or
# equivalent).
AC_DEFUN([AC_LANG_PROGRAM],
[AC_LANG_SOURCE([_AC_LANG_DISPATCH([$0], _AC_LANG, $@)])])


# AC_LANG_CALL(PROLOGUE, FUNCTION)
# --------------------------------
# Call the FUNCTION.
AC_DEFUN([AC_LANG_CALL],
[_AC_LANG_DISPATCH([$0], _AC_LANG, $@)])


# AC_LANG_FUNC_LINK_TRY(FUNCTION)
# -------------------------------
# Produce a source which links correctly iff the FUNCTION exists.
AC_DEFUN([AC_LANG_FUNC_LINK_TRY],
[_AC_LANG_DISPATCH([$0], _AC_LANG, $@)])


# AC_LANG_BOOL_COMPILE_TRY(PROLOGUE, EXPRESSION)
# ----------------------------------------------
# Produce a program that compiles with success iff the boolean EXPRESSION
# evaluates to true at compile time.
AC_DEFUN([AC_LANG_BOOL_COMPILE_TRY],
[_AC_LANG_DISPATCH([$0], _AC_LANG, $@)])


# AC_LANG_INT_SAVE(PROLOGUE, EXPRESSION)
# --------------------------------------
# Produce a program that saves the runtime evaluation of the integer
# EXPRESSION into `conftest.val'.
AC_DEFUN([AC_LANG_INT_SAVE],
[_AC_LANG_DISPATCH([$0], _AC_LANG, $@)])


# --------------- #
# 2b. C sources.  #
# --------------- #

# AC_LANG_SOURCE(C)(BODY)
# -----------------------
# This sometimes fails to find confdefs.h, for some reason.
# #line __oline__ "$[0]"
m4_define([AC_LANG_SOURCE(C)],
[#line __oline__ "configure"
#include "confdefs.h"
$1])


# AC_LANG_PROGRAM(C)([PROLOGUE], [BODY])
# --------------------------------------
# If AC_F77_DUMMY_MAIN was run, then any C/C++ program might be linked
# against Fortran code, hence a dummy main might be needed.
m4_define([AC_LANG_PROGRAM(C)],
[$1
m4_ifdef([_AC_LANG_PROGRAM_C_F77_HOOKS], [_AC_LANG_PROGRAM_C_F77_HOOKS()])dnl
int
main ()
{
dnl Do *not* indent the following line: there may be CPP directives.
dnl Don't move the `;' right after for the same reason.
$2
  ;
  return 0;
}])


# AC_LANG_CALL(C)(PROLOGUE, FUNCTION)
# -----------------------------------
# Avoid conflicting decl of main.
m4_define([AC_LANG_CALL(C)],
[AC_LANG_PROGRAM([$1
m4_if([$2], [main], ,
[/* Override any gcc2 internal prototype to avoid an error.  */
#ifdef __cplusplus
extern "C"
#endif
/* We use char because int might match the return type of a gcc2
   builtin and then its argument prototype would still apply.  */
char $2 ();])], [$2 ();])])


# AC_LANG_FUNC_LINK_TRY(C)(FUNCTION)
# ----------------------------------
# Don't include <ctype.h> because on OSF/1 3.0 it includes
# <sys/types.h> which includes <sys/select.h> which contains a
# prototype for select.  Similarly for bzero.
m4_define([AC_LANG_FUNC_LINK_TRY(C)],
[AC_LANG_PROGRAM(
[/* System header to define __stub macros and hopefully few prototypes,
    which can conflict with char $1 (); below.  */
#include <assert.h>
/* Override any gcc2 internal prototype to avoid an error.  */
#ifdef __cplusplus
extern "C"
#endif
/* We use char because int might match the return type of a gcc2
   builtin and then its argument prototype would still apply.  */
char $1 ();
char (*f) ();
],
[/* The GNU C library defines this for functions which it implements
    to always fail with ENOSYS.  Some functions are actually named
    something starting with __ and the normal name is an alias.  */
#if defined (__stub_$1) || defined (__stub___$1)
choke me
#else
f = $1;
#endif
])])


# AC_LANG_BOOL_COMPILE_TRY(C)(PROLOGUE, EXPRESSION)
# -------------------------------------------------
m4_define([AC_LANG_BOOL_COMPILE_TRY(C)],
[AC_LANG_PROGRAM([$1], [int _array_ @<:@1 - 2 * !($2)@:>@])])


# AC_LANG_INT_SAVE(C)(PROLOGUE, EXPRESSION)
# -----------------------------------------
# We need `stdio.h' to open a `FILE', so the prologue defaults to the
# inclusion of `stdio.h'.
m4_define([AC_LANG_INT_SAVE(C)],
[AC_LANG_PROGRAM([m4_default([$1], [@%:@include <stdio.h>])],
[FILE *f = fopen ("conftest.val", "w");
if (!f)
  exit (1);
fprintf (f, "%d", ($2));
fclose (f);])])


# ----------------- #
# 2c. C++ sources.  #
# ----------------- #

# AC_LANG_SOURCE(C++)(BODY)
# -------------------------
m4_copy([AC_LANG_SOURCE(C)], [AC_LANG_SOURCE(C++)])


# AC_LANG_PROGRAM(C++)([PROLOGUE], [BODY])
# ----------------------------------------
m4_copy([AC_LANG_PROGRAM(C)], [AC_LANG_PROGRAM(C++)])


# AC_LANG_CALL(C++)(PROLOGUE, FUNCTION)
# -------------------------------------
m4_copy([AC_LANG_CALL(C)], [AC_LANG_CALL(C++)])


# AC_LANG_FUNC_LINK_TRY(C++)(FUNCTION)
# ------------------------------------
m4_copy([AC_LANG_FUNC_LINK_TRY(C)], [AC_LANG_FUNC_LINK_TRY(C++)])


# AC_LANG_BOOL_COMPILE_TRY(C++)(PROLOGUE, EXPRESSION)
# ---------------------------------------------------
m4_copy([AC_LANG_BOOL_COMPILE_TRY(C)], [AC_LANG_BOOL_COMPILE_TRY(C++)])


# AC_LANG_INT_SAVE(C++)(PROLOGUE, EXPRESSION)
# -------------------------------------------
m4_copy([AC_LANG_INT_SAVE(C)], [AC_LANG_INT_SAVE(C++)])



# ------------------------ #
# 2d. Fortran 77 sources.  #
# ------------------------ #

# AC_LANG_SOURCE(Fortran 77)(BODY)
# --------------------------------
# FIXME: Apparently, according to former AC_TRY_COMPILER, the CPP
# directives must not be included.  But AC_TRY_RUN_NATIVE was not
# avoiding them, so?
m4_define([AC_LANG_SOURCE(Fortran 77)],
[$1])


# AC_LANG_PROGRAM(Fortran 77)([PROLOGUE], [BODY])
# -----------------------------------------------
# Yes, we discard the PROLOGUE.
m4_define([AC_LANG_PROGRAM(Fortran 77)],
[m4_ifval([$1],
       [m4_warn([syntax], [$0: ignoring PROLOGUE: $1])])dnl
      program main
$2
      end])


# AC_LANG_CALL(Fortran 77)(PROLOGUE, FUNCTION)
# --------------------------------------------
# FIXME: This is a guess, help!
m4_define([AC_LANG_CALL(Fortran 77)],
[AC_LANG_PROGRAM([$1],
[      call $2])])




## -------------------------------------------- ##
## 3. Looking for Compilers and Preprocessors.  ##
## -------------------------------------------- ##

# ----------------------------------------------------- #
# 3a. Generic routines in compilers and preprocessors.  #
# ----------------------------------------------------- #

# AC_LANG_COMPILER
# ----------------
# Find a compiler for the current LANG.  Be sure to be run before
# AC_LANG_PREPROC.
#
# Note that because we might AC_REQUIRE `AC_LANG_COMPILER(C)' for
# instance, the latter must be AC_DEFUN'd, not just define'd.
m4_define([AC_LANG_COMPILER],
[AC_BEFORE([AC_LANG_COMPILER(]_AC_LANG[)],
           [AC_LANG_PREPROC(]_AC_LANG[)])dnl
_AC_LANG_DISPATCH([$0], _AC_LANG, $@)])


# AC_LANG_COMPILER_REQUIRE
# ------------------------
# Ensure we have a compiler for the current LANG.
AC_DEFUN([AC_LANG_COMPILER_REQUIRE],
[m4_require([AC_LANG_COMPILER(]_AC_LANG[)],
            [AC_LANG_COMPILER])])



# _AC_LANG_COMPILER_GNU
# ---------------------
# Check whether the compiler for the current language is GNU.
#
# It doesn't seem necessary right now to have a different source
# according to the current language, since this works fine.  Some day
# it might be needed.  Nevertheless, pay attention to the fact that
# the position of `choke me' on the seventh column is meant: otherwise
# some Fortran compilers (e.g., SGI) might consider it's a
# continuation line, and warn instead of reporting an error.
m4_define([_AC_LANG_COMPILER_GNU],
[AC_CACHE_CHECK([whether we are using the GNU _AC_LANG compiler],
                [ac_cv_[]_AC_LANG_ABBREV[]_compiler_gnu],
[_AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [[#ifndef __GNUC__
       choke me
#endif
]])],
                   [ac_compiler_gnu=yes],
                   [ac_compiler_gnu=no])
ac_cv_[]_AC_LANG_ABBREV[]_compiler_gnu=$ac_compiler_gnu
])])# _AC_LANG_COMPILER_GNU


# AC_LANG_PREPROC
# ---------------
# Find a preprocessor for the current language.  Note that because we
# might AC_REQUIRE `AC_LANG_PREPROC(C)' for instance, the latter must
# be AC_DEFUN'd, not just define'd.  Since the preprocessor depends
# upon the compiler, look for the compiler.
m4_define([AC_LANG_PREPROC],
[AC_LANG_COMPILER_REQUIRE()dnl
_AC_LANG_DISPATCH([$0], _AC_LANG, $@)])


# AC_LANG_PREPROC_REQUIRE
# -----------------------
# Ensure we have a preprocessor for the current language.
AC_DEFUN([AC_LANG_PREPROC_REQUIRE],
[m4_require([AC_LANG_PREPROC(]_AC_LANG[)],
            [AC_LANG_PREPROC])])


# AC_REQUIRE_CPP
# --------------
# Require the preprocessor for the current language.
# FIXME: AU_ALIAS once AC_LANG is officially documented (2.51?).
AC_DEFUN([AC_REQUIRE_CPP],
[AC_LANG_PREPROC_REQUIRE])



# AC_NO_EXECUTABLES
# -----------------
# FIXME: The GCC team has specific needs which the current Autoconf
# framework cannot solve elegantly.  This macro implements a dirty
# hack until Autoconf is abble to provide the services its users
# needs.
#
# Several of the support libraries that are often built with GCC can't
# assume the tool-chain is already capable of linking a program: the
# compiler often expects to be able to link with some of such
# libraries.
#
# In several of these libraries, work-arounds have been introduced to
# avoid the AC_PROG_CC_WORKS test, that would just abort their
# configuration.  The introduction of AC_EXEEXT, enabled either by
# libtool or by CVS autoconf, have just made matters worse.
AC_DEFUN_ONCE([AC_NO_EXECUTABLES],
[m4_divert_push([KILL])

AC_BEFORE([$0], [_AC_COMPILER_EXEEXT_WORKS])
AC_BEFORE([$0], [_AC_COMPILER_EXEEXT])

m4_define([_AC_COMPILER_EXEEXT_WORKS],
[cross_compiling=maybe
])

m4_define([_AC_COMPILER_EXEEXT],
[EXEEXT=
])

m4_define([AC_LINK_IFELSE],
[AC_FATAL([All the tests involving linking were disabled by $0])])

m4_divert_pop()dnl
])# AC_NO_EXECUTABLES



# ----------------------------- #
# Computing EXEEXT and OBJEXT.  #
# ----------------------------- #


# Files to ignore
# ---------------
# Ignore .d files produced by CFLAGS=-MD.
#
# On UWIN (which uses a cc wrapper for MSVC), the compiler also generates
# a .pdb file
#
# When the w32 free Borland C++ command line compiler links a program
# (conftest.exe), it also produces a file named `conftest.tds' in
# addition to `conftest.obj'


# We must not AU define them, because autoupdate would then remove
# them, which is right, but Automake 1.4 would remove the support for
# $(EXEEXT) etc.
# FIXME: Remove this once Automake fixed.
AC_DEFUN([AC_EXEEXT],   [])
AC_DEFUN([AC_OBJEXT],   [])


# _AC_COMPILER_EXEEXT_DEFAULT
# ---------------------------
# Check for the extension used for the default name for executables.
# Beware of `expr' that may return `0' or `'.  Since this macro is
# the first one in touch with the compiler, it should also check that
# it compiles properly.
m4_define([_AC_COMPILER_EXEEXT_DEFAULT],
[# Try to create an executable without -o first, disregard a.out.
# It will help us diagnose broken compilers, and finding out an intuition
# of exeext.
AC_MSG_CHECKING([for _AC_LANG compiler default output])
ac_link_default=`echo "$ac_link" | sed ['s/ -o *conftest[^ ]*//']`
AS_IF([AC_TRY_EVAL(ac_link_default)],
[# Find the output, starting from the most likely.  This scheme is
# not robust to junk in `.', hence go to wildcards (a.*) only as a last
# resort.
for ac_file in `ls a.exe conftest.exe 2>/dev/null;
                ls a.out conftest 2>/dev/null;
                ls a.* conftest.* 2>/dev/null`; do
  case $ac_file in
    *.$ac_ext | *.o | *.obj | *.xcoff | *.tds | *.d | *.pdb ) ;;
    a.out ) # We found the default executable, but exeext='' is most
            # certainly right.
            break;;
    *.* ) ac_cv_exeext=`expr "$ac_file" : ['[^.]*\(\..*\)']`
          # FIXME: I believe we export ac_cv_exeext for Libtool --akim.
          export ac_cv_exeext
          break;;
    * ) break;;
  esac
done],
      [echo "$as_me: failed program was:" >&AS_MESSAGE_LOG_FD
cat conftest.$ac_ext >&AS_MESSAGE_LOG_FD
AC_MSG_ERROR([_AC_LANG compiler cannot create executables], 77)])
ac_exeext=$ac_cv_exeext
AC_MSG_RESULT([$ac_file])
])# _AC_COMPILER_EXEEXT_DEFAULT


# _AC_COMPILER_EXEEXT_WORKS
# -------------------------
m4_define([_AC_COMPILER_EXEEXT_WORKS],
[# Check the compiler produces executables we can run.  If not, either
# the compiler is broken, or we cross compile.
AC_MSG_CHECKING([whether the _AC_LANG compiler works])
# FIXME: These cross compiler hacks should be removed for Autoconf 3.0
# If not cross compiling, check that we can run a simple program.
if test "$cross_compiling" != yes; then
  if AC_TRY_COMMAND([./$ac_file]); then
    cross_compiling=no
  else
    if test "$cross_compiling" = maybe; then
	cross_compiling=yes
    else
	AC_MSG_ERROR([cannot run _AC_LANG compiled programs.
If you meant to cross compile, use `--host'.])
    fi
  fi
fi
AC_MSG_RESULT([yes])
])# _AC_COMPILER_EXEEXT_WORKS


# _AC_COMPILER_EXEEXT_CROSS
# -------------------------
m4_define([_AC_COMPILER_EXEEXT_CROSS],
[# Check the compiler produces executables we can run.  If not, either
# the compiler is broken, or we cross compile.
AC_MSG_CHECKING([whether we are cross compiling])
AC_MSG_RESULT([$cross_compiling])
])# _AC_COMPILER_EXEEXT_CROSS


# _AC_COMPILER_EXEEXT_O
# ---------------------
# Check for the extension used when `-o foo'.  Try to see if ac_cv_exeext,
# as computed by _AC_COMPILER_EXEEXT_DEFAULT is OK.
m4_define([_AC_COMPILER_EXEEXT_O],
[AC_MSG_CHECKING([for executable suffix])
AS_IF([AC_TRY_EVAL(ac_link)],
[# If both `conftest.exe' and `conftest' are `present' (well, observable)
# catch `conftest.exe'.  For instance with Cygwin, `ls conftest' will
# work properly (i.e., refer to `conftest.exe'), while it won't with
# `rm'.
for ac_file in `(ls conftest.exe; ls conftest; ls conftest.*) 2>/dev/null`; do
  case $ac_file in
    *.$ac_ext | *.o | *.obj | *.xcoff | *.tds | *.d | *.pdb ) ;;
    *.* ) ac_cv_exeext=`expr "$ac_file" : ['[^.]*\(\..*\)']`
          export ac_cv_exeext
          break;;
    * ) break;;
  esac
done],
              [AC_MSG_ERROR([cannot compute EXEEXT: cannot compile and link])])
rm -f conftest$ac_cv_exeext
AC_MSG_RESULT([$ac_cv_exeext])
])# _AC_COMPILER_EXEEXT_O


# _AC_COMPILER_EXEEXT
# -------------------
# Check for the extension used for executables.  It compiles a test
# executable.  If this is called, the executable extensions will be
# automatically used by link commands run by the configure script.
#
# Note that some compilers (cross or not), strictly obey to `-o foo' while
# the host requires `foo.exe', so we should not depend upon `-o' to
# test EXEEXT.  But then, be sure no to destroy user files.
#
# Must be run before _AC_COMPILER_OBJEXT because _AC_COMPILER_EXEEXT_DEFAULT
# checks whether the compiler works.
m4_define([_AC_COMPILER_EXEEXT],
[AC_LANG_CONFTEST([AC_LANG_PROGRAM()])
ac_clean_files_save=$ac_clean_files
ac_clean_files="$ac_clean_files a.out a.exe"
_AC_COMPILER_EXEEXT_DEFAULT
_AC_COMPILER_EXEEXT_WORKS
rm -f a.out a.exe conftest$ac_cv_exeext
ac_clean_files=$ac_clean_files_save
_AC_COMPILER_EXEEXT_CROSS
_AC_COMPILER_EXEEXT_O
rm -f conftest.$ac_ext
AC_SUBST([EXEEXT], [$ac_cv_exeext])dnl
ac_exeext=$EXEEXT
])# _AC_COMPILER_EXEEXT


# _AC_COMPILER_OBJEXT
# -------------------
# Check the object extension used by the compiler: typically `.o' or
# `.obj'.  If this is called, some other behaviour will change,
# determined by ac_objext.
#
# This macro is called by AC_LANG_COMPILER, the latter being required
# by the AC_COMPILE_IFELSE macros, so use _AC_COMPILE_IFELSE.  And in fact,
# don't, since _AC_COMPILE_IFELSE needs to know ac_objext for the `test -s'
# it includes.  So do it by hand.
m4_define([_AC_COMPILER_OBJEXT],
[AC_CACHE_CHECK([for object suffix], ac_cv_objext,
[AC_LANG_CONFTEST([AC_LANG_PROGRAM()])
rm -f conftest.o conftest.obj
AS_IF([AC_TRY_EVAL(ac_compile)],
[for ac_file in `(ls conftest.o conftest.obj; ls conftest.*) 2>/dev/null`; do
  case $ac_file in
    *.$ac_ext | *.xcoff | *.tds | *.d | *.pdb ) ;;
    *) ac_cv_objext=`expr "$ac_file" : '.*\.\(.*\)'`
       break;;
  esac
done],
      [echo "$as_me: failed program was:" >&AS_MESSAGE_LOG_FD
cat conftest.$ac_ext >&AS_MESSAGE_LOG_FD
AC_MSG_ERROR([cannot compute OBJEXT: cannot compile])])
rm -f conftest.$ac_cv_objext conftest.$ac_ext])
AC_SUBST([OBJEXT], [$ac_cv_objext])dnl
ac_objext=$OBJEXT
])# _AC_COMPILER_OBJEXT


# -------------------- #
# 3b. The C compiler.  #
# -------------------- #


# _AC_ARG_VAR_CPPFLAGS
# --------------------
# Document and register CPPFLAGS, which is used by
# AC_PROG_{CC, CPP, CXX, CXXCPP}.
AC_DEFUN([_AC_ARG_VAR_CPPFLAGS],
[AC_ARG_VAR([CPPFLAGS],
            [C/C++ preprocessor flags, e.g. -I<include dir> if you have
             headers in a nonstandard directory <include dir>])])


# _AC_ARG_VAR_LDFLAGS
# -------------------
# Document and register LDFLAGS, which is used by
# AC_PROG_{CC, CXX, F77}.
AC_DEFUN([_AC_ARG_VAR_LDFLAGS],
[AC_ARG_VAR([LDFLAGS],
            [linker flags, e.g. -L<lib dir> if you have libraries in a
             nonstandard directory <lib dir>])])



# AC_LANG_PREPROC(C)
# -------------------
# Find the C preprocessor.  Must be AC_DEFUN'd to be AC_REQUIRE'able.
AC_DEFUN([AC_LANG_PREPROC(C)],
[AC_REQUIRE([AC_PROG_CPP])])


# _AC_PROG_PREPROC_WORKS_IFELSE(IF-WORKS, IF-NOT)
# -----------------------------------------------
# Check if $ac_cpp is a working preprocessor that can flag absent
# includes either by the exit status or by warnings.
# Set ac_cpp_err to a non-empty value if the preprocessor failed.
# This macro is for all languages, not only C.
AC_DEFUN([_AC_PROG_PREPROC_WORKS_IFELSE],
[ac_preproc_ok=false
for ac_[]_AC_LANG_ABBREV[]_preproc_warn_flag in '' yes
do
  # Use a header file that comes with gcc, so configuring glibc
  # with a fresh cross-compiler works.
  # On the NeXT, cc -E runs the code through the compiler's parser,
  # not just through cpp. "Syntax error" is here to catch this case.
  _AC_PREPROC_IFELSE([AC_LANG_SOURCE([[@%:@include <assert.h>
                     Syntax error]])],
                     [],
                     [# Broken: fails on valid input.
continue])

  # OK, works on sane cases.  Now check whether non-existent headers
  # can be detected and how.
  _AC_PREPROC_IFELSE([AC_LANG_SOURCE([[@%:@include <ac_nonexistent.h>]])],
                     [# Broken: success on invalid input.
continue],
                     [# Passes both tests.
ac_preproc_ok=:
break])

done
# Because of `break', _AC_PREPROC_IFELSE's cleaning code was skipped.
rm -f conftest.err conftest.$ac_ext
AS_IF([$ac_preproc_ok], [$1], [$2])])# _AC_PROG_PREPROC_WORKS_IFELSE


# AC_PROG_CPP
# -----------
# Find a working C preprocessor.
# We shouldn't have to require AC_PROG_CC, but this is due to the concurrency
# between the AC_LANG_COMPILER_REQUIRE family and that of AC_PROG_CC.
AC_DEFUN([AC_PROG_CPP],
[AC_REQUIRE([AC_PROG_CC])dnl
AC_ARG_VAR([CPP],      [C preprocessor])dnl
_AC_ARG_VAR_CPPFLAGS()dnl
AC_LANG_PUSH(C)dnl
AC_MSG_CHECKING([how to run the C preprocessor])
# On Suns, sometimes $CPP names a directory.
if test -n "$CPP" && test -d "$CPP"; then
  CPP=
fi
if test -z "$CPP"; then
  AC_CACHE_VAL([ac_cv_prog_CPP],
  [dnl
    # Double quotes because CPP needs to be expanded
    for CPP in "$CC -E" "$CC -E -traditional-cpp" "/lib/cpp"
    do
      _AC_PROG_PREPROC_WORKS_IFELSE([break])
    done
    ac_cv_prog_CPP=$CPP
  ])dnl
  CPP=$ac_cv_prog_CPP
else
  ac_cv_prog_CPP=$CPP
fi
AC_MSG_RESULT([$CPP])
_AC_PROG_PREPROC_WORKS_IFELSE([],
                    [AC_MSG_ERROR([C preprocessor "$CPP" fails sanity check])])
AC_SUBST(CPP)dnl
AC_LANG_POP(C)dnl
])# AC_PROG_CPP


# AC_LANG_COMPILER(C)
# -------------------
# Find the C compiler.  Must be AC_DEFUN'd to be AC_REQUIRE'able.
AC_DEFUN([AC_LANG_COMPILER(C)],
[AC_REQUIRE([AC_PROG_CC])])


# ac_cv_prog_gcc
# --------------
# We used to name the cache variable this way.
AU_DEFUN([ac_cv_prog_gcc],
[ac_cv_c_compiler_gnu])


# AC_PROG_CC([COMPILER ...])
# --------------------------
# COMPILER ... is a space separated list of C compilers to search for.
# This just gives the user an opportunity to specify an alternative
# search list for the C compiler.
AC_DEFUN([AC_PROG_CC],
[AC_LANG_PUSH(C)dnl
AC_ARG_VAR([CC],     [C compiler command])dnl
AC_ARG_VAR([CFLAGS], [C compiler flags])dnl
_AC_ARG_VAR_LDFLAGS()dnl
_AC_ARG_VAR_CPPFLAGS()dnl
m4_ifval([$1],
      [AC_CHECK_TOOLS(CC, [$1])],
[AC_CHECK_TOOL(CC, gcc)
if test -z "$CC"; then
  AC_CHECK_TOOL(CC, cc)
fi
if test -z "$CC"; then
  AC_CHECK_PROG(CC, cc, cc, , , /usr/ucb/cc)
fi
if test -z "$CC"; then
  AC_CHECK_TOOLS(CC, cl)
fi
])

test -z "$CC" && AC_MSG_ERROR([no acceptable cc found in \$PATH])

# Provide some information about the compiler.
echo "$as_me:__oline__:" \
     "checking for _AC_LANG compiler version" >&AS_MESSAGE_LOG_FD
ac_compiler=`set X $ac_compile; echo $[2]`
_AC_EVAL([$ac_compiler --version </dev/null >&AS_MESSAGE_LOG_FD])
_AC_EVAL([$ac_compiler -v </dev/null >&AS_MESSAGE_LOG_FD])
_AC_EVAL([$ac_compiler -V </dev/null >&AS_MESSAGE_LOG_FD])

m4_expand_once([_AC_COMPILER_EXEEXT])[]dnl
m4_expand_once([_AC_COMPILER_OBJEXT])[]dnl
_AC_LANG_COMPILER_GNU
GCC=`test $ac_compiler_gnu = yes && echo yes`
_AC_PROG_CC_G
# Some people use a C++ compiler to compile C.  Since we use `exit',
# in C++ we need to declare it.  In case someone uses the same compiler
# for both compiling C and C++ we need to have the C++ compiler decide
# the declaration of exit, since it's the most demanding environment.
_AC_COMPILE_IFELSE([@%:@ifndef __cplusplus
  choke me
@%:@endif],
                   [_AC_PROG_CXX_EXIT_DECLARATION])
AC_LANG_POP(C)dnl
])# AC_PROG_CC


# _AC_PROG_CC_G
# -------------
# Check whether -g works, even if CFLAGS is set, in case the package
# plays around with CFLAGS (such as to build both debugging and normal
# versions of a library), tasteless as that idea is.
m4_define([_AC_PROG_CC_G],
[ac_test_CFLAGS=${CFLAGS+set}
ac_save_CFLAGS=$CFLAGS
CFLAGS="-g"
AC_CACHE_CHECK(whether $CC accepts -g, ac_cv_prog_cc_g,
               [_AC_COMPILE_IFELSE([AC_LANG_PROGRAM()], [ac_cv_prog_cc_g=yes],
                                                        [ac_cv_prog_cc_g=no])])
if test "$ac_test_CFLAGS" = set; then
  CFLAGS=$ac_save_CFLAGS
elif test $ac_cv_prog_cc_g = yes; then
  if test "$GCC" = yes; then
    CFLAGS="-g -O2"
  else
    CFLAGS="-g"
  fi
else
  if test "$GCC" = yes; then
    CFLAGS="-O2"
  else
    CFLAGS=
  fi
fi[]dnl
])# _AC_PROG_CC_G


# AC_PROG_GCC_TRADITIONAL
# -----------------------
AC_DEFUN([AC_PROG_GCC_TRADITIONAL],
[if test $ac_cv_c_compiler_gnu = yes; then
    AC_CACHE_CHECK(whether $CC needs -traditional,
      ac_cv_prog_gcc_traditional,
[  ac_pattern="Autoconf.*'x'"
  AC_EGREP_CPP($ac_pattern, [#include <sgtty.h>
Autoconf TIOCGETP],
  ac_cv_prog_gcc_traditional=yes, ac_cv_prog_gcc_traditional=no)

  if test $ac_cv_prog_gcc_traditional = no; then
    AC_EGREP_CPP($ac_pattern, [#include <termio.h>
Autoconf TCGETA],
    ac_cv_prog_gcc_traditional=yes)
  fi])
  if test $ac_cv_prog_gcc_traditional = yes; then
    CC="$CC -traditional"
  fi
fi
])# AC_PROG_GCC_TRADITIONAL


# AC_PROG_CC_C_O
# --------------
AC_DEFUN([AC_PROG_CC_C_O],
[AC_REQUIRE([AC_PROG_CC])dnl
if test "x$CC" != xcc; then
  AC_MSG_CHECKING([whether $CC and cc understand -c and -o together])
else
  AC_MSG_CHECKING([whether cc understands -c and -o together])
fi
set dummy $CC; ac_cc=`echo $[2] |
		      sed 's/[[^a-zA-Z0-9_]]/_/g;s/^[[0-9]]/_/'`
AC_CACHE_VAL(ac_cv_prog_cc_${ac_cc}_c_o,
[AC_LANG_CONFTEST([AC_LANG_PROGRAM([])])
# Make sure it works both with $CC and with simple cc.
# We do the test twice because some compilers refuse to overwrite an
# existing .o file with -o, though they will create one.
ac_try='$CC -c conftest.$ac_ext -o conftest.$ac_objext >&AS_MESSAGE_LOG_FD'
if AC_TRY_EVAL(ac_try) &&
   test -f conftest.$ac_objext && AC_TRY_EVAL(ac_try);
then
  eval ac_cv_prog_cc_${ac_cc}_c_o=yes
  if test "x$CC" != xcc; then
    # Test first that cc exists at all.
    if AC_TRY_COMMAND(cc -c conftest.$ac_ext >&AS_MESSAGE_LOG_FD); then
      ac_try='cc -c conftest.$ac_ext -o conftest.$ac_objext >&AS_MESSAGE_LOG_FD'
      if AC_TRY_EVAL(ac_try) &&
	 test -f conftest.$ac_objext && AC_TRY_EVAL(ac_try);
      then
        # cc works too.
        :
      else
        # cc exists but doesn't like -o.
        eval ac_cv_prog_cc_${ac_cc}_c_o=no
      fi
    fi
  fi
else
  eval ac_cv_prog_cc_${ac_cc}_c_o=no
fi
rm -f conftest*
])dnl
if eval "test \"`echo '$ac_cv_prog_cc_'${ac_cc}_c_o`\" = yes"; then
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
  AC_DEFINE(NO_MINUS_C_MINUS_O, 1,
            [Define if your C compiler doesn't accept -c and -o together.])
fi
])# AC_PROG_CC_C_O


# ---------------------- #
# 3c. The C++ compiler.  #
# ---------------------- #


# AC_LANG_PREPROC(C++)
# ---------------------
# Find the C++ preprocessor.  Must be AC_DEFUN'd to be AC_REQUIRE'able.
AC_DEFUN([AC_LANG_PREPROC(C++)],
[AC_REQUIRE([AC_PROG_CXXCPP])])


# AC_PROG_CXXCPP
# --------------
# Find a working C++ preprocessor.
# We shouldn't have to require AC_PROG_CC, but this is due to the concurrency
# between the AC_LANG_COMPILER_REQUIRE family and that of AC_PROG_CXX.
AC_DEFUN([AC_PROG_CXXCPP],
[AC_REQUIRE([AC_PROG_CXX])dnl
AC_ARG_VAR([CXXCPP],   [C++ preprocessor])dnl
_AC_ARG_VAR_CPPFLAGS()dnl
AC_LANG_PUSH(C++)dnl
AC_MSG_CHECKING([how to run the C++ preprocessor])
if test -z "$CXXCPP"; then
  AC_CACHE_VAL(ac_cv_prog_CXXCPP,
  [dnl
    # Double quotes because CXXCPP needs to be expanded
    for CXXCPP in "$CXX -E" "/lib/cpp"
    do
      _AC_PROG_PREPROC_WORKS_IFELSE([break])
    done
    ac_cv_prog_CXXCPP=$CXXCPP
  ])dnl
  CXXCPP=$ac_cv_prog_CXXCPP
else
  ac_cv_prog_CXXCPP=$CXXCPP
fi
AC_MSG_RESULT([$CXXCPP])
_AC_PROG_PREPROC_WORKS_IFELSE([],
              [AC_MSG_ERROR([C++ preprocessor "$CXXCPP" fails sanity check])])
AC_SUBST(CXXCPP)dnl
AC_LANG_POP(C++)dnl
])# AC_PROG_CXXCPP


# AC_LANG_COMPILER(C++)
# ---------------------
# Find the C++ compiler.  Must be AC_DEFUN'd to be AC_REQUIRE'able.
AC_DEFUN([AC_LANG_COMPILER(C++)],
[AC_REQUIRE([AC_PROG_CXX])])


# ac_cv_prog_gxx
# --------------
# We used to name the cache variable this way.
AU_DEFUN([ac_cv_prog_gxx],
[ac_cv_cxx_compiler_gnu])


# AC_PROG_CXX([LIST-OF-COMPILERS])
# --------------------------------
# LIST-OF-COMPILERS is a space separated list of C++ compilers to search
# for (if not specified, a default list is used).  This just gives the
# user an opportunity to specify an alternative search list for the C++
# compiler.
# aCC	HP-UX C++ compiler much better than `CC', so test before.
# FCC   Fujitsu C++ compiler
# KCC	KAI C++ compiler
# RCC	Rational C++
# xlC_r	AIX C Set++ (with support for reentrant code)
# xlC	AIX C Set++
AC_DEFUN([AC_PROG_CXX],
[AC_LANG_PUSH(C++)dnl
AC_ARG_VAR([CXX],      [C++ compiler command])dnl
AC_ARG_VAR([CXXFLAGS], [C++ compiler flags])dnl
_AC_ARG_VAR_LDFLAGS()dnl
_AC_ARG_VAR_CPPFLAGS()dnl
AC_CHECK_TOOLS(CXX,
               [$CCC m4_default([$1],
                          [g++ c++ gpp aCC CC cxx cc++ cl FCC KCC RCC xlC_r xlC])],
               g++)

# Provide some information about the compiler.
echo "$as_me:__oline__:" \
     "checking for _AC_LANG compiler version" >&AS_MESSAGE_LOG_FD
ac_compiler=`set X $ac_compile; echo $[2]`
_AC_EVAL([$ac_compiler --version </dev/null >&AS_MESSAGE_LOG_FD])
_AC_EVAL([$ac_compiler -v </dev/null >&AS_MESSAGE_LOG_FD])
_AC_EVAL([$ac_compiler -V </dev/null >&AS_MESSAGE_LOG_FD])

m4_expand_once([_AC_COMPILER_EXEEXT])[]dnl
m4_expand_once([_AC_COMPILER_OBJEXT])[]dnl
_AC_LANG_COMPILER_GNU
GXX=`test $ac_compiler_gnu = yes && echo yes`
_AC_PROG_CXX_G
_AC_PROG_CXX_EXIT_DECLARATION
AC_LANG_POP(C++)dnl
])# AC_PROG_CXX


# _AC_PROG_CXX_G
# --------------
# Check whether -g works, even if CXXFLAGS is set, in case the package
# plays around with CXXFLAGS (such as to build both debugging and
# normal versions of a library), tasteless as that idea is.
m4_define([_AC_PROG_CXX_G],
[ac_test_CXXFLAGS=${CXXFLAGS+set}
ac_save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="-g"
AC_CACHE_CHECK(whether $CXX accepts -g, ac_cv_prog_cxx_g,
               [_AC_COMPILE_IFELSE([AC_LANG_PROGRAM()],
                                   [ac_cv_prog_cxx_g=yes],
                                   [ac_cv_prog_cxx_g=no])])
if test "$ac_test_CXXFLAGS" = set; then
  CXXFLAGS=$ac_save_CXXFLAGS
elif test $ac_cv_prog_cxx_g = yes; then
  if test "$GXX" = yes; then
    CXXFLAGS="-g -O2"
  else
    CXXFLAGS="-g"
  fi
else
  if test "$GXX" = yes; then
    CXXFLAGS="-O2"
  else
    CXXFLAGS=
  fi
fi[]dnl
])# _AC_PROG_CXX_G


# _AC_PROG_CXX_EXIT_DECLARATION
# -----------------------------
# Find a valid prototype for exit and declare it in confdefs.h.
m4_define([_AC_PROG_CXX_EXIT_DECLARATION],
[for ac_declaration in \
   ''\
   '#include <stdlib.h>' \
   'extern "C" void std::exit (int) throw (); using std::exit;' \
   'extern "C" void std::exit (int); using std::exit;' \
   'extern "C" void exit (int) throw ();' \
   'extern "C" void exit (int);' \
   'void exit (int);'
do
  _AC_COMPILE_IFELSE([AC_LANG_PROGRAM([@%:@include <stdlib.h>
$ac_declaration],
                                      [exit (42);])],
                     [],
                     [continue])
  _AC_COMPILE_IFELSE([AC_LANG_PROGRAM([$ac_declaration],
                                      [exit (42);])],
                     [break])
done
rm -f conftest*
if test -n "$ac_declaration"; then
  echo '#ifdef __cplusplus' >>confdefs.h
  echo $ac_declaration      >>confdefs.h
  echo '#endif'             >>confdefs.h
fi
])# _AC_PROG_CXX_EXIT_DECLARATION


# ----------------------------- #
# 3d. The Fortran 77 compiler.  #
# ----------------------------- #


# AC_LANG_PREPROC(Fortran 77)
# ---------------------------
# Find the Fortran 77 preprocessor.  Must be AC_DEFUN'd to be AC_REQUIRE'able.
AC_DEFUN([AC_LANG_PREPROC(Fortran 77)],
[m4_warn([syntax],
         [$0: No preprocessor defined for ]_AC_LANG)])


# AC_LANG_COMPILER(Fortran 77)
# ----------------------------
# Find the Fortran 77 compiler.  Must be AC_DEFUN'd to be
# AC_REQUIRE'able.
AC_DEFUN([AC_LANG_COMPILER(Fortran 77)],
[AC_REQUIRE([AC_PROG_F77])])


# ac_cv_prog_g77
# --------------
# We used to name the cache variable this way.
AU_DEFUN([ac_cv_prog_g77],
[ac_cv_f77_compiler_gnu])


# AC_PROG_F77([COMPILERS...])
# ---------------------------
# COMPILERS is a space separated list of Fortran 77 compilers to search
# for.
# Fortran 95 isn't strictly backwards-compatiable with Fortran 77, but
# `f95' is worth trying.
#
# Compilers are ordered by
#  1. F77, F90, F95
#  2. Good/tested native compilers, bad/untested native compilers
#  3. Wrappers around f2c go last.
#
# `fort77' and `fc' are wrappers around `f2c', `fort77' being better.
# It is believed that under HP-UX `fort77' is the name of the native
# compiler.  On some Cray systems, fort77 is a native compiler.
# cf77 and cft77 are (older) Cray F77 compilers.
# frt is the Fujitsu F77 compiler.
# pgf77 and pgf90 are the Portland Group F77 and F90 compilers.
# xlf/xlf90/xlf95 are IBM (AIX) F77/F90/F95 compilers.
# lf95 is the Lahey-Fujitsu compiler.
# fl32 is the Microsoft Fortran "PowerStation" compiler.
# af77 is the Apogee F77 compiler for Intergraph hardware running CLIX.
# epcf90 is the "Edinburgh Portable Compiler" F90.
# fort is the Compaq Fortran 90 (now 95) compiler for Tru64 and Linux/Alpha.
AC_DEFUN([AC_PROG_F77],
[AC_LANG_PUSH(Fortran 77)dnl
AC_ARG_VAR([F77],    [Fortran 77 compiler command])dnl
AC_ARG_VAR([FFLAGS], [Fortran 77 compiler flags])dnl
_AC_ARG_VAR_LDFLAGS()dnl
AC_CHECK_TOOLS(F77,
      [m4_default([$1],
                  [g77 f77 xlf cf77 cft77 frt pgf77 fl32 af77 fort77 f90 xlf90 pgf90 epcf90 f95 fort xlf95 lf95 g95 fc])])

# Provide some information about the compiler.
echo "$as_me:__oline__:" \
     "checking for _AC_LANG compiler version" >&AS_MESSAGE_LOG_FD
ac_compiler=`set X $ac_compile; echo $[2]`
_AC_EVAL([$ac_compiler --version </dev/null >&AS_MESSAGE_LOG_FD])
_AC_EVAL([$ac_compiler -v </dev/null >&AS_MESSAGE_LOG_FD])
_AC_EVAL([$ac_compiler -V </dev/null >&AS_MESSAGE_LOG_FD])

m4_expand_once([_AC_COMPILER_EXEEXT])[]dnl
m4_expand_once([_AC_COMPILER_OBJEXT])[]dnl
# If we don't use `.F' as extension, the preprocessor is not run on the
# input file.
ac_save_ext=$ac_ext
ac_ext=F
_AC_LANG_COMPILER_GNU
ac_ext=$ac_save_ext
G77=`test $ac_compiler_gnu = yes && echo yes`
_AC_PROG_F77_G
AC_LANG_POP(Fortran 77)dnl
])# AC_PROG_F77


# _AC_PROG_F77_G
# --------------
# Check whether -g works, even if FFLAGS is set, in case the package
# plays around with FFLAGS (such as to build both debugging and normal
# versions of a library), tasteless as that idea is.
m4_define([_AC_PROG_F77_G],
[ac_test_FFLAGS=${FFLAGS+set}
ac_save_FFLAGS=$FFLAGS
FFLAGS=
AC_CACHE_CHECK(whether $F77 accepts -g, ac_cv_prog_f77_g,
[FFLAGS=-g
_AC_COMPILE_IFELSE([AC_LANG_PROGRAM()],
[ac_cv_prog_f77_g=yes],
[ac_cv_prog_f77_g=no])
])
if test "$ac_test_FFLAGS" = set; then
  FFLAGS=$ac_save_FFLAGS
elif test $ac_cv_prog_f77_g = yes; then
  if test "$G77" = yes; then
    FFLAGS="-g -O2"
  else
    FFLAGS="-g"
  fi
else
  if test "$G77" = yes; then
    FFLAGS="-O2"
  else
    FFLAGS=
  fi
fi[]dnl
])# _AC_PROG_F77_G


# AC_PROG_F77_C_O
# ---------------
# Test if the Fortran 77 compiler accepts the options `-c' and `-o'
# simultaneously, and define `F77_NO_MINUS_C_MINUS_O' if it does not.
#
# The usefulness of this macro is questionable, as I can't really see
# why anyone would use it.  The only reason I include it is for
# completeness, since a similar test exists for the C compiler.
AC_DEFUN([AC_PROG_F77_C_O],
[AC_REQUIRE([AC_PROG_F77])dnl
AC_CACHE_CHECK([whether $F77 understand -c and -o together],
               [ac_cv_prog_f77_c_o],
[AC_LANG_CONFTEST([AC_LANG_PROGRAM([])])
# We test twice because some compilers refuse to overwrite an existing
# `.o' file with `-o', although they will create one.
ac_try='$F77 $FFLAGS -c conftest.$ac_ext -o conftest.$ac_objext >&AS_MESSAGE_LOG_FD'
if AC_TRY_EVAL(ac_try) &&
     test -f conftest.$ac_objext &&
     AC_TRY_EVAL(ac_try); then
  ac_cv_prog_f77_c_o=yes
else
  ac_cv_prog_f77_c_o=no
fi
rm -f conftest*])
if test $ac_cv_prog_f77_c_o = no; then
  AC_DEFINE(F77_NO_MINUS_C_MINUS_O, 1,
            [Define if your Fortran 77 compiler doesn't accept -c and -o together.])
fi
])# AC_PROG_F77_C_O





## ------------------------------- ##
## 4. Compilers' characteristics.  ##
## ------------------------------- ##


# -------------------------------- #
# 4b. C compiler characteristics.  #
# -------------------------------- #

# AC_PROG_CC_STDC
# ---------------
# If the C compiler in not in ANSI C mode by default, try to add an
# option to output variable @code{CC} to make it so.  This macro tries
# various options that select ANSI C on some system or another.  It
# considers the compiler to be in ANSI C mode if it handles function
# prototypes correctly.
AC_DEFUN([AC_PROG_CC_STDC],
[AC_REQUIRE([AC_PROG_CC])dnl
AC_BEFORE([$0], [AC_C_INLINE])dnl
AC_BEFORE([$0], [AC_C_CONST])dnl
dnl Force this before AC_PROG_CPP.  Some cpp's, eg on HPUX, require
dnl a magic option to avoid problems with ANSI preprocessor commands
dnl like #elif.
dnl FIXME: can't do this because then AC_AIX won't work due to a
dnl circular dependency.
dnl AC_BEFORE([$0], [AC_PROG_CPP])
AC_MSG_CHECKING([for $CC option to accept ANSI C])
AC_CACHE_VAL(ac_cv_prog_cc_stdc,
[ac_cv_prog_cc_stdc=no
ac_save_CC=$CC
AC_LANG_CONFTEST([AC_LANG_PROGRAM(
[[#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
/* Most of the following tests are stolen from RCS 5.7's src/conf.sh.  */
struct buf { int x; };
FILE * (*rcsopen) (struct buf *, struct stat *, int);
static char *e (p, i)
     char **p;
     int i;
{
  return p[i];
}
static char *f (char * (*g) (char **, int), char **p, ...)
{
  char *s;
  va_list v;
  va_start (v,p);
  s = g (p, va_arg (v,int));
  va_end (v);
  return s;
}
int test (int i, double x);
struct s1 {int (*f) (int a);};
struct s2 {int (*f) (double a);};
int pairnames (int, char **, FILE *(*)(struct buf *, struct stat *, int), int, int);
int argc;
char **argv;]],
[[return f (e, argv, 0) != argv[0]  ||  f (e, argv, 1) != argv[1];]])])
# Don't try gcc -ansi; that turns off useful extensions and
# breaks some systems' header files.
# AIX			-qlanglvl=ansi
# Ultrix and OSF/1	-std1
# HP-UX 10.20 and later	-Ae
# HP-UX older versions	-Aa -D_HPUX_SOURCE
# SVR4			-Xc -D__EXTENSIONS__
for ac_arg in "" -qlanglvl=ansi -std1 -Ae "-Aa -D_HPUX_SOURCE" "-Xc -D__EXTENSIONS__"
do
  CC="$ac_save_CC $ac_arg"
  AC_COMPILE_IFELSE([],
                    [ac_cv_prog_cc_stdc=$ac_arg
break])
done
rm -f conftest.$ac_ext conftest.$ac_objext
CC=$ac_save_CC
])
case "x$ac_cv_prog_cc_stdc" in
  x|xno)
    AC_MSG_RESULT([none needed]) ;;
  *)
    AC_MSG_RESULT([$ac_cv_prog_cc_stdc])
    CC="$CC $ac_cv_prog_cc_stdc" ;;
esac
])# AC_PROG_CC_STDC


# AC_C_CROSS
# ----------
# Has been merged into AC_PROG_CC.
AU_DEFUN([AC_C_CROSS], [])


# AC_C_CHAR_UNSIGNED
# ------------------
AC_DEFUN([AC_C_CHAR_UNSIGNED],
[AH_VERBATIM([__CHAR_UNSIGNED__],
[/* Define if type `char' is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
# undef __CHAR_UNSIGNED__
#endif])dnl
AC_CACHE_CHECK(whether char is unsigned, ac_cv_c_char_unsigned,
[AC_COMPILE_IFELSE([AC_LANG_BOOL_COMPILE_TRY([AC_INCLUDES_DEFAULT([])],
                                             [((char) -1) < 0])],
                   ac_cv_c_char_unsigned=no, ac_cv_c_char_unsigned=yes)])
if test $ac_cv_c_char_unsigned = yes && test "$GCC" != yes; then
  AC_DEFINE(__CHAR_UNSIGNED__)
fi
])# AC_C_CHAR_UNSIGNED


# AC_C_LONG_DOUBLE
# ----------------
AC_DEFUN([AC_C_LONG_DOUBLE],
[AC_CACHE_CHECK(for long double, ac_cv_c_long_double,
[if test "$GCC" = yes; then
  ac_cv_c_long_double=yes
else
AC_TRY_RUN(
[int
main ()
{
  /* The Stardent Vistra knows sizeof(long double), but does not
     support it.  */
  long double foo = 0.0;
  /* On Ultrix 4.3 cc, long double is 4 and double is 8.  */
  exit (sizeof (long double) < sizeof (double));
}],
ac_cv_c_long_double=yes, ac_cv_c_long_double=no)
fi])
if test $ac_cv_c_long_double = yes; then
  AC_DEFINE(HAVE_LONG_DOUBLE, 1,
            [Define if the `long double' type works.])
fi
])# AC_C_LONG_DOUBLE


# AC_C_BIGENDIAN
# --------------
AC_DEFUN([AC_C_BIGENDIAN],
[AC_CACHE_CHECK(whether byte ordering is bigendian, ac_cv_c_bigendian,
[ac_cv_c_bigendian=unknown
# See if sys/param.h defines the BYTE_ORDER macro.
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <sys/types.h>
#include <sys/param.h>
],
[#if !BYTE_ORDER || !BIG_ENDIAN || !LITTLE_ENDIAN
 bogus endian macros
#endif
])],
[# It does; now see whether it defined to BIG_ENDIAN or not.
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <sys/types.h>
#include <sys/param.h>
], [#if BYTE_ORDER != BIG_ENDIAN
 not big endian
#endif
])],               [ac_cv_c_bigendian=yes],
                   [ac_cv_c_bigendian=no])])
if test $ac_cv_c_bigendian = unknown; then
AC_TRY_RUN(
[int
main ()
{
  /* Are we little or big endian?  From Harbison&Steele.  */
  union
  {
    long l;
    char c[sizeof (long)];
  } u;
  u.l = 1;
  exit (u.c[sizeof (long) - 1] == 1);
}], ac_cv_c_bigendian=no, ac_cv_c_bigendian=yes)
fi])
if test $ac_cv_c_bigendian = yes; then
  AC_DEFINE(WORDS_BIGENDIAN, 1,
            [Define if your processor stores words with the most significant
             byte first (like Motorola and SPARC, unlike Intel and VAX).])
fi
])# AC_C_BIGENDIAN


# AC_C_INLINE
# -----------
# Do nothing if the compiler accepts the inline keyword.
# Otherwise define inline to __inline__ or __inline if one of those work,
# otherwise define inline to be empty.
AC_DEFUN([AC_C_INLINE],
[AC_REQUIRE([AC_PROG_CC_STDC])dnl
AC_CACHE_CHECK([for inline], ac_cv_c_inline,
[ac_cv_c_inline=no
for ac_kw in inline __inline__ __inline; do
  AC_COMPILE_IFELSE([AC_LANG_SOURCE(
[#ifndef __cplusplus
static $ac_kw int static_foo () {return 0; }
$ac_kw int foo () {return 0; }
#endif
])],
                    [ac_cv_c_inline=$ac_kw; break])
done
])
case $ac_cv_c_inline in
  inline | yes) ;;
  no) AC_DEFINE(inline,,
                [Define as `__inline' if that's what the C compiler calls it,
                 or to nothing if it is not supported.]) ;;
  *)  AC_DEFINE_UNQUOTED(inline, $ac_cv_c_inline) ;;
esac
])# AC_C_INLINE


# AC_C_CONST
# ----------
AC_DEFUN([AC_C_CONST],
[AC_REQUIRE([AC_PROG_CC_STDC])dnl
AC_CACHE_CHECK([for an ANSI C-conforming const], ac_cv_c_const,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],
[[/* FIXME: Include the comments suggested by Paul. */
#ifndef __cplusplus
  /* Ultrix mips cc rejects this.  */
  typedef int charset[2];
  const charset x;
  /* SunOS 4.1.1 cc rejects this.  */
  char const *const *ccp;
  char **p;
  /* NEC SVR4.0.2 mips cc rejects this.  */
  struct point {int x, y;};
  static struct point const zero = {0,0};
  /* AIX XL C 1.02.0.0 rejects this.
     It does not let you subtract one const X* pointer from another in
     an arm of an if-expression whose if-part is not a constant
     expression */
  const char *g = "string";
  ccp = &g + (g ? g-g : 0);
  /* HPUX 7.0 cc rejects these. */
  ++ccp;
  p = (char**) ccp;
  ccp = (char const *const *) p;
  { /* SCO 3.2v4 cc rejects this.  */
    char *t;
    char const *s = 0 ? (char *) 0 : (char const *) 0;

    *t++ = 0;
  }
  { /* Someone thinks the Sun supposedly-ANSI compiler will reject this.  */
    int x[] = {25, 17};
    const int *foo = &x[0];
    ++foo;
  }
  { /* Sun SC1.0 ANSI compiler rejects this -- but not the above. */
    typedef const int *iptr;
    iptr p = 0;
    ++p;
  }
  { /* AIX XL C 1.02.0.0 rejects this saying
       "k.c", line 2.27: 1506-025 (S) Operand must be a modifiable lvalue. */
    struct s { int j; const int *ap[3]; };
    struct s *b; b->j = 5;
  }
  { /* ULTRIX-32 V3.1 (Rev 9) vcc rejects this */
    const int foo = 10;
  }
#endif
]])],
                   [ac_cv_c_const=yes],
                   [ac_cv_c_const=no])])
if test $ac_cv_c_const = no; then
  AC_DEFINE(const,,
            [Define to empty if `const' does not conform to ANSI C.])
fi
])# AC_C_CONST


# AC_C_VOLATILE
# -------------
# Note that, unlike const, #defining volatile to be the empty string can
# actually turn a correct program into an incorrect one, since removing
# uses of volatile actually grants the compiler permission to perform
# optimizations that could break the user's code.  So, do not #define
# volatile away unless it is really necessary to allow the user's code
# to compile cleanly.  Benign compiler failures should be tolerated.
AC_DEFUN([AC_C_VOLATILE],
[AC_REQUIRE([AC_PROG_CC_STDC])dnl
AC_CACHE_CHECK([for working volatile], ac_cv_c_volatile,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [
volatile int x;
int * volatile y;])],
                   [ac_cv_c_volatile=yes],
                   [ac_cv_c_volatile=no])])
if test $ac_cv_c_volatile = no; then
  AC_DEFINE(volatile,,
            [Define to empty if the keyword `volatile' does not work.
             Warning: valid code using `volatile' can become incorrect
             without.  Disable with care.])
fi
])# AC_C_VOLATILE


# AC_C_STRINGIZE
# --------------
# Checks if `#' can be used to glue strings together at the CPP level.
# Defines HAVE_STRINGIZE if positive.
AC_DEFUN([AC_C_STRINGIZE],
[AC_CACHE_CHECK([for preprocessor stringizing operator],
                [ac_cv_c_stringize],
[AC_EGREP_CPP([@%:@teststring],
              [@%:@define x(y) #y

char *s = x(teststring);],
              [ac_cv_c_stringize=no],
              [ac_cv_c_stringize=yes])])
if test $ac_cv_c_stringize = yes; then
  AC_DEFINE(HAVE_STRINGIZE, 1,
            [Define if cpp supports the ANSI @%:@ stringizing operator.])
fi
])# AC_C_STRINGIZE


# AC_C_PROTOTYPES
# ---------------
# Check if the C compiler supports prototypes, included if it needs
# options.
AC_DEFUN([AC_C_PROTOTYPES],
[AC_REQUIRE([AC_PROG_CC_STDC])dnl
AC_MSG_CHECKING([for function prototypes])
if test "$ac_cv_prog_cc_stdc" != no; then
  AC_MSG_RESULT([yes])
  AC_DEFINE(PROTOTYPES, 1,
            [Define if the C compiler supports function prototypes.])
else
  AC_MSG_RESULT([no])
fi
])# AC_C_PROTOTYPES




# ---------------------------------------- #
# 4d. Fortan 77 compiler characteristics.  #
# ---------------------------------------- #


# _AC_PROG_F77_V_OUTPUT([FLAG = $ac_cv_prog_f77_v])
# -------------------------------------------------
# Link a trivial Fortran program, compiling with a verbose output FLAG
# (which default value, $ac_cv_prog_f77_v, is computed by
# _AC_PROG_F77_V), and return the output in $ac_f77_v_output.  This
# output is processed in the way expected by AC_F77_LIBRARY_LDFLAGS,
# so that any link flags that are echoed by the compiler appear as
# space-separated items.
AC_DEFUN([_AC_PROG_F77_V_OUTPUT],
[AC_REQUIRE([AC_PROG_F77])dnl
AC_LANG_PUSH(Fortran 77)dnl

AC_LANG_CONFTEST([AC_LANG_PROGRAM([])])

# Compile and link our simple test program by passing a flag (argument
# 1 to this macro) to the Fortran 77 compiler in order to get
# "verbose" output that we can then parse for the Fortran 77 linker
# flags.
ac_save_FFLAGS=$FFLAGS
FFLAGS="$FFLAGS m4_default([$1], [$ac_cv_prog_f77_v])"
(eval echo $as_me:__oline__: \"$ac_link\") >&AS_MESSAGE_LOG_FD
ac_f77_v_output=`eval $ac_link AS_MESSAGE_LOG_FD>&1 2>&1 | grep -v 'Driving:'`
echo "$ac_f77_v_output" >&AS_MESSAGE_LOG_FD
FFLAGS=$ac_save_FFLAGS

rm -f conftest*
AC_LANG_POP(Fortran 77)dnl

# If we are using xlf then replace all the commas with spaces.
if echo $ac_f77_v_output | grep xlfentry >/dev/null 2>&1; then
  ac_f77_v_output=`echo $ac_f77_v_output | sed 's/,/ /g'`
fi

# If we are using Cray Fortran then delete quotes.
# Use "\"" instead of '"' for font-lock-mode.
# FIXME: a more general fix for quoted arguments with spaces?
if echo $ac_f77_v_output | grep cft90 >/dev/null 2>&1; then
  ac_f77_v_output=`echo $ac_f77_v_output | sed "s/\"//g"`
fi[]dnl
])# _AC_PROG_F77_V_OUTPUT


# _AC_PROG_F77_V
# --------------
#
# Determine the flag that causes the Fortran 77 compiler to print
# information of library and object files (normally -v)
# Needed for AC_F77_LIBRARY_FLAGS
# Some compilers don't accept -v (Lahey: -verbose, xlf: -V, Fujitsu: -###)
AC_DEFUN([_AC_PROG_F77_V],
[AC_CACHE_CHECK([how to get verbose linking output from $F77],
                [ac_cv_prog_f77_v],
[AC_LANG_ASSERT(Fortran 77)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM()],
[ac_cv_prog_f77_v=
# Try some options frequently used verbose output
for ac_verb in -v -verbose --verbose -V -\#\#\#; do
  _AC_PROG_F77_V_OUTPUT($ac_verb)
  # look for -l* and *.a constructs in the output
  for ac_arg in $ac_f77_v_output; do
     case $ac_arg in
        [[\\/]]*.a | ?:[[\\/]]*.a | -[[lLRu]]*)
          ac_cv_prog_f77_v=$ac_verb
          break 2 ;;
     esac
  done
done
if test -z "$ac_cv_prog_f77_v"; then
   AC_MSG_WARN([cannot determine how to obtain linking information from $F77])
fi],
                  [AC_MSG_WARN([compilation failed])])
])])# _AC_PROG_F77_V


# AC_F77_LIBRARY_LDFLAGS
# ----------------------
#
# Determine the linker flags (e.g. "-L" and "-l") for the Fortran 77
# intrinsic and run-time libraries that are required to successfully
# link a Fortran 77 program or shared library.  The output variable
# FLIBS is set to these flags.
#
# This macro is intended to be used in those situations when it is
# necessary to mix, e.g. C++ and Fortran 77, source code into a single
# program or shared library.
#
# For example, if object files from a C++ and Fortran 77 compiler must
# be linked together, then the C++ compiler/linker must be used for
# linking (since special C++-ish things need to happen at link time
# like calling global constructors, instantiating templates, enabling
# exception support, etc.).
#
# However, the Fortran 77 intrinsic and run-time libraries must be
# linked in as well, but the C++ compiler/linker doesn't know how to
# add these Fortran 77 libraries.  Hence, the macro
# "AC_F77_LIBRARY_LDFLAGS" was created to determine these Fortran 77
# libraries.
#
# This macro was packaged in its current form by Matthew D. Langston.
# However, nearly all of this macro came from the "OCTAVE_FLIBS" macro
# in "octave-2.0.13/aclocal.m4", and full credit should go to John
# W. Eaton for writing this extremely useful macro.  Thank you John.
AC_DEFUN([AC_F77_LIBRARY_LDFLAGS],
[AC_LANG_PUSH(Fortran 77)dnl
_AC_PROG_F77_V
AC_CACHE_CHECK([for Fortran 77 libraries], ac_cv_flibs,
[if test "x$FLIBS" != "x"; then
  ac_cv_flibs="$FLIBS" # Let the user override the test.
else

_AC_PROG_F77_V_OUTPUT

ac_cv_flibs=

# Save positional arguments (if any)
ac_save_positional="$[@]"

set X $ac_f77_v_output
while test $[@%:@] != 1; do
  shift
  ac_arg=$[1]
  case $ac_arg in
        [[\\/]]*.a | ?:[[\\/]]*.a)
          AC_LIST_MEMBER_OF($ac_arg, $ac_cv_flibs, ,
              ac_cv_flibs="$ac_cv_flibs $ac_arg")
          ;;
        -bI:*)
          AC_LIST_MEMBER_OF($ac_arg, $ac_cv_flibs, ,
             [AC_LINKER_OPTION([$ac_arg], ac_cv_flibs)])
          ;;
          # Ignore these flags.
        -lang* | -lcrt0.o | -lc | -lgcc | -LANG:=*)
          ;;
        -lkernel32)
          test x"$CYGWIN" != xyes && ac_cv_flibs="$ac_cv_flibs $ac_arg"
          ;;
        -[[LRuY]])
          # These flags, when seen by themselves, take an argument.
          # We remove the space between option and argument and re-iterate
          # unless we find an empty arg or a new option (starting with -)
	  case $[2] in
             "" | -*);;
             *)
		ac_arg="$ac_arg$[2]"
		shift; shift
		set X $ac_arg "$[@]"
		;;
	  esac
          ;;
        -YP,*)
          for ac_j in `echo $ac_arg | sed -e 's/-YP,/-L/;s/:/ -L/g'`; do
            AC_LIST_MEMBER_OF($ac_j, $ac_cv_flibs, ,
                            [ac_arg="$ac_arg $ac_j"
                             ac_cv_flibs="$ac_cv_flibs $ac_j"])
          done
          ;;
        -[[lLR]]*)
          AC_LIST_MEMBER_OF($ac_arg, $ac_cv_flibs, ,
                          ac_cv_flibs="$ac_cv_flibs $ac_arg")
          ;;
          # Ignore everything else.
  esac
done
# restore positional arguments
set X $ac_save_positional; shift

# We only consider "LD_RUN_PATH" on Solaris systems.  If this is seen,
# then we insist that the "run path" must be an absolute path (i.e. it
# must begin with a "/").
case `(uname -sr) 2>/dev/null` in
   "SunOS 5"*)
      ac_ld_run_path=`echo $ac_f77_v_output |
                        sed -n 's,^.*LD_RUN_PATH *= *\(/[[^ ]]*\).*$,-R\1,p'`
      test "x$ac_ld_run_path" != x &&
        AC_LINKER_OPTION([$ac_ld_run_path], ac_cv_flibs)
      ;;
esac
fi # test "x$FLIBS" = "x"
])
FLIBS="$ac_cv_flibs"
AC_SUBST(FLIBS)
AC_LANG_POP(Fortran 77)dnl
])# AC_F77_LIBRARY_LDFLAGS


# AC_F77_DUMMY_MAIN([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# -----------------------------------------------------------
#
# Detect name of dummy main routine required by the Fortran libraries,
# (if any) and define F77_DUMMY_MAIN to this name (which should be
# used for a dummy declaration, if it is defined).  On some systems,
# linking a C program to the Fortran library does not work unless you
# supply a dummy function called something like MAIN__.
#
# Execute ACTION-IF-NOT-FOUND if no way of successfully linking a C
# program with the F77 libs is found; default to exiting with an error
# message.  Execute ACTION-IF-FOUND if a dummy routine name is needed
# and found or if it is not needed (default to defining F77_DUMMY_MAIN
# when needed).
#
# What is technically happening is that the Fortran libraries provide
# their own main() function, which usually initializes Fortran I/O and
# similar stuff, and then calls MAIN__, which is the entry point of
# your program.  Usually, a C program will override this with its own
# main() routine, but the linker sometimes complain if you don't
# provide a dummy (never-called) MAIN__ routine anyway.
#
# Of course, programs that want to allow Fortran subroutines to do
# I/O, etcetera, should call their main routine MAIN__() (or whatever)
# instead of main().  A separate autoconf test (AC_F77_MAIN) checks
# for the routine to use in this case (since the semantics of the test
# are slightly different).  To link to e.g. purely numerical
# libraries, this is normally not necessary, however, and most C/C++
# programs are reluctant to turn over so much control to Fortran.  =)
#
# The name variants we check for are (in order):
#   MAIN__ (g77, MAIN__ required on some systems; IRIX, MAIN__ optional)
#   MAIN_, __main (SunOS)
#   MAIN _MAIN __MAIN main_ main__ _main (we follow DDD and try these too)
AC_DEFUN([AC_F77_DUMMY_MAIN],
[AC_REQUIRE([AC_F77_LIBRARY_LDFLAGS])dnl
m4_define([_AC_LANG_PROGRAM_C_F77_HOOKS],
[#ifdef F77_DUMMY_MAIN
#  ifdef __cplusplus
     extern "C"
#  endif
   int F77_DUMMY_MAIN() { return 1; }
#endif
])
AC_CACHE_CHECK([for dummy main to link with Fortran 77 libraries],
               ac_cv_f77_dummy_main,
[AC_LANG_PUSH(C)dnl
 ac_f77_dm_save_LIBS=$LIBS
 LIBS="$LIBS $FLIBS"

 # First, try linking without a dummy main:
 AC_TRY_LINK([], [],
             ac_cv_f77_dummy_main=none,
             ac_cv_f77_dummy_main=unknown)

 if test $ac_cv_f77_dummy_main = unknown; then
   for ac_func in MAIN__ MAIN_ __main MAIN _MAIN __MAIN main_ main__ _main; do
     AC_TRY_LINK([@%:@define F77_DUMMY_MAIN $ac_func],
                 [], [ac_cv_f77_dummy_main=$ac_func; break])
   done
 fi
 rm -f conftest*
 LIBS=$ac_f77_dm_save_LIBS
 AC_LANG_POP(C)dnl
])
F77_DUMMY_MAIN=$ac_cv_f77_dummy_main
AS_IF([test "$F77_DUMMY_MAIN" != unknown],
      [m4_default([$1],
[if test $F77_DUMMY_MAIN != none; then
  AC_DEFINE_UNQUOTED([F77_DUMMY_MAIN], $F77_DUMMY_MAIN,
                     [Define to dummy `main' function (if any) required to
                      link to the Fortran 77 libraries.])
fi])],
      [m4_default([$2],
                [AC_MSG_ERROR([Linking to Fortran libraries from C fails.])])])
])# AC_F77_DUMMY_MAIN


# AC_F77_MAIN
# -----------
# Define F77_MAIN to name of alternate main() function for use with
# the Fortran libraries.  (Typically, the libraries may define their
# own main() to initialize I/O, etcetera, that then call your own
# routine called MAIN__ or whatever.)  See AC_F77_DUMMY_MAIN, above.
# If no such alternate name is found, just define F77_MAIN to main.
#
AC_DEFUN([AC_F77_MAIN],
[AC_REQUIRE([AC_F77_LIBRARY_LDFLAGS])dnl
AC_CACHE_CHECK([for alternate main to link with Fortran 77 libraries],
               ac_cv_f77_main,
[AC_LANG_PUSH(C)dnl
 ac_f77_m_save_LIBS=$LIBS
 LIBS="$LIBS $FLIBS"
 ac_cv_f77_main="main" # default entry point name

 for ac_func in MAIN__ MAIN_ __main MAIN _MAIN __MAIN main_ main__ _main; do
   AC_TRY_LINK([#undef F77_DUMMY_MAIN
@%:@define main $ac_func], [], [ac_cv_f77_main=$ac_func; break])
 done
 rm -f conftest*
 LIBS=$ac_f77_m_save_LIBS
 AC_LANG_POP(C)dnl
])
AC_DEFINE_UNQUOTED([F77_MAIN], $ac_cv_f77_main,
                   [Define to alternate name for `main' routine that is
                    called from a `main' in the Fortran libraries.])
])# AC_F77_MAIN


# _AC_F77_NAME_MANGLING
# ---------------------
# Test for the name mangling scheme used by the Fortran 77 compiler.
#
# Sets ac_cv_f77_mangling. The value contains three fields, separated
# by commas:
#
# lower case / upper case:
#    case translation of the Fortan 77 symbols
# underscore / no underscore:
#    whether the compiler appends "_" to symbol names
# extra underscore / no extra underscore:
#    whether the compiler appends an extra "_" to symbol names already
#    containing at least one underscore
#
AC_DEFUN([_AC_F77_NAME_MANGLING],
[AC_REQUIRE([AC_F77_LIBRARY_LDFLAGS])dnl
AC_REQUIRE([AC_F77_DUMMY_MAIN])dnl
AC_CACHE_CHECK([for Fortran 77 name-mangling scheme],
               ac_cv_f77_mangling,
[AC_LANG_PUSH(Fortran 77)dnl
AC_COMPILE_IFELSE(
[      subroutine foobar()
      return
      end
      subroutine foo_bar()
      return
      end],
[mv conftest.$ac_objext cf77_test.$ac_objext

  AC_LANG_PUSH(C)dnl

  ac_save_LIBS=$LIBS
  LIBS="cf77_test.$ac_objext $LIBS $FLIBS"

  ac_success=no
  for ac_foobar in foobar FOOBAR; do
    for ac_underscore in "" "_"; do
      ac_func="$ac_foobar$ac_underscore"
      AC_TRY_LINK_FUNC($ac_func,
         [ac_success=yes; break 2])
    done
  done

  if test "$ac_success" = "yes"; then
     case $ac_foobar in
        foobar)
           ac_case=lower
           ac_foo_bar=foo_bar
           ;;
        FOOBAR)
           ac_case=upper
           ac_foo_bar=FOO_BAR
           ;;
     esac

     ac_success_extra=no
     for ac_extra in "" "_"; do
        ac_func="$ac_foo_bar$ac_underscore$ac_extra"
        AC_TRY_LINK_FUNC($ac_func,
        [ac_success_extra=yes; break])
     done

     if test "$ac_success_extra" = "yes"; then
	ac_cv_f77_mangling="$ac_case case"
        if test -z "$ac_underscore"; then
           ac_cv_f77_mangling="$ac_cv_f77_mangling, no underscore"
	else
           ac_cv_f77_mangling="$ac_cv_f77_mangling, underscore"
        fi
        if test -z "$ac_extra"; then
           ac_cv_f77_mangling="$ac_cv_f77_mangling, no extra underscore"
	else
           ac_cv_f77_mangling="$ac_cv_f77_mangling, extra underscore"
        fi
      else
	ac_cv_f77_mangling="unknown"
      fi
  else
     ac_cv_f77_mangling="unknown"
  fi

  LIBS=$ac_save_LIBS
  AC_LANG_POP(C)dnl
  rm -f cf77_test* conftest*])
AC_LANG_POP(Fortran 77)dnl
])
])# _AC_F77_NAME_MANGLING

# The replacement is empty.
AU_DEFUN([AC_F77_NAME_MANGLING], [])


# AC_F77_WRAPPERS
# ---------------
# Defines C macros F77_FUNC(name,NAME) and F77_FUNC_(name,NAME) to
# properly mangle the names of C identifiers, and C identifiers with
# underscores, respectively, so that they match the name mangling
# scheme used by the Fortran 77 compiler.
AC_DEFUN([AC_F77_WRAPPERS],
[AC_REQUIRE([_AC_F77_NAME_MANGLING])dnl
AH_TEMPLATE([F77_FUNC],
    [Define to a macro mangling the given C identifier (in lower and upper
     case), which must not contain underscores, for linking with Fortran.])dnl
AH_TEMPLATE([F77_FUNC_],
    [As F77_FUNC, but for C identifiers containing underscores.])dnl
case $ac_cv_f77_mangling in
  "lower case, no underscore, no extra underscore")
          AC_DEFINE([F77_FUNC(name,NAME)],  [name])
          AC_DEFINE([F77_FUNC_(name,NAME)], [name]) ;;
  "lower case, no underscore, extra underscore")
          AC_DEFINE([F77_FUNC(name,NAME)],  [name])
          AC_DEFINE([F77_FUNC_(name,NAME)], [name ## _]) ;;
  "lower case, underscore, no extra underscore")
          AC_DEFINE([F77_FUNC(name,NAME)],  [name ## _])
          AC_DEFINE([F77_FUNC_(name,NAME)], [name ## _]) ;;
  "lower case, underscore, extra underscore")
          AC_DEFINE([F77_FUNC(name,NAME)],  [name ## _])
          AC_DEFINE([F77_FUNC_(name,NAME)], [name ## __]) ;;
  "upper case, no underscore, no extra underscore")
          AC_DEFINE([F77_FUNC(name,NAME)],  [NAME])
          AC_DEFINE([F77_FUNC_(name,NAME)], [NAME]) ;;
  "upper case, no underscore, extra underscore")
          AC_DEFINE([F77_FUNC(name,NAME)],  [NAME])
          AC_DEFINE([F77_FUNC_(name,NAME)], [NAME ## _]) ;;
  "upper case, underscore, no extra underscore")
          AC_DEFINE([F77_FUNC(name,NAME)],  [NAME ## _])
          AC_DEFINE([F77_FUNC_(name,NAME)], [NAME ## _]) ;;
  "upper case, underscore, extra underscore")
          AC_DEFINE([F77_FUNC(name,NAME)],  [NAME ## _])
          AC_DEFINE([F77_FUNC_(name,NAME)], [NAME ## __]) ;;
  *)
          AC_MSG_WARN([unknown Fortran 77 name-mangling scheme])
          ;;
esac
])# AC_F77_WRAPPERS


# AC_F77_FUNC(NAME, [SHELLVAR = NAME])
# ------------------------------------
# For a Fortran subroutine of given NAME, define a shell variable
# $SHELLVAR to the Fortran-77 mangled name.  If the SHELLVAR
# argument is not supplied, it defaults to NAME.
AC_DEFUN([AC_F77_FUNC],
[AC_REQUIRE([_AC_F77_NAME_MANGLING])dnl
case $ac_cv_f77_mangling in
  upper*) ac_val="m4_toupper([$1])" ;;
  lower*) ac_val="m4_tolower([$1])" ;;
  *)      ac_val="unknown" ;;
esac
case $ac_cv_f77_mangling in *," underscore"*) ac_val="$ac_val"_ ;; esac
m4_if(m4_index([$1],[_]),-1,[],
[case $ac_cv_f77_mangling in *," extra underscore"*) ac_val="$ac_val"_ ;; esac
])
m4_default([$2],[$1])="$ac_val"
])# AC_F77_FUNC
