# This file is part of Autoconf.                       -*- Autoconf -*-
# Parameterized macros.
# Copyright 1992, 1993, 1994, 1995, 1996, 1998, 1999, 2000, 2001
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
## The diversions.  ##
## ---------------- ##


# We heavily use m4's diversions both for the initializations and for
# required macros (see AC_REQUIRE), because in both cases we have to
# issue high in `configure' something which is discovered late.
#
# KILL is only used to suppress output.
#
# The layers of `configure'.  We let m4 undivert them by itself, when
# it reaches the end of `configure.ac'.
#
# - BINSH
#   AC_REQUIRE'd #! /bin/sh line
# - REVISION
#   Sent by AC_REVISION
# - NOTICE
#   copyright notice(s)
# - DEFAULTS
#   early initializations (defaults)
# - PARSE_ARGS
#   initialization code, option handling loop.
#
# - HELP_BEGIN
#   Handling `configure --help'.
# - HELP_CANON
#   Help msg for AC_CANONICAL_*
# - HELP_ENABLE
#   Help msg from AC_ARG_ENABLE.
# - HELP_WITH
#   Help msg from AC_ARG_WITH.
# - HELP_VAR
#   Help msg from AC_ARG_VAR.
# - HELP_VAR_END
#   A small paragraph on the use of the variables.
# - HELP_END
#   Tail of the handling of --help.
#
# - VERSION_BEGIN
#   Head of the handling of --version.
# - VERSION_FSF
#   FSF copyright notice for --version.
# - VERSION_USER
#   User copyright notice for --version.
# - VERSION_END
#   Tail of the handling of --version.
#
# - INIT_PREPARE
#   Tail of initialization code.
#
# - BODY
#   the tests and output code
#


# _m4_divert(DIVERSION-NAME)
# --------------------------
# Convert a diversion name into its number.  Otherwise, return
# DIVERSION-NAME which is supposed to be an actual diversion number.
# Of course it would be nicer to use m4_case here, instead of zillions
# of little macros, but it then takes twice longer to run `autoconf'!
m4_define([_m4_divert(BINSH)],           0)
m4_define([_m4_divert(REVISION)],        1)
m4_define([_m4_divert(NOTICE)],          2)
m4_define([_m4_divert(DEFAULTS)],        3)
m4_define([_m4_divert(PARSE_ARGS)],      4)

m4_define([_m4_divert(HELP_BEGIN)],     10)
m4_define([_m4_divert(HELP_CANON)],     11)
m4_define([_m4_divert(HELP_ENABLE)],    12)
m4_define([_m4_divert(HELP_WITH)],      13)
m4_define([_m4_divert(HELP_VAR)],       14)
m4_define([_m4_divert(HELP_VAR_END)],   15)
m4_define([_m4_divert(HELP_END)],       16)

m4_define([_m4_divert(VERSION_BEGIN)],  20)
m4_define([_m4_divert(VERSION_FSF)],    21)
m4_define([_m4_divert(VERSION_USER)],   22)
m4_define([_m4_divert(VERSION_END)],    23)

m4_define([_m4_divert(INIT_PREPARE)],   30)

m4_define([_m4_divert(BODY)],           40)

m4_define([_m4_divert(PREPARE)],       100)



# AC_DIVERT_PUSH(DIVERSION-NAME)
# AC_DIVERT_POP
# ------------------------------
m4_copy([m4_divert_push],[AC_DIVERT_PUSH])
m4_copy([m4_divert_pop], [AC_DIVERT_POP])


## ------------------------------- ##
## Defining macros in autoconf::.  ##
## ------------------------------- ##


# AC_DEFUN(NAME, EXPANSION)
# -------------------------
# Same as `m4_define' but equip the macro with the needed machinery
# for `AC_REQUIRE'.
#
# We don't use this macro to define some frequently called macros that
# are not involved in ordering constraints, to save m4 processing.
m4_define([AC_DEFUN],
[m4_defun([$1], [$2[]AC_PROVIDE([$1])])])


# AC_DEFUN_ONCE(NAME, EXPANSION)
# ------------------------------
# As AC_DEFUN, but issues the EXPANSION only once, and warns if used
# several times.
m4_define([AC_DEFUN_ONCE],
[m4_defun_once([$1], [$2[]AC_PROVIDE([$1])])])


# AC_OBSOLETE(THIS-MACRO-NAME, [SUGGESTION])
# ------------------------------------------
m4_define([AC_OBSOLETE],
[AC_DIAGNOSE([obsolete], [$1 is obsolete$2])])






## ----------------------------- ##
## Dependencies between macros.  ##
## ----------------------------- ##


# AC_BEFORE(THIS-MACRO-NAME, CALLED-MACRO-NAME)
# ---------------------------------------------
m4_define([AC_BEFORE],
[AC_PROVIDE_IFELSE([$2], [AC_DIAGNOSE([syntax], [$2 was called before $1])])])


# AC_REQUIRE(STRING)
# ------------------
# If STRING has never been AC_PROVIDE'd, then expand it. A macro must
# be AC_DEFUN'd if either it is AC_REQUIRE'd, or it AC_REQUIRE's.
m4_copy([m4_require], [AC_REQUIRE])


# AC_PROVIDE(MACRO-NAME)
# ----------------------
# Ideally we should just use `m4_provide($1)', but unfortunately many
# third party macros know that we use `AC_PROVIDE_$1' and they depend
# on it.
m4_define([AC_PROVIDE],
[m4_define([AC_PROVIDE_$1])m4_provide([$1])])


# AC_PROVIDE_IFELSE(MACRO-NAME, IF-PROVIDED, IF-NOT-PROVIDED)
# -----------------------------------------------------------
# If MACRO-NAME is provided do IF-PROVIDED, else IF-NOT-PROVIDED.
# The purpose of this macro is to provide the user with a means to
# check macros which are provided without letting her know how the
# information is coded.
m4_define([AC_PROVIDE_IFELSE],
[m4_ifdef([AC_PROVIDE_$1],
          [$2], [$3])])




## --------------------------------- ##
## Defining macros in autoupdate::.  ##
## --------------------------------- ##


# AU_DEFINE(NAME, GLUE-CODE, [MESSAGE])
# -------------------------------------
#
# Declare `autoupdate::NAME' to be `GLUE-CODE', with all the needed
# wrapping actions required by `autoupdate'.
# We do not define anything in `autoconf::'.
m4_define([AU_DEFINE],
[AC_DEFUN([$1], [$2])])


# AU_DEFUN(NAME, NEW-CODE, [MESSAGE])
# -----------------------------------
# Declare that the macro NAME is now obsoleted, and should be replaced
# by NEW-CODE.  Tell the user she should run autoupdate, and include
# the additional MESSAGE.
#
# Also define NAME as a macro which code is NEW-CODE.
#
# This allows to share the same code for both supporting obsoleted macros,
# and to update a configure.ac.
# See `acobsolete.m4' for a longer description.
m4_define([AU_DEFUN],
[AU_DEFINE([$1],
           [AC_DIAGNOSE([obsolete], [The macro `$1' is obsolete.
You should run autoupdate.])dnl
$2],
           [$3])dnl
])


# AU_ALIAS(OLD-NAME, NEW-NAME)
# ----------------------------
# The OLD-NAME is no longer used, just use NEW-NAME instead.  There is
# little difference with using AU_DEFUN but the fact there is little
# interest in running the test suite on both OLD-NAME and NEW-NAME.
# This macro makes it possible to distinguish such cases.
#
# Do not use `defn' since then autoupdate would replace an old macro
# call with the new macro body instead of the new macro call.
m4_define([AU_ALIAS],
[AU_DEFUN([$1], [$2($][@)])])



## ------------------------- ##
## Interface to autoheader.  ##
## ------------------------- ##


# AH_OUTPUT(KEY, TEXT)
# --------------------
# Pass TEXT to autoheader.
# This macro is `read' only via `autoconf --trace', it outputs nothing.
m4_define([AH_OUTPUT], [])


# AH_VERBATIM(KEY, TEMPLATE)
# --------------------------
# If KEY is direct (i.e., no indirection such as in KEY=$my_func which
# may occur if there is AC_CHECK_FUNCS($my_func)), issue an autoheader
# TEMPLATE associated to the KEY.  Otherwise, do nothing.  TEMPLATE is
# output as is, with no formating.
m4_define([AH_VERBATIM],
[AS_LITERAL_IF([$1],
               [AH_OUTPUT([$1], AS_ESCAPE([[$2]]))])
])


# _AH_VERBATIM_OLD(KEY, TEMPLATE)
# -------------------------------
# Same as above, but with bugward compatibility.
m4_define([_AH_VERBATIM_OLD],
[AS_LITERAL_IF([$1],
               [AH_OUTPUT([$1], _AS_QUOTE([[$2]]))])
])


# AH_TEMPLATE(KEY, DESCRIPTION)
# -----------------------------
# Issue an autoheader template for KEY, i.e., a comment composed of
# DESCRIPTION (properly wrapped), and then #undef KEY.
m4_define([AH_TEMPLATE],
[AH_VERBATIM([$1],
             m4_text_wrap([$2 */], [   ], [/* ])[
#undef $1])])


# _AH_TEMPLATE_OLD(KEY, DESCRIPTION)
# ----------------------------------
# Same as above, but with bugward compatibility.
m4_define([_AH_TEMPLATE_OLD],
[_AH_VERBATIM_OLD([$1],
                  m4_text_wrap([$2 */], [   ], [/* ])[
#undef $1])])


# AH_TOP(TEXT)
# ------------
# Output TEXT at the top of `config.h.in'.
m4_define([AH_TOP],
[m4_define([_AH_COUNTER], m4_incr(_AH_COUNTER))dnl
AH_VERBATIM([0000]_AH_COUNTER, [$1])])


# AH_BOTTOM(TEXT)
# ---------------
# Output TEXT at the bottom of `config.h.in'.
m4_define([AH_BOTTOM],
[m4_define([_AH_COUNTER], m4_incr(_AH_COUNTER))dnl
AH_VERBATIM([zzzz]_AH_COUNTER, [$1])])

# Initialize.
m4_define([_AH_COUNTER], [0])




## ----------------------------- ##
## Implementing Autoconf loops.  ##
## ----------------------------- ##


# AC_FOREACH(VARIABLE, LIST, EXPRESSION)
# --------------------------------------
#
# Compute EXPRESSION assigning to VARIABLE each value of the LIST.
# LIST is a /bin/sh list, i.e., it has the form ` item_1 item_2
# ... item_n ': white spaces are separators, and leading and trailing
# spaces are meaningless.
#
# This macro is robust to active symbols:
#    AC_FOREACH([Var], [ active
#    b	act\
#    ive  ], [-Var-])end
#    => -active--b--active-end
m4_define([AC_FOREACH],
[m4_foreach([$1], m4_split(m4_normalize([$2])), [$3])])




## ----------------------------------- ##
## Helping macros to display strings.  ##
## ----------------------------------- ##


# AC_HELP_STRING(LHS, RHS, [COLUMN])
# ----------------------------------
#
# Format an Autoconf macro's help string so that it looks pretty when
# the user executes "configure --help".  This macro takes three
# arguments, a "left hand side" (LHS), a "right hand side" (RHS), and
# the COLUMN which is a string of white spaces which leads to the
# the RHS column (default: 26 white spaces).
#
# The resulting string is suitable for use in other macros that require
# a help string (e.g. AC_ARG_WITH).
#
# Here is the sample string from the Autoconf manual (Node: External
# Software) which shows the proper spacing for help strings.
#
#    --with-readline         support fancy command line editing
#  ^ ^                       ^
#  | |                       |
#  | column 2                column 26
#  |
#  column 0
#
# A help string is made up of a "left hand side" (LHS) and a "right
# hand side" (RHS).  In the example above, the LHS is
# "--with-readline", while the RHS is "support fancy command line
# editing".
#
# If the LHS extends past column 24, then the LHS is terminated with a
# newline so that the RHS is on a line of its own beginning in column
# 26.
#
# Therefore, if the LHS were instead "--with-readline-blah-blah-blah",
# then the AC_HELP_STRING macro would expand into:
#
#
#    --with-readline-blah-blah-blah
#  ^ ^                       support fancy command line editing
#  | |                       ^
#  | column 2                |
#  column 0                  column 26
#
m4_define([AC_HELP_STRING],
[m4_pushdef([AC_Prefix], m4_default([$3], [                          ]))dnl
m4_pushdef([AC_Prefix_Format],
           [  %-]m4_eval(m4_len(AC_Prefix) - 3)[s ])dnl [  %-23s ]
m4_text_wrap([$2], AC_Prefix, m4_format(AC_Prefix_Format, [$1]))dnl
m4_popdef([AC_Prefix_Format])dnl
m4_popdef([AC_Prefix])dnl
])




## ---------------------------------------------- ##
## Information on the package being Autoconf'ed.  ##
## ---------------------------------------------- ##


# It is suggested that the macros in this section appear before
# AC_INIT in `configure.ac'.  Nevertheless, this is just stylistic,
# and from the implementation point of, AC_INIT *must* be expanded
# beforehand: it puts data in diversions which must appear before the
# data provided by the macros of this section.

# The solution is to require AC_INIT in each of these macros.  AC_INIT
# has the needed magic so that it can't be expanded twice.



# _AC_INIT_PACKAGE(PACKAGE-NAME, VERSION,
#                  [BUG-REPORT],
#                  [TAR-NAME = unGNU'd lower case PACKAGE-NAME])
# --------------------------------------------------------------
m4_define([_AC_INIT_PACKAGE],
[m4_define([AC_PACKAGE_NAME],     [$1])
m4_define([AC_PACKAGE_TARNAME],
          m4_tolower(m4_patsubst([[[$1]]], [GNU ])))
m4_define([AC_PACKAGE_VERSION],   [$2])
m4_define([AC_PACKAGE_STRING],    [$1 $2])
m4_define([AC_PACKAGE_BUGREPORT], [$3])
])


# AC_COPYRIGHT(TEXT, [VERSION-DIVERSION = VERSION_USER])
# ------------------------------------------------------
# Append Copyright information in the top of `configure'.  TEXT is
# evaluated once, hence TEXT can use macros.  Note that we do not
# prepend `# ' but `@%:@ ', since m4 does not evaluate the comments.
# Had we used `# ', the Copyright sent in the beginning of `configure'
# would have not been evaluated.  Another solution, a bit fragile,
# would have be to use m4_quote to force an evaluation:
#
#     m4_patsubst(m4_quote($1), [^], [# ])
m4_define([AC_COPYRIGHT],
[m4_divert_text([NOTICE],
[m4_patsubst([
$1], [^], [@%:@ ])])dnl
m4_divert_text(m4_default([$2], [VERSION_USER]),
[
$1])dnl
])# AC_COPYRIGHT


# AC_REVISION(REVISION-INFO)
# --------------------------
# The second quote in the translit is just to cope with font-lock-mode
# which sees the opening of a string.
m4_define([AC_REVISION],
[m4_divert_text([REVISION],
                [@%:@ From __file__ m4_translit([$1], [$""]).])dnl
])




## ---------------------------------------- ##
## Requirements over the Autoconf version.  ##
## ---------------------------------------- ##


# AU::AC_PREREQ(VERSION)
# ----------------------
# Update this `AC_PREREQ' statement to require the current version of
# Autoconf.  But fail if ever this autoupdate is too old.
#
# Note that `m4_defn([AC_ACVERSION])' below are expanded before calling
# `AU_DEFUN', i.e., it is hard coded.  Otherwise it would be quite
# complex for autoupdate to import the value of `AC_ACVERSION'.  We
# could `AU_DEFUN' `AC_ACVERSION', but this would replace all its
# occurrences with the current version of Autoconf, which is certainly
# not what mean the user.
AU_DEFUN([AC_PREREQ],
[m4_if(m4_version_compare(]m4_defn([AC_ACVERSION])[, [$1]), -1,
    [m4_fatal([Autoconf version $1 or higher is required for this script])])dnl
[AC_PREREQ(]]m4_defn([AC_ACVERSION])[[)]])


# AC_PREREQ(VERSION)
# ------------------
# Complain and exit if the Autoconf version is less than VERSION.
m4_define([AC_PREREQ],
[m4_if(m4_version_compare(m4_defn([AC_ACVERSION]), [$1]), -1,
     [AC_FATAL([Autoconf version $1 or higher is required for this script])])])






## ---------------- ##
## Initialization.  ##
## ---------------- ##


# All the following macros are used by AC_INIT.  Ideally, they should
# be presented in the order in which they are output.  Please, help us
# sorting it, or at least, don't augment the entropy.


# _AC_INIT_NOTICE
# ---------------
m4_define([_AC_INIT_NOTICE],
[m4_divert_text([NOTICE],
[@%:@ Guess values for system-dependent variables and create Makefiles.
@%:@ Generated by Autoconf AC_ACVERSION[]dnl
m4_ifset([AC_PACKAGE_STRING], [ for AC_PACKAGE_STRING]).])

m4_ifset([AC_PACKAGE_BUGREPORT],
         [m4_divert_text([NOTICE],
                         [@%:@
@%:@ Report bugs to <AC_PACKAGE_BUGREPORT>.])])
])


# _AC_INIT_COPYRIGHT
# ------------------
# We dump to VERSION_FSF to make sure we are inserted before the
# user copyrights, and after the setup of the --version handling.
m4_define([_AC_INIT_COPYRIGHT],
[AC_COPYRIGHT(
[Copyright 1992, 1993, 1994, 1995, 1996, 1998, 1999, 2000, 2001
Free Software Foundation, Inc.
This configure script is free software; the Free Software Foundation
gives unlimited permission to copy, distribute and modify it.],
              [VERSION_FSF])dnl
])


# File Descriptors
# ----------------
# Set up the file descriptors used by `configure'.
# File descriptor usage:
# 0 standard input
# 1 file creation
# 2 errors and warnings
# AS_MESSAGE_LOG_FD compiler messages saved in config.log
# AS_MESSAGE_FD checking for... messages and results

m4_define([AS_MESSAGE_FD], 6)
# That's how they used to be named.
AU_ALIAS([AC_FD_CC],  [AS_MESSAGE_LOG_FD])
AU_ALIAS([AC_FD_MSG], [AS_MESSAGE_FD])


# _AC_INIT_DEFAULTS
# -----------------
# Values which defaults can be set from `configure.ac'.
# `/bin/machine' is used in `glibcbug'.  The others are used in config.*
m4_define([_AC_INIT_DEFAULTS],
[m4_divert_push([DEFAULTS])dnl

AS_SHELL_SANITIZE

# Name of the host.
# hostname on some systems (SVR3.2, Linux) returns a bogus exit status,
# so uname gets run too.
ac_hostname=`(hostname || uname -n) 2>/dev/null | sed 1q`

exec AS_MESSAGE_FD>&1

#
# Initializations.
#
ac_default_prefix=/usr/local
cross_compiling=no
subdirs=
MFLAGS= MAKEFLAGS=
AC_SUBST(SHELL, ${CONFIG_SHELL-/bin/sh})dnl

# Maximum number of lines to put in a shell here document.
# This variable seems obsolete.  It should probably be removed, and
# only ac_max_sed_lines should be used.
: ${ac_max_here_lines=38}

m4_divert_pop([DEFAULTS])dnl
])# _AC_INIT_DEFAULTS


# AC_PREFIX_DEFAULT(PREFIX)
# -------------------------
AC_DEFUN([AC_PREFIX_DEFAULT],
[m4_divert_text([DEFAULTS], [ac_default_prefix=$1])])


# AC_CONFIG_SRCDIR([UNIQUE-FILE-IN-SOURCE-DIR])
# ---------------------------------------------
# UNIQUE-FILE-IN-SOURCE-DIR is a filename unique to this package,
# relative to the directory that configure is in, which we can look
# for to find out if srcdir is correct.
AC_DEFUN([AC_CONFIG_SRCDIR],
[m4_divert_text([DEFAULTS], [ac_unique_file="$1"])])


# _AC_INIT_SRCDIR
# ---------------
# Compute `srcdir' based on `$ac_unique_file'.
m4_define([_AC_INIT_SRCDIR],
[m4_divert_push([PARSE_ARGS])dnl

# Find the source files, if location was not specified.
if test -z "$srcdir"; then
  ac_srcdir_defaulted=yes
  # Try the directory containing this script, then its parent.
  ac_prog=$[0]
dnl FIXME: should use AS_DIRNAME here once it is made DOS-friendly.
  ac_confdir=`echo "$ac_prog" | sed 's%[[\\/][^\\/][^\\/]]*$%%'`
  test "x$ac_confdir" = "x$ac_prog" && ac_confdir=.
  srcdir=$ac_confdir
  if test ! -r $srcdir/$ac_unique_file; then
    srcdir=..
  fi
else
  ac_srcdir_defaulted=no
fi
if test ! -r $srcdir/$ac_unique_file; then
  if test "$ac_srcdir_defaulted" = yes; then
    AC_MSG_ERROR([cannot find sources in $ac_confdir or ..])
  else
    AC_MSG_ERROR([cannot find sources in $srcdir])
  fi
fi
dnl Double slashes in pathnames in object file debugging info
dnl mess up M-x gdb in Emacs.
srcdir=`echo "$srcdir" | sed 's%\([[^\\/]]\)[[\\/]]*$%\1%'`
m4_divert_pop([PARSE_ARGS])dnl
])# _AC_INIT_SRCDIR


# _AC_INIT_PARSE_ARGS
# -------------------
m4_define([_AC_INIT_PARSE_ARGS],
[m4_divert_push([PARSE_ARGS])dnl

# Initialize some variables set by options.
ac_init_help=
ac_init_version=false
# The variables have the same names as the options, with
# dashes changed to underlines.
cache_file=/dev/null
AC_SUBST(exec_prefix, NONE)dnl
no_create=
no_recursion=
AC_SUBST(prefix, NONE)dnl
program_prefix=NONE
program_suffix=NONE
AC_SUBST(program_transform_name, [s,x,x,])dnl
silent=
site=
srcdir=
verbose=
x_includes=NONE
x_libraries=NONE

# Installation directory options.
# These are left unexpanded so users can "make install exec_prefix=/foo"
# and all the variables that are supposed to be based on exec_prefix
# by default will actually change.
# Use braces instead of parens because sh, perl, etc. also accept them.
AC_SUBST([bindir],         ['${exec_prefix}/bin'])dnl
AC_SUBST([sbindir],        ['${exec_prefix}/sbin'])dnl
AC_SUBST([libexecdir],     ['${exec_prefix}/libexec'])dnl
AC_SUBST([datadir],        ['${prefix}/share'])dnl
AC_SUBST([sysconfdir],     ['${prefix}/etc'])dnl
AC_SUBST([sharedstatedir], ['${prefix}/com'])dnl
AC_SUBST([localstatedir],  ['${prefix}/var'])dnl
AC_SUBST([libdir],         ['${exec_prefix}/lib'])dnl
AC_SUBST([includedir],     ['${prefix}/include'])dnl
AC_SUBST([oldincludedir],  ['/usr/include'])dnl
AC_SUBST([infodir],        ['${prefix}/info'])dnl
AC_SUBST([mandir],         ['${prefix}/man'])dnl

# Identity of this package.
AC_SUBST([PACKAGE_NAME],
         [m4_ifdef([AC_PACKAGE_NAME],      ['AC_PACKAGE_NAME'])])dnl
AC_SUBST([PACKAGE_TARNAME],
         [m4_ifdef([AC_PACKAGE_TARNAME],   ['AC_PACKAGE_TARNAME'])])dnl
AC_SUBST([PACKAGE_VERSION],
         [m4_ifdef([AC_PACKAGE_VERSION],   ['AC_PACKAGE_VERSION'])])dnl
AC_SUBST([PACKAGE_STRING],
         [m4_ifdef([AC_PACKAGE_STRING],    ['AC_PACKAGE_STRING'])])dnl
AC_SUBST([PACKAGE_BUGREPORT],
         [m4_ifdef([AC_PACKAGE_BUGREPORT], ['AC_PACKAGE_BUGREPORT'])])dnl

ac_prev=
for ac_option
do
  # If the previous option needs an argument, assign it.
  if test -n "$ac_prev"; then
    eval "$ac_prev=\$ac_option"
    ac_prev=
    continue
  fi

  ac_optarg=`expr "x$ac_option" : 'x[[^=]]*=\(.*\)'`

  # Accept the important Cygnus configure options, so we can diagnose typos.

  case $ac_option in

  -bindir | --bindir | --bindi | --bind | --bin | --bi)
    ac_prev=bindir ;;
  -bindir=* | --bindir=* | --bindi=* | --bind=* | --bin=* | --bi=*)
    bindir=$ac_optarg ;;

  -build | --build | --buil | --bui | --bu)
    ac_prev=build_alias ;;
  -build=* | --build=* | --buil=* | --bui=* | --bu=*)
    build_alias=$ac_optarg ;;

  -cache-file | --cache-file | --cache-fil | --cache-fi \
  | --cache-f | --cache- | --cache | --cach | --cac | --ca | --c)
    ac_prev=cache_file ;;
  -cache-file=* | --cache-file=* | --cache-fil=* | --cache-fi=* \
  | --cache-f=* | --cache-=* | --cache=* | --cach=* | --cac=* | --ca=* | --c=*)
    cache_file=$ac_optarg ;;

  --config-cache | -C)
    cache_file=config.cache ;;

  -datadir | --datadir | --datadi | --datad | --data | --dat | --da)
    ac_prev=datadir ;;
  -datadir=* | --datadir=* | --datadi=* | --datad=* | --data=* | --dat=* \
  | --da=*)
    datadir=$ac_optarg ;;

  -disable-* | --disable-*)
    ac_feature=`expr "x$ac_option" : 'x-*disable-\(.*\)'`
    # Reject names that are not valid shell variable names.
    expr "x$ac_feature" : "[.*[^-_$as_cr_alnum]]" >/dev/null &&
      AC_MSG_ERROR([invalid feature name: $ac_feature])
    ac_feature=`echo $ac_feature | sed 's/-/_/g'`
    eval "enable_$ac_feature=no" ;;

  -enable-* | --enable-*)
    ac_feature=`expr "x$ac_option" : 'x-*enable-\([[^=]]*\)'`
    # Reject names that are not valid shell variable names.
    expr "x$ac_feature" : "[.*[^-_$as_cr_alnum]]" >/dev/null &&
      AC_MSG_ERROR([invalid feature name: $ac_feature])
    ac_feature=`echo $ac_feature | sed 's/-/_/g'`
    case $ac_option in
      *=*) ac_optarg=`echo "$ac_optarg" | sed "s/'/'\\\\\\\\''/g"`;;
      *) ac_optarg=yes ;;
    esac
    eval "enable_$ac_feature='$ac_optarg'" ;;

  -exec-prefix | --exec_prefix | --exec-prefix | --exec-prefi \
  | --exec-pref | --exec-pre | --exec-pr | --exec-p | --exec- \
  | --exec | --exe | --ex)
    ac_prev=exec_prefix ;;
  -exec-prefix=* | --exec_prefix=* | --exec-prefix=* | --exec-prefi=* \
  | --exec-pref=* | --exec-pre=* | --exec-pr=* | --exec-p=* | --exec-=* \
  | --exec=* | --exe=* | --ex=*)
    exec_prefix=$ac_optarg ;;

  -gas | --gas | --ga | --g)
    # Obsolete; use --with-gas.
    with_gas=yes ;;

  -help | --help | --hel | --he | -h)
    ac_init_help=long ;;
  -help=r* | --help=r* | --hel=r* | --he=r* | -hr*)
    ac_init_help=recursive ;;
  -help=s* | --help=s* | --hel=s* | --he=s* | -hs*)
    ac_init_help=short ;;

  -host | --host | --hos | --ho)
    ac_prev=host_alias ;;
  -host=* | --host=* | --hos=* | --ho=*)
    host_alias=$ac_optarg ;;

  -includedir | --includedir | --includedi | --included | --include \
  | --includ | --inclu | --incl | --inc)
    ac_prev=includedir ;;
  -includedir=* | --includedir=* | --includedi=* | --included=* | --include=* \
  | --includ=* | --inclu=* | --incl=* | --inc=*)
    includedir=$ac_optarg ;;

  -infodir | --infodir | --infodi | --infod | --info | --inf)
    ac_prev=infodir ;;
  -infodir=* | --infodir=* | --infodi=* | --infod=* | --info=* | --inf=*)
    infodir=$ac_optarg ;;

  -libdir | --libdir | --libdi | --libd)
    ac_prev=libdir ;;
  -libdir=* | --libdir=* | --libdi=* | --libd=*)
    libdir=$ac_optarg ;;

  -libexecdir | --libexecdir | --libexecdi | --libexecd | --libexec \
  | --libexe | --libex | --libe)
    ac_prev=libexecdir ;;
  -libexecdir=* | --libexecdir=* | --libexecdi=* | --libexecd=* | --libexec=* \
  | --libexe=* | --libex=* | --libe=*)
    libexecdir=$ac_optarg ;;

  -localstatedir | --localstatedir | --localstatedi | --localstated \
  | --localstate | --localstat | --localsta | --localst \
  | --locals | --local | --loca | --loc | --lo)
    ac_prev=localstatedir ;;
  -localstatedir=* | --localstatedir=* | --localstatedi=* | --localstated=* \
  | --localstate=* | --localstat=* | --localsta=* | --localst=* \
  | --locals=* | --local=* | --loca=* | --loc=* | --lo=*)
    localstatedir=$ac_optarg ;;

  -mandir | --mandir | --mandi | --mand | --man | --ma | --m)
    ac_prev=mandir ;;
  -mandir=* | --mandir=* | --mandi=* | --mand=* | --man=* | --ma=* | --m=*)
    mandir=$ac_optarg ;;

  -nfp | --nfp | --nf)
    # Obsolete; use --without-fp.
    with_fp=no ;;

  -no-create | --no-create | --no-creat | --no-crea | --no-cre \
  | --no-cr | --no-c)
    no_create=yes ;;

  -no-recursion | --no-recursion | --no-recursio | --no-recursi \
  | --no-recurs | --no-recur | --no-recu | --no-rec | --no-re | --no-r)
    no_recursion=yes ;;

  -oldincludedir | --oldincludedir | --oldincludedi | --oldincluded \
  | --oldinclude | --oldinclud | --oldinclu | --oldincl | --oldinc \
  | --oldin | --oldi | --old | --ol | --o)
    ac_prev=oldincludedir ;;
  -oldincludedir=* | --oldincludedir=* | --oldincludedi=* | --oldincluded=* \
  | --oldinclude=* | --oldinclud=* | --oldinclu=* | --oldincl=* | --oldinc=* \
  | --oldin=* | --oldi=* | --old=* | --ol=* | --o=*)
    oldincludedir=$ac_optarg ;;

  -prefix | --prefix | --prefi | --pref | --pre | --pr | --p)
    ac_prev=prefix ;;
  -prefix=* | --prefix=* | --prefi=* | --pref=* | --pre=* | --pr=* | --p=*)
    prefix=$ac_optarg ;;

  -program-prefix | --program-prefix | --program-prefi | --program-pref \
  | --program-pre | --program-pr | --program-p)
    ac_prev=program_prefix ;;
  -program-prefix=* | --program-prefix=* | --program-prefi=* \
  | --program-pref=* | --program-pre=* | --program-pr=* | --program-p=*)
    program_prefix=$ac_optarg ;;

  -program-suffix | --program-suffix | --program-suffi | --program-suff \
  | --program-suf | --program-su | --program-s)
    ac_prev=program_suffix ;;
  -program-suffix=* | --program-suffix=* | --program-suffi=* \
  | --program-suff=* | --program-suf=* | --program-su=* | --program-s=*)
    program_suffix=$ac_optarg ;;

  -program-transform-name | --program-transform-name \
  | --program-transform-nam | --program-transform-na \
  | --program-transform-n | --program-transform- \
  | --program-transform | --program-transfor \
  | --program-transfo | --program-transf \
  | --program-trans | --program-tran \
  | --progr-tra | --program-tr | --program-t)
    ac_prev=program_transform_name ;;
  -program-transform-name=* | --program-transform-name=* \
  | --program-transform-nam=* | --program-transform-na=* \
  | --program-transform-n=* | --program-transform-=* \
  | --program-transform=* | --program-transfor=* \
  | --program-transfo=* | --program-transf=* \
  | --program-trans=* | --program-tran=* \
  | --progr-tra=* | --program-tr=* | --program-t=*)
    program_transform_name=$ac_optarg ;;

  -q | -quiet | --quiet | --quie | --qui | --qu | --q \
  | -silent | --silent | --silen | --sile | --sil)
    silent=yes ;;

  -sbindir | --sbindir | --sbindi | --sbind | --sbin | --sbi | --sb)
    ac_prev=sbindir ;;
  -sbindir=* | --sbindir=* | --sbindi=* | --sbind=* | --sbin=* \
  | --sbi=* | --sb=*)
    sbindir=$ac_optarg ;;

  -sharedstatedir | --sharedstatedir | --sharedstatedi \
  | --sharedstated | --sharedstate | --sharedstat | --sharedsta \
  | --sharedst | --shareds | --shared | --share | --shar \
  | --sha | --sh)
    ac_prev=sharedstatedir ;;
  -sharedstatedir=* | --sharedstatedir=* | --sharedstatedi=* \
  | --sharedstated=* | --sharedstate=* | --sharedstat=* | --sharedsta=* \
  | --sharedst=* | --shareds=* | --shared=* | --share=* | --shar=* \
  | --sha=* | --sh=*)
    sharedstatedir=$ac_optarg ;;

  -site | --site | --sit)
    ac_prev=site ;;
  -site=* | --site=* | --sit=*)
    site=$ac_optarg ;;

  -srcdir | --srcdir | --srcdi | --srcd | --src | --sr)
    ac_prev=srcdir ;;
  -srcdir=* | --srcdir=* | --srcdi=* | --srcd=* | --src=* | --sr=*)
    srcdir=$ac_optarg ;;

  -sysconfdir | --sysconfdir | --sysconfdi | --sysconfd | --sysconf \
  | --syscon | --sysco | --sysc | --sys | --sy)
    ac_prev=sysconfdir ;;
  -sysconfdir=* | --sysconfdir=* | --sysconfdi=* | --sysconfd=* | --sysconf=* \
  | --syscon=* | --sysco=* | --sysc=* | --sys=* | --sy=*)
    sysconfdir=$ac_optarg ;;

  -target | --target | --targe | --targ | --tar | --ta | --t)
    ac_prev=target_alias ;;
  -target=* | --target=* | --targe=* | --targ=* | --tar=* | --ta=* | --t=*)
    target_alias=$ac_optarg ;;

  -v | -verbose | --verbose | --verbos | --verbo | --verb)
    verbose=yes ;;

  -version | --version | --versio | --versi | --vers | -V)
    ac_init_version=: ;;

  -with-* | --with-*)
    ac_package=`expr "x$ac_option" : 'x-*with-\([[^=]]*\)'`
    # Reject names that are not valid shell variable names.
    expr "x$ac_package" : "[.*[^-_$as_cr_alnum]]" >/dev/null &&
      AC_MSG_ERROR([invalid package name: $ac_package])
    ac_package=`echo $ac_package| sed 's/-/_/g'`
    case $ac_option in
      *=*) ac_optarg=`echo "$ac_optarg" | sed "s/'/'\\\\\\\\''/g"`;;
      *) ac_optarg=yes ;;
    esac
    eval "with_$ac_package='$ac_optarg'" ;;

  -without-* | --without-*)
    ac_package=`expr "x$ac_option" : 'x-*without-\(.*\)'`
    # Reject names that are not valid shell variable names.
    expr "x$ac_package" : "[.*[^-_$as_cr_alnum]]" >/dev/null &&
      AC_MSG_ERROR([invalid package name: $ac_package])
    ac_package=`echo $ac_package | sed 's/-/_/g'`
    eval "with_$ac_package=no" ;;

  --x)
    # Obsolete; use --with-x.
    with_x=yes ;;

  -x-includes | --x-includes | --x-include | --x-includ | --x-inclu \
  | --x-incl | --x-inc | --x-in | --x-i)
    ac_prev=x_includes ;;
  -x-includes=* | --x-includes=* | --x-include=* | --x-includ=* | --x-inclu=* \
  | --x-incl=* | --x-inc=* | --x-in=* | --x-i=*)
    x_includes=$ac_optarg ;;

  -x-libraries | --x-libraries | --x-librarie | --x-librari \
  | --x-librar | --x-libra | --x-libr | --x-lib | --x-li | --x-l)
    ac_prev=x_libraries ;;
  -x-libraries=* | --x-libraries=* | --x-librarie=* | --x-librari=* \
  | --x-librar=* | --x-libra=* | --x-libr=* | --x-lib=* | --x-li=* | --x-l=*)
    x_libraries=$ac_optarg ;;

  -*) AC_MSG_ERROR([unrecognized option: $ac_option
Try `$[0] --help' for more information.])
    ;;

  *=*)
    ac_envvar=`expr "x$ac_option" : 'x\([[^=]]*\)='`
    # Reject names that are not valid shell variable names.
    expr "x$ac_envvar" : "[.*[^_$as_cr_alnum]]" >/dev/null &&
      AC_MSG_ERROR([invalid variable name: $ac_envvar])
    ac_optarg=`echo "$ac_optarg" | sed "s/'/'\\\\\\\\''/g"`
    eval "$ac_envvar='$ac_optarg'"
    export $ac_envvar ;;

  *)
    # FIXME: should be removed in autoconf 3.0.
    AC_MSG_WARN([you should use --build, --host, --target])
    expr "x$ac_option" : "[.*[^-._$as_cr_alnum]]" >/dev/null &&
      AC_MSG_WARN([invalid host type: $ac_option])
    : ${build_alias=$ac_option} ${host_alias=$ac_option} ${target_alias=$ac_option}
    ;;

  esac
done

if test -n "$ac_prev"; then
  ac_option=--`echo $ac_prev | sed 's/_/-/g'`
  AC_MSG_ERROR([missing argument to $ac_option])
fi

# Be sure to have absolute paths.
for ac_var in exec_prefix prefix
do
  eval ac_val=$`echo $ac_var`
  case $ac_val in
    [[\\/$]]* | ?:[[\\/]]* | NONE | '' ) ;;
    *)  AC_MSG_ERROR([expected an absolute path for --$ac_var: $ac_val]);;
  esac
done

# Be sure to have absolute paths.
for ac_var in bindir sbindir libexecdir datadir sysconfdir sharedstatedir \
              localstatedir libdir includedir oldincludedir infodir mandir
do
  eval ac_val=$`echo $ac_var`
  case $ac_val in
    [[\\/$]]* | ?:[[\\/]]* ) ;;
    *)  AC_MSG_ERROR([expected an absolute path for --$ac_var: $ac_val]);;
  esac
done

# There might be people who depend on the old broken behavior: `$host'
# used to hold the argument of --host etc.
build=$build_alias
host=$host_alias
target=$target_alias

# FIXME: should be removed in autoconf 3.0.
if test "x$host_alias" != x; then
  if test "x$build_alias" = x; then
    cross_compiling=maybe
    AC_MSG_WARN([If you wanted to set the --build type, don't use --host.
    If a cross compiler is detected then cross compile mode will be used.])
  elif test "x$build_alias" != "x$host_alias"; then
    cross_compiling=yes
  fi
fi

ac_tool_prefix=
test -n "$host_alias" && ac_tool_prefix=$host_alias-

test "$silent" = yes && exec AS_MESSAGE_FD>/dev/null

m4_divert_pop([PARSE_ARGS])dnl
])# _AC_INIT_PARSE_ARGS


# _AC_INIT_HELP
# -------------
# Handle the `configure --help' message.
m4_define([_AC_INIT_HELP],
[m4_divert_push([HELP_BEGIN])dnl

#
# Report the --help message.
#
if test "$ac_init_help" = "long"; then
  # Omit some internal or obsolete options to make the list less imposing.
  # This message is too long to be a string in the A/UX 3.1 sh.
  cat <<EOF
\`configure' configures m4_ifset([AC_PACKAGE_STRING],
                        [AC_PACKAGE_STRING],
                        [this package]) to adapt to many kinds of systems.

Usage: $[0] [[OPTION]]... [[VAR=VALUE]]...

[To assign environment variables (e.g., CC, CFLAGS...), specify them as
VAR=VALUE.  See below for descriptions of some of the useful variables.

Defaults for the options are specified in brackets.

Configuration:
  -h, --help              display this help and exit
      --help=short        display options specific to this package
      --help=recursive    display the short help of all the included packages
  -V, --version           display version information and exit
  -q, --quiet, --silent   do not print \`checking...' messages
      --cache-file=FILE   cache test results in FILE [disabled]
  -C, --config-cache      alias for \`--cache-file=config.cache'
  -n, --no-create         do not create output files
      --srcdir=DIR        find the sources in DIR [configure dir or \`..']

EOF

  cat <<EOF
Installation directories:
  --prefix=PREFIX         install architecture-independent files in PREFIX
                          [$ac_default_prefix]
  --exec-prefix=EPREFIX   install architecture-dependent files in EPREFIX
                          [PREFIX]

By default, \`make install' will install all the files in
\`$ac_default_prefix/bin', \`$ac_default_prefix/lib' etc.  You can specify
an installation prefix other than \`$ac_default_prefix' using \`--prefix',
for instance \`--prefix=\$HOME'.

For better control, use the options below.

Fine tuning of the installation directories:
  --bindir=DIR           user executables [EPREFIX/bin]
  --sbindir=DIR          system admin executables [EPREFIX/sbin]
  --libexecdir=DIR       program executables [EPREFIX/libexec]
  --datadir=DIR          read-only architecture-independent data [PREFIX/share]
  --sysconfdir=DIR       read-only single-machine data [PREFIX/etc]
  --sharedstatedir=DIR   modifiable architecture-independent data [PREFIX/com]
  --localstatedir=DIR    modifiable single-machine data [PREFIX/var]
  --libdir=DIR           object code libraries [EPREFIX/lib]
  --includedir=DIR       C header files [PREFIX/include]
  --oldincludedir=DIR    C header files for non-gcc [/usr/include]
  --infodir=DIR          info documentation [PREFIX/info]
  --mandir=DIR           man documentation [PREFIX/man]
EOF

  cat <<\EOF]
m4_divert_pop([HELP_BEGIN])dnl
dnl The order of the diversions here is
dnl - HELP_BEGIN
dnl   which may be prolongated by extra generic options such as with X or
dnl   AC_ARG_PROGRAM.  Displayed only in long --help.
dnl
dnl - HELP_CANON
dnl   Support for cross compilation (--build, --host and --target).
dnl   Display only in long --help.
dnl
dnl - HELP_ENABLE
dnl   which starts with the trailer of the HELP_BEGIN, HELP_CANON section,
dnl   then implements the header of the non generic options.
dnl
dnl - HELP_WITH
dnl
dnl - HELP_VAR
dnl
dnl - HELP_VAR_END
dnl
dnl - HELP_END
dnl   initialized below, in which we dump the trailer (handling of the
dnl   recursion for instance).
m4_divert_push([HELP_ENABLE])dnl
EOF
fi

if test -n "$ac_init_help"; then
m4_ifset([AC_PACKAGE_STRING],
[  case $ac_init_help in
     short | recursive ) echo "Configuration of AC_PACKAGE_STRING:";;
   esac])
  cat <<\EOF
m4_divert_pop([HELP_ENABLE])dnl
m4_divert_push([HELP_END])dnl
m4_ifset([AC_PACKAGE_BUGREPORT], [
Report bugs to <AC_PACKAGE_BUGREPORT>.])
EOF
fi

if test "$ac_init_help" = "recursive"; then
  # If there are subdirs, report their specific --help.
  ac_popdir=`pwd`
  for ac_subdir in : $ac_subdirs_all; do test "x$ac_subdir" = x: && continue
    cd $ac_subdir
    # A "../" for each directory in /$ac_subdir.
    ac_dots=`echo $ac_subdir |
             sed 's,^\./,,;s,[[^/]]$,&/,;s,[[^/]]*/,../,g'`

    case $srcdir in
    .) # No --srcdir option.  We are building in place.
      ac_sub_srcdir=$srcdir ;;
    [[\\/]]* | ?:[[\\/]]* ) # Absolute path.
      ac_sub_srcdir=$srcdir/$ac_subdir ;;
    *) # Relative path.
      ac_sub_srcdir=$ac_dots$srcdir/$ac_subdir ;;
    esac

    # Check for guested configure; otherwise get Cygnus style configure.
    if test -f $ac_sub_srcdir/configure.gnu; then
      echo
      $SHELL $ac_sub_srcdir/configure.gnu  --help=recursive
    elif test -f $ac_sub_srcdir/configure; then
      echo
      $SHELL $ac_sub_srcdir/configure  --help=recursive
    elif test -f $ac_sub_srcdir/configure.ac ||
           test -f $ac_sub_srcdir/configure.in; then
      echo
      $ac_configure --help
    else
      AC_MSG_WARN([no configuration information is in $ac_subdir])
    fi
    cd $ac_popdir
  done
fi

test -n "$ac_init_help" && exit 0
m4_divert_pop([HELP_END])dnl
])# _AC_INIT_HELP


# _AC_INIT_VERSION
# ----------------
# Handle the `configure --version' message.
m4_define([_AC_INIT_VERSION],
[m4_divert_text([VERSION_BEGIN],
[if $ac_init_version; then
  cat <<\EOF])dnl
m4_ifset([AC_PACKAGE_STRING],
         [m4_divert_text([VERSION_BEGIN],
                         [dnl
m4_ifset([AC_PACKAGE_NAME], [AC_PACKAGE_NAME ])configure[]dnl
m4_ifset([AC_PACKAGE_VERSION], [ AC_PACKAGE_VERSION])
generated by GNU Autoconf AC_ACVERSION])])
m4_divert_text([VERSION_END],
[EOF
  exit 0
fi])dnl
])# _AC_INIT_VERSION


# _AC_INIT_CONFIG_LOG
# -------------------
# Initialize the config.log file descriptor and write header to it.
m4_define([_AC_INIT_CONFIG_LOG],
[m4_divert_text([INIT_PREPARE],
[m4_define([AS_MESSAGE_LOG_FD], 5)dnl
exec AS_MESSAGE_LOG_FD>config.log
cat >&AS_MESSAGE_LOG_FD <<EOF
This file contains any messages produced by compilers while
running configure, to aid debugging if configure makes a mistake.

It was created by m4_ifset([AC_PACKAGE_NAME],
                           [AC_PACKAGE_NAME ])dnl
$as_me[]m4_ifset([AC_PACKAGE_VERSION],
                 [ AC_PACKAGE_VERSION]), which was
generated by GNU Autoconf AC_ACVERSION.  Invocation command line was

  $ $[0] $[@]

EOF
AS_UNAME >&AS_MESSAGE_LOG_FD

cat >&AS_MESSAGE_LOG_FD <<EOF
## ------------ ##
## Core tests.  ##
## ------------ ##

EOF
])])# _AC_INIT_CONFIG_LOG

# _AC_INIT_PREPARE_FS_SEPARATORS
# ------------------------------
# Compute the directory and path separators.
# FIXME: Full version should include dir separator, documentation about
# AC_SUBST'ed variables etc.
m4_define([_AC_INIT_PREPARE_FS_SEPARATORS],
[echo "#! $SHELL" >conftest.sh
echo  "exit 0"   >>conftest.sh
chmod +x conftest.sh
if AC_RUN_LOG([PATH=".;."; conftest.sh]); then
  ac_path_separator=';'
else
  ac_path_separator=:
fi
AC_SUBST([PATH_SEPARATOR], "$ac_path_separator")dnl
rm -f conftest.sh
])


# _AC_INIT_PREPARE
# ----------------
# Called by AC_INIT to build the preamble of the `configure' scripts.
# 1. Trap and clean up various tmp files.
# 2. Set up the fd and output files
# 3. Remember the options given to `configure' for `config.status --recheck'.
# 4. Ensure a correct environment
# 5. Required macros (cache, default AC_SUBST etc.)
m4_define([_AC_INIT_PREPARE],
[m4_divert_push([INIT_PREPARE])dnl

# Keep a trace of the command line.
# Strip out --no-create and --no-recursion so they do not pile up.
# Also quote any args containing shell meta-characters.
ac_configure_args=
ac_sep=
for ac_arg
do
  case $ac_arg in
  -no-create | --no-create | --no-creat | --no-crea | --no-cre \
  | --no-cr | --no-c) ;;
  -no-recursion | --no-recursion | --no-recursio | --no-recursi \
  | --no-recurs | --no-recur | --no-recu | --no-rec | --no-re | --no-r) ;;
dnl If you change this globbing pattern, test it on an old shell --
dnl it's sensitive.  Putting any kind of quote in it causes syntax errors.
[  *" "*|*"	"*|*[\[\]\~\#\$\^\&\*\(\)\{\}\\\|\;\<\>\?\"\']*)]
    ac_arg=`echo "$ac_arg" | sed "s/'/'\\\\\\\\''/g"`
    ac_configure_args="$ac_configure_args$ac_sep'$ac_arg'"
    ac_sep=" " ;;
  *) ac_configure_args="$ac_configure_args$ac_sep$ac_arg"
     ac_sep=" " ;;
  esac
  # Get rid of the leading space.
done

# When interrupted or exit'd, cleanup temporary files, and complete
# config.log.  We remove comments because anyway the quotes in there
# would cause problems or look ugly.
trap 'exit_status=$?
  # Save into config.log some information that might help in debugging.
  echo >&AS_MESSAGE_LOG_FD
  echo ["## ----------------- ##"] >&AS_MESSAGE_LOG_FD
  echo ["## Cache variables.  ##"] >&AS_MESSAGE_LOG_FD
  echo ["## ----------------- ##"] >&AS_MESSAGE_LOG_FD
  echo >&AS_MESSAGE_LOG_FD
  m4_patsubst(m4_patsubst(m4_dquote(m4_defn([_AC_CACHE_DUMP])),
                          [^ *\(#.*\)?
]),
              ['], ['"'"']) >&AS_MESSAGE_LOG_FD
  sed "/^$/d" confdefs.h >conftest.log
  if test -s conftest.log; then
    echo >&AS_MESSAGE_LOG_FD
    echo ["## ------------ ##"] >&AS_MESSAGE_LOG_FD
    echo ["## confdefs.h.  ##"] >&AS_MESSAGE_LOG_FD
    echo ["## ------------ ##"] >&AS_MESSAGE_LOG_FD
    echo >&AS_MESSAGE_LOG_FD
    cat conftest.log >&AS_MESSAGE_LOG_FD
  fi
  (echo; echo) >&AS_MESSAGE_LOG_FD
  test "$ac_signal" != 0 &&
    echo "$as_me: caught signal $ac_signal" >&AS_MESSAGE_LOG_FD
  echo "$as_me: exit $exit_status" >&AS_MESSAGE_LOG_FD
  rm -rf conftest* confdefs* core core.* *.core conf$[$]* $ac_clean_files &&
    exit $exit_status
     ' 0
for ac_signal in 1 2 13 15; do
  trap 'ac_signal='$ac_signal'; AS_EXIT([1])' $ac_signal
done
ac_signal=0

# confdefs.h avoids OS command line length limits that DEFS can exceed.
rm -rf conftest* confdefs.h
# AIX cpp loses on an empty file, so make sure it contains at least a newline.
echo >confdefs.h

# Let the site file select an alternate cache file if it wants to.
AC_SITE_LOAD
AC_CACHE_LOAD
_AC_ARG_VAR_VALIDATE
_AC_ARG_VAR_PRECIOUS(build_alias)dnl
_AC_ARG_VAR_PRECIOUS(host_alias)dnl
_AC_ARG_VAR_PRECIOUS(target_alias)dnl
AC_LANG_PUSH(C)

_AC_PROG_ECHO()dnl
_AC_INIT_PREPARE_FS_SEPARATORS

dnl Substitute for predefined variables.
AC_SUBST(DEFS)dnl
AC_SUBST(LIBS)dnl
m4_divert_pop([INIT_PREPARE])dnl
])# _AC_INIT_PREPARE


# AU::AC_INIT([UNIQUE-FILE-IN-SOURCE-DIR])
# ----------------------------------------
# This macro is used only for Autoupdate.
AU_DEFUN([AC_INIT],
[m4_ifval([$2], [[AC_INIT($@)]],
          [m4_ifval([$1],
[[AC_INIT]
AC_CONFIG_SRCDIR([$1])], [[AC_INIT]])])[]dnl
])


# AC_PLAIN_SCRIPT
# ---------------
# Simulate AC_INIT, i.e., pretend this is the beginning of the `configure'
# generation.  This is used by some tests, and let `autoconf' be used to
# generate other scripts than `configure'.
m4_define([AC_PLAIN_SCRIPT],
[AS_INIT

# Forbidden tokens and exceptions.
m4_pattern_forbid([^_?A[CHUM]_])
m4_pattern_forbid([_AC_])
# Actually reserved by M4sh.
m4_pattern_allow([^AS_FLAGS$])

m4_divert_push([BODY])dnl
m4_wrap([m4_divert_pop([BODY])[]])dnl
])



# AC_INIT([PACKAGE, VERSION, [BUG-REPORT])
# ----------------------------------------
# Include the user macro files, prepare the diversions, and output the
# preamble of the `configure' script.
# Note that the order is important: first initialize, then set the
# AC_CONFIG_SRCDIR.
m4_define([AC_INIT],
[AC_PLAIN_SCRIPT
m4_ifval([$2], [_AC_INIT_PACKAGE($@)])
m4_divert_text([BINSH], [@%:@! /bin/sh])
_AC_INIT_DEFAULTS
_AC_INIT_PARSE_ARGS
_AC_INIT_SRCDIR
_AC_INIT_HELP
_AC_INIT_VERSION
_AC_INIT_CONFIG_LOG
_AC_INIT_PREPARE
_AC_INIT_NOTICE
_AC_INIT_COPYRIGHT
m4_ifval([$2], , [m4_ifval([$1], [AC_CONFIG_SRCDIR([$1])])])dnl
])




## ----------------------------- ##
## Selecting optional features.  ##
## ----------------------------- ##


# AC_ARG_ENABLE(FEATURE, HELP-STRING, [ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ------------------------------------------------------------------------
AC_DEFUN([AC_ARG_ENABLE],
[m4_divert_once([HELP_ENABLE], [[
Optional Features:
  --disable-FEATURE       do not include FEATURE (same as --enable-FEATURE=no)
  --enable-FEATURE[=ARG]  include FEATURE [ARG=yes]]])dnl
m4_divert_once([HELP_ENABLE], [$2])dnl
# Check whether --enable-$1 or --disable-$1 was given.
if test "[${enable_]m4_patsubst([$1], -, _)+set}" = set; then
  enableval="[$enable_]m4_patsubst([$1], -, _)"
  $3
m4_ifvaln([$4], [else
  $4])dnl
fi; dnl
])# AC_ARG_ENABLE


AU_DEFUN([AC_ENABLE],
[AC_ARG_ENABLE([$1], [  --enable-$1], [$2], [$3])])


## ------------------------------ ##
## Working with optional software ##
## ------------------------------ ##



# AC_ARG_WITH(PACKAGE, HELP-STRING, ACTION-IF-TRUE, [ACTION-IF-FALSE])
# --------------------------------------------------------------------
AC_DEFUN([AC_ARG_WITH],
[m4_divert_once([HELP_WITH], [[
Optional Packages:
  --with-PACKAGE[=ARG]    use PACKAGE [ARG=yes]
  --without-PACKAGE       do not use PACKAGE (same as --with-PACKAGE=no)]])
m4_divert_once([HELP_WITH], [$2])dnl
# Check whether --with-$1 or --without-$1 was given.
if test "[${with_]m4_patsubst([$1], -, _)+set}" = set; then
  withval="[$with_]m4_patsubst([$1], -, _)"
  $3
m4_ifvaln([$4], [else
  $4])dnl
fi; dnl
])# AC_ARG_WITH

AU_DEFUN([AC_WITH],
[AC_ARG_WITH([$1], [  --with-$1], [$2], [$3])])



## ----------------------------------------- ##
## Remembering variables for reconfiguring.  ##
## ----------------------------------------- ##


# _AC_ARG_VAR_PRECIOUS(VARNAME)
# -----------------------------
# Declare VARNAME is precious.
#
# We try to diagnose when precious variables have changed.  To do this,
# make two early snapshots (after the option processing to take
# explicit variables into account) of those variables: one (ac_env_)
# which represents the current run, and a second (ac_cv_env_) which,
# at the first run, will be saved in the cache.  As an exception to
# the cache mechanism, its loading will override these variables (non
# `ac_cv_env_' cache value are only set when unset).
#
# In subsequent runs, after having loaded the cache, compare
# ac_cv_env_foo against ac_env_foo.  See _AC_ARG_VAR_VALIDATE.
m4_define([_AC_ARG_VAR_PRECIOUS],
[AC_SUBST([$1])dnl
m4_divert_once([PARSE_ARGS],
[ac_env_$1_set=${$1+set}
ac_env_$1_value=$$1
ac_cv_env_$1_set=${$1+set}
ac_cv_env_$1_value=$$1])dnl
])


# _AC_ARG_VAR_VALIDATE
# --------------------
# The precious variables are saved twice at the beginning of
# configure.  E.g., PRECIOUS, is saved as `ac_env_PRECIOUS_SET' and
# `ac_env_PRECIOUS_VALUE' on the one hand and `ac_cv_env_PRECIOUS_SET'
# and `ac_cv_env_PRECIOUS_VALUE' on the other hand.
#
# Now the cache has just been load, so `ac_cv_env_' represents the
# content of the cached values, while `ac_env_' represents that of the
# current values.
#
# So we check that `ac_env_' and `ac_cv_env_' are consistant.  If
# they aren't, die.
m4_define([_AC_ARG_VAR_VALIDATE],
[# Check that the precious variables saved in the cache have kept the same
# value.
ac_cache_corrupted=false
for ac_var in `(set) 2>&1 |
               sed -n 's/^ac_env_\([[a-zA-Z_0-9]]*\)_set=.*/\1/p'`; do
  eval ac_old_set=\$ac_cv_env_${ac_var}_set
  eval ac_new_set=\$ac_env_${ac_var}_set
  eval ac_old_val="\$ac_cv_env_${ac_var}_value"
  eval ac_new_val="\$ac_env_${ac_var}_value"
  case $ac_old_set,$ac_new_set in
    set,)
      AS_MESSAGE([error: `$ac_var' was set to `$ac_old_val' in the previous run], 2)
      ac_cache_corrupted=: ;;
    ,set)
      AS_MESSAGE([error: `$ac_var' was not set in the previous run], 2)
      ac_cache_corrupted=: ;;
    ,);;
    *)
      if test "x$ac_old_val" != "x$ac_new_val"; then
        AS_MESSAGE([error: `$ac_var' has changed since the previous run:], 2)
        AS_MESSAGE([  former value:  $ac_old_val], 2)
        AS_MESSAGE([  current value: $ac_new_val], 2)
        ac_cache_corrupted=:
      fi;;
  esac
  # Pass precious variables to config.status.  It doesn't matter if
  # we pass some twice (in addition to the command line arguments).
  if test "$ac_new_set" = set; then
    case $ac_new_val in
dnl If you change this globbing pattern, test it on an old shell --
dnl it's sensitive.  Putting any kind of quote in it causes syntax errors.
[    *" "*|*"	"*|*[\[\]\~\#\$\^\&\*\(\)\{\}\\\|\;\<\>\?\"\']*)]
      ac_arg=$ac_var=`echo "$ac_new_val" | sed "s/'/'\\\\\\\\''/g"`
      ac_configure_args="$ac_configure_args '$ac_arg'"
      ;;
    *) ac_configure_args="$ac_configure_args $ac_var=$ac_new_val"
       ;;
    esac
  fi
done
if $ac_cache_corrupted; then
  AS_MESSAGE([error: changes in the environment can compromise the build], 2)
  AS_ERROR([run `make distclean' and/or `rm $cache_file' and start over])
fi
])# _AC_ARG_VAR_VALIDATE


# AC_ARG_VAR(VARNAME, DOCUMENTATION)
# ----------------------------------
# Register VARNAME as a precious variable, and document it in
# `configure --help' (but only once).
AC_DEFUN([AC_ARG_VAR],
[m4_divert_once([HELP_VAR], [[
Some influential environment variables:]])dnl
m4_divert_once([HELP_VAR_END], [[
Use these variables to override the choices made by `configure' or to help
it to find libraries and programs with nonstandard names/locations.]])dnl
m4_expand_once([m4_divert_once([HELP_VAR],
                               [AC_HELP_STRING([$1], [$2], [              ])])],
               [$0($1)])dnl
_AC_ARG_VAR_PRECIOUS([$1])dnl
])# AC_ARG_VAR





## ---------------------------- ##
## Transforming program names.  ##
## ---------------------------- ##


# AC_ARG_PROGRAM
# --------------
# This macro is expanded only once, to avoid that `foo' ends up being
# installed as `ggfoo'.
AC_DEFUN_ONCE([AC_ARG_PROGRAM],
[dnl Document the options.
m4_divert_push([HELP_BEGIN])dnl

Program names:
  --program-prefix=PREFIX            prepend PREFIX to installed program names
  --program-suffix=SUFFIX            append SUFFIX to installed program names
  --program-transform-name=PROGRAM   run sed PROGRAM on installed program names
m4_divert_pop([HELP_BEGIN])dnl
test "$program_prefix" != NONE &&
  program_transform_name="s,^,$program_prefix,;$program_transform_name"
# Use a double $ so make ignores it.
test "$program_suffix" != NONE &&
  program_transform_name="s,\$,$program_suffix,;$program_transform_name"
# Double any \ or $.  echo might interpret backslashes.
# By default was `s,x,x', remove it if useless.
cat <<\_ACEOF >conftest.sed
[s/[\\$]/&&/g;s/;s,x,x,$//]
_ACEOF
program_transform_name=`echo $program_transform_name | sed -f conftest.sed`
rm conftest.sed
])# AC_ARG_PROGRAM





## ------------------------- ##
## Finding auxiliary files.  ##
## ------------------------- ##


# AC_CONFIG_AUX_DIR(DIR)
# ----------------------
# Find install-sh, config.sub, config.guess, and Cygnus configure
# in directory DIR.  These are auxiliary files used in configuration.
# DIR can be either absolute or relative to $srcdir.
AC_DEFUN([AC_CONFIG_AUX_DIR],
[AC_CONFIG_AUX_DIRS($1 $srcdir/$1)])


# AC_CONFIG_AUX_DIR_DEFAULT
# -------------------------
# The default is `$srcdir' or `$srcdir/..' or `$srcdir/../..'.
# There's no need to call this macro explicitly; just AC_REQUIRE it.
AC_DEFUN([AC_CONFIG_AUX_DIR_DEFAULT],
[AC_CONFIG_AUX_DIRS($srcdir $srcdir/.. $srcdir/../..)])


# AC_CONFIG_AUX_DIRS(DIR ...)
# ---------------------------
# Internal subroutine.
# Search for the configuration auxiliary files in directory list $1.
# We look only for install-sh, so users of AC_PROG_INSTALL
# do not automatically need to distribute the other auxiliary files.
AC_DEFUN([AC_CONFIG_AUX_DIRS],
[ac_aux_dir=
for ac_dir in $1; do
  if test -f $ac_dir/install-sh; then
    ac_aux_dir=$ac_dir
    ac_install_sh="$ac_aux_dir/install-sh -c"
    break
  elif test -f $ac_dir/install.sh; then
    ac_aux_dir=$ac_dir
    ac_install_sh="$ac_aux_dir/install.sh -c"
    break
  elif test -f $ac_dir/shtool; then
    ac_aux_dir=$ac_dir
    ac_install_sh="$ac_aux_dir/shtool install -c"
    break
  fi
done
if test -z "$ac_aux_dir"; then
  AC_MSG_ERROR([cannot find install-sh or install.sh in $1])
fi
ac_config_guess="$SHELL $ac_aux_dir/config.guess"
ac_config_sub="$SHELL $ac_aux_dir/config.sub"
ac_configure="$SHELL $ac_aux_dir/configure" # This should be Cygnus configure.
AC_PROVIDE([AC_CONFIG_AUX_DIR_DEFAULT])dnl
])# AC_CONFIG_AUX_DIRS




## ----------------------------------- ##
## Getting the canonical system type.  ##
## ----------------------------------- ##

# The inputs are:
#    configure --host=HOST --target=TARGET --build=BUILD
#
# The rules are:
# 1. Build defaults to the current platform, as determined by config.guess.
# 2. Host defaults to build.
# 3. Target defaults to host.


# _AC_CANONICAL_SPLIT(THING)
# --------------------------
# Generate the variables THING, THING_{alias cpu vendor os}.
m4_define([_AC_CANONICAL_SPLIT],
[AC_SUBST([$1],       [$ac_cv_$1])dnl
AC_SUBST([$1_cpu],
         [`echo $ac_cv_$1 | sed 's/^\([[^-]]*\)-\([[^-]]*\)-\(.*\)$/\1/'`])dnl
AC_SUBST([$1_vendor],
         [`echo $ac_cv_$1 | sed 's/^\([[^-]]*\)-\([[^-]]*\)-\(.*\)$/\2/'`])dnl
AC_SUBST([$1_os],
         [`echo $ac_cv_$1 | sed 's/^\([[^-]]*\)-\([[^-]]*\)-\(.*\)$/\3/'`])dnl
])# _AC_CANONICAL_SPLIT


# AC_CANONICAL_BUILD
# ------------------
AC_DEFUN_ONCE([AC_CANONICAL_BUILD],
[AC_REQUIRE([AC_CONFIG_AUX_DIR_DEFAULT])dnl
m4_divert_text([HELP_CANON],
[[
System types:
  --build=BUILD     configure for building on BUILD [guessed]]])dnl
# Make sure we can run config.sub.
$ac_config_sub sun4 >/dev/null 2>&1 ||
  AC_MSG_ERROR([cannot run $ac_config_sub])

AC_CACHE_CHECK([build system type], [ac_cv_build],
[ac_cv_build_alias=$build_alias
test -z "$ac_cv_build_alias" &&
  ac_cv_build_alias=`$ac_config_guess`
test -z "$ac_cv_build_alias" &&
  AC_MSG_ERROR([cannot guess build type; you must specify one])
ac_cv_build=`$ac_config_sub $ac_cv_build_alias` ||
  AC_MSG_ERROR([$ac_config_sub $ac_cv_build_alias failed.])
])
_AC_CANONICAL_SPLIT(build)
test -z "$build_alias" &&
  build_alias=$ac_cv_build
])# AC_CANONICAL_BUILD


# AC_CANONICAL_HOST
# -----------------
AC_DEFUN_ONCE([AC_CANONICAL_HOST],
[AC_REQUIRE([AC_CANONICAL_BUILD])dnl
m4_divert_text([HELP_CANON],
[[  --host=HOST       build programs to run on HOST [BUILD]]])dnl
AC_CACHE_CHECK([host system type], [ac_cv_host],
[ac_cv_host_alias=$host_alias
test -z "$ac_cv_host_alias" &&
  ac_cv_host_alias=$ac_cv_build_alias
ac_cv_host=`$ac_config_sub $ac_cv_host_alias` ||
  AC_MSG_ERROR([$ac_config_sub $ac_cv_host_alias failed])
])
_AC_CANONICAL_SPLIT([host])
test -z "$host_alias" &&
  host_alias=$ac_cv_host
])# AC_CANONICAL_HOST


# AC_CANONICAL_TARGET
# -------------------
AC_DEFUN_ONCE([AC_CANONICAL_TARGET],
[AC_REQUIRE([AC_CANONICAL_HOST])dnl
AC_BEFORE([$0], [AC_ARG_PROGRAM])dnl
m4_divert_text([HELP_CANON],
[[  --target=TARGET   configure for building compilers for TARGET [HOST]]])dnl
AC_CACHE_CHECK([target system type], [ac_cv_target],
[dnl Set target_alias.
ac_cv_target_alias=$target_alias
test "x$ac_cv_target_alias" = "x" &&
  ac_cv_target_alias=$ac_cv_host_alias
ac_cv_target=`$ac_config_sub $ac_cv_target_alias` ||
  AC_MSG_ERROR([$ac_config_sub $ac_cv_target_alias failed])
])
_AC_CANONICAL_SPLIT([target])

# The aliases save the names the user supplied, while $host etc.
# will get canonicalized.
test -n "$target_alias" &&
  test "$program_prefix$program_suffix$program_transform_name" = \
    NONENONEs,x,x, &&
  program_prefix=${target_alias}-
test -z "$target_alias" &&
  target_alias=$ac_cv_target
])# AC_CANONICAL_TARGET


AU_ALIAS([AC_CANONICAL_SYSTEM], [AC_CANONICAL_TARGET])


# AU::AC_VALIDATE_CACHED_SYSTEM_TUPLE([CMD])
# ------------------------------------------
# If the cache file is inconsistent with the current host,
# target and build system types, execute CMD or print a default
# error message.  Now handled via _AC_ARG_VAR_PRECIOUS.
AU_DEFUN([AC_VALIDATE_CACHED_SYSTEM_TUPLE], [])


## ---------------------- ##
## Caching test results.  ##
## ---------------------- ##


# AC_SITE_LOAD
# ------------
# Look for site or system specific initialization scripts.
m4_define([AC_SITE_LOAD],
[# Prefer explicitly selected file to automatically selected ones.
if test -z "$CONFIG_SITE"; then
  if test "x$prefix" != xNONE; then
    CONFIG_SITE="$prefix/share/config.site $prefix/etc/config.site"
  else
    CONFIG_SITE="$ac_default_prefix/share/config.site $ac_default_prefix/etc/config.site"
  fi
fi
for ac_site_file in $CONFIG_SITE; do
  if test -r "$ac_site_file"; then
    AC_MSG_NOTICE([loading site script $ac_site_file])
    cat "$ac_site_file" >&AS_MESSAGE_LOG_FD
    . "$ac_site_file"
  fi
done
])


# AC_CACHE_LOAD
# -------------
m4_define([AC_CACHE_LOAD],
[if test -r "$cache_file"; then
  # Some versions of bash will fail to source /dev/null (special
  # files actually), so we avoid doing that.
  if test -f "$cache_file"; then
    AC_MSG_NOTICE([loading cache $cache_file])
    case $cache_file in
      [[\\/]]* | ?:[[\\/]]* ) . $cache_file;;
      *)                      . ./$cache_file;;
    esac
  fi
else
  AC_MSG_NOTICE([creating cache $cache_file])
  >$cache_file
fi
])# AC_CACHE_LOAD


# _AC_CACHE_DUMP
# --------------
# Dump the cache to stdout.  It can be in a pipe (this is a requirement).
m4_define([_AC_CACHE_DUMP],
[# The following way of writing the cache mishandles newlines in values,
# but we know of no workaround that is simple, portable, and efficient.
# So, don't put newlines in cache variables' values.
# Ultrix sh set writes to stderr and can't be redirected directly,
# and sets the high bit in the cache file unless we assign to the vars.
{
  (set) 2>&1 |
    case `(ac_space=' '; set | grep ac_space) 2>&1` in
    *ac_space=\ *)
      # `set' does not quote correctly, so add quotes (double-quote
      # substitution turns \\\\ into \\, and sed turns \\ into \).
      sed -n \
        ["s/'/'\\\\''/g;
    	  s/^\\([_$as_cr_alnum]*_cv_[_$as_cr_alnum]*\\)=\\(.*\\)/\\1='\\2'/p"]
      ;;
    *)
      # `set' quotes correctly as required by POSIX, so do not add quotes.
      sed -n \
        ["s/^\\([_$as_cr_alnum]*_cv_[_$as_cr_alnum]*\\)=\\(.*\\)/\\1=\\2/p"]
      ;;
    esac;
}dnl
])# _AC_CACHE_DUMP


# AC_CACHE_SAVE
# -------------
# Save the cache.
# Allow a site initialization script to override cache values.
m4_define([AC_CACHE_SAVE],
[cat >confcache <<\_ACEOF
# This file is a shell script that caches the results of configure
# tests run on this system so they can be shared between configure
# scripts and configure runs, see configure's option --config-cache.
# It is not useful on other systems.  If it contains results you don't
# want to keep, you may remove or edit it.
#
# config.status only pays attention to the cache file if you give it
# the --recheck option to rerun configure.
#
# `ac_cv_env_foo' variables (set or unset) will be overriden when
# loading this file, other *unset* `ac_cv_foo' will be assigned the
# following values.

_ACEOF

_AC_CACHE_DUMP() |
  sed ['
     t clear
     : clear
     s/^\([^=]*\)=\(.*[{}].*\)$/test "${\1+set}" = set || &/
     t end
     /^ac_cv_env/!s/^\([^=]*\)=\(.*\)$/\1=${\1=\2}/
     : end'] >>confcache
if cmp -s $cache_file confcache; then :; else
  if test -w $cache_file; then
    test "x$cache_file" != "x/dev/null" && echo "updating cache $cache_file"
    cat confcache >$cache_file
  else
    echo "not updating unwritable cache $cache_file"
  fi
fi
rm -f confcache[]dnl
])# AC_CACHE_SAVE


# AC_CACHE_VAL(CACHE-ID, COMMANDS-TO-SET-IT)
# ------------------------------------------
# The name of shell var CACHE-ID must contain `_cv_' in order to get saved.
# Should be dnl'ed.  Try to catch common mistakes.
m4_define([AC_CACHE_VAL],
[m4_if(m4_regexp([$2], [AC_DEFINE]), [-1], [],
      [AC_DIAGNOSE(syntax,
[$0($1, ...): suspicious presence of an AC_DEFINE in the second argument, ]dnl
[where no actions should be taken])])dnl
AS_VAR_SET_IF([$1],
              [echo $ECHO_N "(cached) $ECHO_C" >&AS_MESSAGE_FD],
              [$2])])


# AC_CACHE_CHECK(MESSAGE, CACHE-ID, COMMANDS)
# -------------------------------------------
# Do not call this macro with a dnl right behind.
m4_define([AC_CACHE_CHECK],
[AC_MSG_CHECKING([$1])
AC_CACHE_VAL([$2], [$3])dnl
AC_MSG_RESULT_UNQUOTED([AS_VAR_GET([$2])])])



## ---------------------- ##
## Defining CPP symbols.  ##
## ---------------------- ##


# AC_DEFINE_TRACE_LITERAL(LITERAL-CPP-SYMBOL)
# -------------------------------------------
# This macro is useless, it is used only with --trace to collect the
# list of *literals* CPP values passed to AC_DEFINE/AC_DEFINE_UNQUOTED.
m4_define([AC_DEFINE_TRACE_LITERAL])


# AC_DEFINE_TRACE(CPP-SYMBOL)
# ---------------------------
# This macro is a wrapper around AC_DEFINE_TRACE_LITERAL which filters
# out non literal symbols.
m4_define([AC_DEFINE_TRACE],
[AS_LITERAL_IF([$1], [AC_DEFINE_TRACE_LITERAL([$1])])])


# AC_DEFINE(VARIABLE, [VALUE], [DESCRIPTION])
# -------------------------------------------
# Set VARIABLE to VALUE, verbatim, or 1.  Remember the value
# and if VARIABLE is affected the same VALUE, do nothing, else
# die.  The third argument is used by autoheader.
m4_define([AC_DEFINE],
[AC_DEFINE_TRACE([$1])dnl
m4_ifval([$3], [_AH_TEMPLATE_OLD([$1], [$3])])dnl
cat >>confdefs.h <<\EOF
[@%:@define] $1 m4_if($#, 2, [$2], $#, 3, [$2], 1)
EOF
])


# AC_DEFINE_UNQUOTED(VARIABLE, [VALUE], [DESCRIPTION])
# ----------------------------------------------------
# Similar, but perform shell substitutions $ ` \ once on VALUE.
m4_define([AC_DEFINE_UNQUOTED],
[AC_DEFINE_TRACE([$1])dnl
m4_ifval([$3], [_AH_TEMPLATE_OLD([$1], [$3])])dnl
cat >>confdefs.h <<EOF
[@%:@define] $1 m4_if($#, 2, [$2], $#, 3, [$2], 1)
EOF
])



## -------------------------- ##
## Setting output variables.  ##
## -------------------------- ##


# _AC_SUBST(VARIABLE, PROGRAM)
# ----------------------------
# If VARIABLE has not already been AC_SUBST'ed, append the sed PROGRAM
# to `_AC_SUBST_SED_PROGRAM'.
m4_define([_AC_SUBST],
[m4_expand_once([m4_append([_AC_SUBST_SED_PROGRAM],
[$2
])])dnl
])

# Initialize.
m4_define([_AC_SUBST_SED_PROGRAM])


# AC_SUBST(VARIABLE, [VALUE])
# ---------------------------
# Create an output variable from a shell VARIABLE.  If VALUE is given
# assign it to VARIABLE.  Use `""' is you want to set VARIABLE to an
# empty value, not an empty second argument.
#
# Beware that if you change this macro, you also have to change the
# sed script at the top of _AC_OUTPUT_FILES.
m4_define([AC_SUBST],
[m4_ifvaln([$2], [$1=$2])[]dnl
_AC_SUBST([$1], [s,@$1@,[$]$1,;t t])dnl
])# AC_SUBST


# AC_SUBST_FILE(VARIABLE)
# -----------------------
# Read the comments of the preceding macro.
m4_define([AC_SUBST_FILE],
[_AC_SUBST([$1], [/@$1@/r [$]$1
s,@$1@,,;t t])])



## --------------------------------------- ##
## Printing messages at autoconf runtime.  ##
## --------------------------------------- ##

# In fact, I think we should promote the use of m4_warn and m4_fatal
# directly.  This will also avoid to some people to get it wrong
# between AC_FATAL and AC_MSG_ERROR.


# AC_DIAGNOSE(CATEGORY, MESSAGE)
# AC_FATAL(MESSAGE, [EXIT-STATUS])
# --------------------------------
m4_copy([m4_warn],  [AC_DIAGNOSE])
m4_copy([m4_fatal], [AC_FATAL])


# AC_WARNING(MESSAGE)
# -------------------
# Report a MESSAGE to the user of autoconf if `-W' or `-W all' was
# specified.
m4_define([AC_WARNING],
[AC_DIAGNOSE([syntax], [$1])])




## ---------------------------------------- ##
## Printing messages at configure runtime.  ##
## ---------------------------------------- ##


# _AC_ECHO_N(STRING, [FD = AS_MESSAGE_FD])
# ------------------------------------
# Same as _AS_ECHO, but echo doesn't return to a new line.
m4_define([_AC_ECHO_N],
[echo $ECHO_N "_AS_QUOTE([$1])$ECHO_C" >&m4_default([$2],
                                                    [AS_MESSAGE_FD])])


# AC_MSG_CHECKING(FEATURE)
# ------------------------
m4_define([AC_MSG_CHECKING],
[_AS_ECHO([$as_me:__oline__: checking $1], AS_MESSAGE_LOG_FD)
_AC_ECHO_N([checking $1... ])[]dnl
])


# AC_MSG_RESULT(RESULT)
# ---------------------
m4_define([AC_MSG_RESULT],
[_AS_ECHO([$as_me:__oline__: result: $1], AS_MESSAGE_LOG_FD)
_AS_ECHO([${ECHO_T}$1])[]dnl
])


# AC_MSG_RESULT_UNQUOTED(RESULT)
# ------------------------------
# Likewise, but perform $ ` \ shell substitutions.
m4_define([AC_MSG_RESULT_UNQUOTED],
[_AS_ECHO_UNQUOTED([$as_me:__oline__: result: $1], AS_MESSAGE_LOG_FD)
_AS_ECHO_UNQUOTED([${ECHO_T}$1])[]dnl
])


# AC_MSG_WARN(PROBLEM)
# AC_MSG_NOTICE(STRING)
# AC_MSG_ERROR(ERROR, [EXIT-STATUS = 1])
# --------------------------------------
m4_copy([AS_WARN],    [AC_MSG_WARN])
m4_copy([AS_MESSAGE], [AC_MSG_NOTICE])
m4_copy([AS_ERROR],   [AC_MSG_ERROR])


# AU::AC_CHECKING(FEATURE)
# ------------------------
AU_DEFUN([AC_CHECKING],
[AS_MESSAGE([checking $1...])])


# AU::AC_VERBOSE(STRING)
# ----------------------
AU_ALIAS([AC_VERBOSE], [AC_MSG_RESULT])






## ---------------------------- ##
## Compiler-running mechanics.  ##
## ---------------------------- ##


# _AC_RUN_LOG(COMMAND, LOG-COMMANDS)
# ----------------------------------
# Eval COMMAND, save the exit status in ac_status, and log it.
AC_DEFUN([_AC_RUN_LOG],
[{ ($2) >&AS_MESSAGE_LOG_FD
  ($1) 2>&AS_MESSAGE_LOG_FD
  ac_status=$?
  echo "$as_me:__oline__: \$? = $ac_status" >&AS_MESSAGE_LOG_FD
  (exit $ac_status); }])


# _AC_RUN_LOG_STDERR(COMMAND, LOG-COMMANDS)
# -----------------------------------------
# Eval COMMAND, save its stderr into conftest.err, save the exit status
# in ac_status, and log it.
# Note that when tracing, most shells will leave the traces in stderr
AC_DEFUN([_AC_RUN_LOG_STDERR],
[{ ($2) >&AS_MESSAGE_LOG_FD
  ($1) 2>conftest.er1
  ac_status=$?
  egrep -v '^ *\+' conftest.er1 >conftest.err
  rm -f conftest.er1
  cat conftest.err >&AS_MESSAGE_LOG_FD
  echo "$as_me:__oline__: \$? = $ac_status" >&AS_MESSAGE_LOG_FD
  (exit $ac_status); }])


# _AC_EVAL(COMMAND)
# -----------------
# Eval COMMAND, save the exit status in ac_status, and log it.
AC_DEFUN([_AC_EVAL],
[_AC_RUN_LOG([eval $1],
             [eval echo "$as_me:__oline__: \"$1\""])])


# _AC_EVAL_STDERR(COMMAND)
# ------------------------
# Eval COMMAND, save its stderr into conftest.err, save the exit status
# in ac_status, and log it.
# Note that when tracing, most shells will leave the traces in stderr
AC_DEFUN([_AC_EVAL_STDERR],
[_AC_RUN_LOG_STDERR([eval $1],
                    [eval echo "$as_me:__oline__: \"$1\""])])


# AC_TRY_EVAL(VARIABLE)
# ---------------------
# The purpose of this macro is to "configure:123: command line"
# written into config.log for every test run.
AC_DEFUN([AC_TRY_EVAL],
[_AC_EVAL([$$1])])


# AC_TRY_COMMAND(COMMAND)
# -----------------------
AC_DEFUN([AC_TRY_COMMAND],
[{ ac_try='$1'
  _AC_EVAL([$ac_try]); }])


# AC_RUN_LOG(COMMAND)
# -------------------
AC_DEFUN([AC_RUN_LOG],
[_AC_RUN_LOG([$1],
             [echo "$as_me:__oline__: AS_ESCAPE([$1])"])])


## ------------------ ##
## Default includes.  ##
## ------------------ ##

# Always use the same set of default headers for all the generic
# macros.  It is easier to document, to extend, and to understand than
# having specific defaults for each macro.

# _AC_INCLUDES_DEFAULT_REQUIREMENTS
# ---------------------------------
# Required when AC_INCLUDES_DEFAULT uses its default branch.
AC_DEFUN([_AC_INCLUDES_DEFAULT_REQUIREMENTS],
[m4_divert_text([DEFAULTS],
[# Factoring default headers for most tests.
dnl If ever you change this variable, please keep autoconf.texi in sync.
ac_includes_default="\
#include <stdio.h>
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif
#if HAVE_INTTYPES_H
# include <inttypes.h>
#else
# if HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif"
])dnl
AC_REQUIRE([AC_HEADER_STDC])dnl
# On IRIX 5.3, sys/types and inttypes.h are conflicting.
AC_CHECK_HEADERS([sys/types.h sys/stat.h stdlib.h string.h memory.h strings.h \
                  inttypes.h stdint.h unistd.h],
                 [], [], $ac_includes_default)
])# _AC_INCLUDES_DEFAULT_REQUIREMENTS


# AC_INCLUDES_DEFAULT([INCLUDES])
# -------------------------------
# If INCLUDES is empty, expand in default includes, otherwise in
# INCLUDES.
# In most cases INCLUDES is not double quoted as it should, and if
# for instance INCLUDES = `#include <stdio.h>' then unless we force
# a newline, the hash will swallow the closing paren etc. etc.
# The usual failure.
# Take no risk: for the newline.
AC_DEFUN([AC_INCLUDES_DEFAULT],
[m4_ifval([$1], [$1
],
          [AC_REQUIRE([_AC_INCLUDES_DEFAULT_REQUIREMENTS])dnl
$ac_includes_default])])




## ----------------------- ##
## Checking for programs.  ##
## ----------------------- ##


# AC_SHELL_PATH_WALK([PATH = $PATH], BODY)
# ----------------------------------------
# Walk through PATH running BODY for each `ac_dir'.
#
# `$ac_dummy' forces splitting on constant user-supplied paths.
# POSIX.2 word splitting is done only on the output of word
# expansions, not every word.  This closes a longstanding sh security
# hole.
m4_define([AC_SHELL_PATH_WALK],
[ac_save_IFS=$IFS; IFS=$ac_path_separator
ac_dummy="m4_default([$1], [$PATH])"
for ac_dir in $ac_dummy; do
  IFS=$ac_save_IFS
  test -z "$ac_dir" && ac_dir=.
  $2
done
])


# AC_CHECK_PROG(VARIABLE, PROG-TO-CHECK-FOR,
#               [VALUE-IF-FOUND], [VALUE-IF-NOT-FOUND],
#               [PATH], [REJECT])
# -----------------------------------------------------
AC_DEFUN([AC_CHECK_PROG],
[# Extract the first word of "$2", so it can be a program name with args.
set dummy $2; ac_word=$[2]
AC_MSG_CHECKING([for $ac_word])
AC_CACHE_VAL(ac_cv_prog_$1,
[if test -n "$$1"; then
  ac_cv_prog_$1="$$1" # Let the user override the test.
else
m4_ifvaln([$6],
[  ac_prog_rejected=no])dnl
  AC_SHELL_PATH_WALK([$5],
[AS_EXECUTABLE_P("$ac_dir/$ac_word") || continue
m4_ifvaln([$6],
[if test "$ac_dir/$ac_word" = "$6"; then
  ac_prog_rejected=yes
  continue
fi])dnl
ac_cv_prog_$1="$3"
echo "$as_me:__oline__: found $ac_dir/$ac_word" >&AS_MESSAGE_LOG_FD
break])
m4_ifvaln([$6],
[if test $ac_prog_rejected = yes; then
  # We found a bogon in the path, so make sure we never use it.
  set dummy $ac_cv_prog_$1
  shift
  if test $[@%:@] != 0; then
    # We chose a different compiler from the bogus one.
    # However, it has the same basename, so the bogon will be chosen
    # first if we set $1 to just the basename; use the full file name.
    shift
    set dummy "$ac_dir/$ac_word" ${1+"$[@]"}
    shift
    ac_cv_prog_$1="$[@]"
m4_if([$2], [$4],
[  else
    # Default is a loser.
    AC_MSG_ERROR([$1=$6 unacceptable, but no other $4 found in dnl
m4_default([$5], [\$PATH])])
])dnl
  fi
fi])dnl
dnl If no 4th arg is given, leave the cache variable unset,
dnl so AC_CHECK_PROGS will keep looking.
m4_ifvaln([$4],
[  test -z "$ac_cv_prog_$1" && ac_cv_prog_$1="$4"])dnl
fi])dnl
$1=$ac_cv_prog_$1
if test -n "$$1"; then
  AC_MSG_RESULT([$$1])
else
  AC_MSG_RESULT([no])
fi
AC_SUBST($1)dnl
])# AC_CHECK_PROG


# AC_CHECK_PROGS(VARIABLE, PROGS-TO-CHECK-FOR, [VALUE-IF-NOT-FOUND],
#                [PATH])
# ------------------------------------------------------------------
AC_DEFUN([AC_CHECK_PROGS],
[for ac_prog in $2
do
  AC_CHECK_PROG([$1], [$ac_prog], [$ac_prog], , [$4])
  test -n "$$1" && break
done
m4_ifvaln([$3], [test -n "$$1" || $1="$3"])])


# AC_PATH_PROG(VARIABLE, PROG-TO-CHECK-FOR, [VALUE-IF-NOT-FOUND], [PATH])
# -----------------------------------------------------------------------
AC_DEFUN([AC_PATH_PROG],
[# Extract the first word of "$2", so it can be a program name with args.
set dummy $2; ac_word=$[2]
AC_MSG_CHECKING([for $ac_word])
AC_CACHE_VAL([ac_cv_path_$1],
[case $$1 in
  [[\\/]]* | ?:[[\\/]]*)
  ac_cv_path_$1="$$1" # Let the user override the test with a path.
  ;;
  *)
  AC_SHELL_PATH_WALK([$4],
[if AS_EXECUTABLE_P("$ac_dir/$ac_word"); then
   ac_cv_path_$1="$ac_dir/$ac_word"
   echo "$as_me:__oline__: found $ac_dir/$ac_word" >&AS_MESSAGE_LOG_FD
   break
fi])
dnl If no 3rd arg is given, leave the cache variable unset,
dnl so AC_PATH_PROGS will keep looking.
m4_ifvaln([$3],
[  test -z "$ac_cv_path_$1" && ac_cv_path_$1="$3"])dnl
  ;;
esac])dnl
AC_SUBST([$1], [$ac_cv_path_$1])
if test -n "$$1"; then
  AC_MSG_RESULT([$$1])
else
  AC_MSG_RESULT([no])
fi
])# AC_PATH_PROG


# AC_PATH_PROGS(VARIABLE, PROGS-TO-CHECK-FOR, [VALUE-IF-NOT-FOUND],
#               [PATH])
# -----------------------------------------------------------------
AC_DEFUN([AC_PATH_PROGS],
[for ac_prog in $2
do
  AC_PATH_PROG([$1], [$ac_prog], , [$4])
  test -n "$$1" && break
done
m4_ifvaln([$3], [test -n "$$1" || $1="$3"])dnl
])




## -------------------- ##
## Checking for tools.  ##
## -------------------- ##


# AC_CHECK_TOOL_PREFIX
# --------------------
AU_DEFUN([AC_CHECK_TOOL_PREFIX])


# AC_PATH_TOOL(VARIABLE, PROG-TO-CHECK-FOR, [VALUE-IF-NOT-FOUND], [PATH])
# -----------------------------------------------------------------------
# (Use different variables $1 and ac_pt_$1 so that cache vars don't conflict.)
AC_DEFUN([AC_PATH_TOOL],
[if test -n "$ac_tool_prefix"; then
  AC_PATH_PROG([$1], [${ac_tool_prefix}$2], , [$4])
fi
if test -z "$ac_cv_path_$1"; then
  ac_pt_$1=$$1
  AC_PATH_PROG([ac_pt_$1], [$2], [$3], [$4])
  $1=$ac_pt_$1
else
  $1="$ac_cv_path_$1"
fi
])# AC_PATH_TOOL


# AC_CHECK_TOOL(VARIABLE, PROG-TO-CHECK-FOR, [VALUE-IF-NOT-FOUND], [PATH])
# ------------------------------------------------------------------------
# (Use different variables $1 and ac_ct_$1 so that cache vars don't conflict.)
AC_DEFUN([AC_CHECK_TOOL],
[if test -n "$ac_tool_prefix"; then
  AC_CHECK_PROG([$1], [${ac_tool_prefix}$2], [${ac_tool_prefix}$2], , [$4])
fi
if test -z "$ac_cv_prog_$1"; then
  ac_ct_$1=$$1
  AC_CHECK_PROG([ac_ct_$1], [$2], [$2], [$3], [$4])
  $1=$ac_ct_$1
else
  $1="$ac_cv_prog_$1"
fi
])# AC_CHECK_TOOL


# AC_CHECK_TOOLS(VARIABLE, PROGS-TO-CHECK-FOR, [VALUE-IF-NOT-FOUND],
#                [PATH])
# ------------------------------------------------------------------
# Check for each tool in PROGS-TO-CHECK-FOR with the cross prefix. If
# none can be found with a cross prefix, then use the first one that
# was found without the cross prefix.
AC_DEFUN([AC_CHECK_TOOLS],
[if test -n "$ac_tool_prefix"; then
  for ac_prog in $2
  do
    AC_CHECK_PROG([$1],
                  [$ac_tool_prefix$ac_prog], [$ac_tool_prefix$ac_prog],,
                  [$4])
    test -n "$$1" && break
  done
fi
if test -z "$$1"; then
  ac_ct_$1=$$1
  AC_CHECK_PROGS([ac_ct_$1], [$2], [$3], [$4])
  $1=$ac_ct_$1
fi
])# AC_CHECK_TOOLS


# AC_PREFIX_PROGRAM(PROGRAM)
# --------------------------
# Guess the value for the `prefix' variable by looking for
# the argument program along PATH and taking its parent.
# Example: if the argument is `gcc' and we find /usr/local/gnu/bin/gcc,
# set `prefix' to /usr/local/gnu.
# This comes too late to find a site file based on the prefix,
# and it might use a cached value for the path.
# No big loss, I think, since most configures don't use this macro anyway.
AC_DEFUN([AC_PREFIX_PROGRAM],
[dnl Get an upper case version of $[1].
m4_pushdef([AC_Prog], m4_toupper([$1]))dnl
if test "x$prefix" = xNONE; then
dnl We reimplement AC_MSG_CHECKING (mostly) to avoid the ... in the middle.
  echo $ECHO_N "checking for prefix by $ECHO_C" >&AS_MESSAGE_FD
  AC_PATH_PROG(m4_quote(AC_Prog), [$1])
  if test -n "$ac_cv_path_[]AC_Prog"; then
    prefix=`AS_DIRNAME(["$ac_cv_path_[]AC_Prog"])`
  fi
fi
m4_popdef([AC_Prog])dnl
])# AC_PREFIX_PROGRAM




## ------------------------ ##
## Checking for libraries.  ##
## ------------------------ ##


# AC_TRY_LINK_FUNC(FUNC, ACTION-IF-FOUND, ACTION-IF-NOT-FOUND)
# ------------------------------------------------------------
# Try to link a program that calls FUNC, handling GCC builtins.  If
# the link succeeds, execute ACTION-IF-FOUND; otherwise, execute
# ACTION-IF-NOT-FOUND.
AC_DEFUN([AC_TRY_LINK_FUNC],
[AC_LINK_IFELSE([AC_LANG_CALL([], [$1])], [$2], [$3])])


# AC_SEARCH_LIBS(FUNCTION, SEARCH-LIBS,
#                [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#                [OTHER-LIBRARIES])
# --------------------------------------------------------
# Search for a library defining FUNC, if it's not already available.
AC_DEFUN([AC_SEARCH_LIBS],
[AC_CACHE_CHECK([for library containing $1], [ac_cv_search_$1],
[ac_func_search_save_LIBS=$LIBS
ac_cv_search_$1=no
AC_TRY_LINK_FUNC([$1], [ac_cv_search_$1="none required"])
if test "$ac_cv_search_$1" = no; then
  for ac_lib in $2; do
    LIBS="-l$ac_lib $5 $ac_func_search_save_LIBS"
    AC_TRY_LINK_FUNC([$1],
                     [ac_cv_search_$1="-l$ac_lib"
break])
  done
fi
LIBS=$ac_func_search_save_LIBS])
AS_IF([test "$ac_cv_search_$1" != no],
  [test "$ac_cv_search_$1" = "none required" || LIBS="$ac_cv_search_$1 $LIBS"
  $3],
      [$4])dnl
])



# AC_CHECK_LIB(LIBRARY, FUNCTION,
#              [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#              [OTHER-LIBRARIES])
# ------------------------------------------------------
#
# Use a cache variable name containing both the library and function name,
# because the test really is for library $1 defining function $2, not
# just for library $1.  Separate tests with the same $1 and different $2s
# may have different results.
#
# Note that using directly AS_VAR_PUSHDEF([ac_Lib], [ac_cv_lib_$1_$2])
# is asking for troubles, since AC_CHECK_LIB($lib, fun) would give
# ac_cv_lib_$lib_fun, which is definitely not what was meant.  Hence
# the AS_LITERAL_IF indirection.
#
# FIXME: This macro is extremely suspicious.  It DEFINEs unconditionnally,
# whatever the FUNCTION, in addition to not being a *S macro.  Note
# that the cache does depend upon the function we are looking for.
#
# It is on purpose we used `ac_check_lib_save_LIBS' and not just
# `ac_save_LIBS': there are many macros which don't want to see `LIBS'
# changed but still want to use AC_CHECK_LIB, so they save `LIBS'.
# And ``ac_save_LIBS' is too tempting a name, so let's leave them some
# freedom.
AC_DEFUN([AC_CHECK_LIB],
[m4_ifval([$3], , [AH_CHECK_LIB([$1])])dnl
AS_LITERAL_IF([$1],
              [AS_VAR_PUSHDEF([ac_Lib], [ac_cv_lib_$1_$2])],
              [AS_VAR_PUSHDEF([ac_Lib], [ac_cv_lib_$1''_$2])])dnl
AC_CACHE_CHECK([for $2 in -l$1], ac_Lib,
[ac_check_lib_save_LIBS=$LIBS
LIBS="-l$1 $5 $LIBS"
AC_TRY_LINK_FUNC([$2],
                 [AS_VAR_SET(ac_Lib, yes)],
                 [AS_VAR_SET(ac_Lib, no)])
LIBS=$ac_check_lib_save_LIBS])
AS_IF([test AS_VAR_GET(ac_Lib) = yes],
      [m4_default([$3], [AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_LIB$1))
  LIBS="-l$1 $LIBS"
])],
      [$4])dnl
AS_VAR_POPDEF([ac_Lib])dnl
])# AC_CHECK_LIB


# AH_CHECK_LIB(LIBNAME)
# ---------------------
m4_define([AH_CHECK_LIB],
[AH_TEMPLATE(AS_TR_CPP(HAVE_LIB$1),
             [Define if you have the `]$1[' library (-l]$1[).])])


# AC_HAVE_LIBRARY(LIBRARY,
#                 [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#                 [OTHER-LIBRARIES])
# ---------------------------------------------------------
#
# This macro is equivalent to calling `AC_CHECK_LIB' with a FUNCTION
# argument of `main'.  In addition, LIBRARY can be written as any of
# `foo', `-lfoo', or `libfoo.a'.  In all of those cases, the compiler
# is passed `-lfoo'.  However, LIBRARY cannot be a shell variable;
# it must be a literal name.
AU_DEFUN([AC_HAVE_LIBRARY],
[m4_pushdef([AC_Lib_Name],
            m4_patsubst(m4_patsubst([[$1]],
                                    [lib\([^\.]*\)\.a], [\1]),
                        [-l], []))dnl
AC_CHECK_LIB(AC_Lib_Name, main, [$2], [$3], [$4])dnl
ac_cv_lib_[]AC_Lib_Name()=ac_cv_lib_[]AC_Lib_Name()_main
m4_popdef([AC_Lib_Name])dnl
])



## ------------------------ ##
## Examining declarations.  ##
## ------------------------ ##



# _AC_PREPROC_IFELSE(PROGRAM, [ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ----------------------------------------------------------------
# Try to preprocess PROGRAM.
#
# This macro can be used during the selection of a preprocessor.
# Run cpp and set ac_cpp_err to "yes" for an error, to
# "$ac_(c,cxx)_preproc_warn_flag" if there are warnings or to "" if
# neither warnings nor errors have been detected.  eval is necessary
# to expand ac_cpp.
AC_DEFUN([_AC_PREPROC_IFELSE],
[m4_ifvaln([$1], [AC_LANG_CONFTEST([$1])])dnl
if _AC_EVAL_STDERR([$ac_cpp conftest.$ac_ext]) >/dev/null; then
  if test -s conftest.err; then
    ac_cpp_err=$ac_[]_AC_LANG_ABBREV[]_preproc_warn_flag
  else
    ac_cpp_err=
  fi
else
  ac_cpp_err=yes
fi
if test -z "$ac_cpp_err"; then
  m4_default([$2], :)
else
  echo "$as_me: failed program was:" >&AS_MESSAGE_LOG_FD
  cat conftest.$ac_ext >&AS_MESSAGE_LOG_FD
  $3
fi
rm -f conftest.err m4_ifval([$1], [conftest.$ac_ext])[]dnl
])# _AC_PREPROC_IFELSE


# AC_PREPROC_IFELSE(PROGRAM, [ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ---------------------------------------------------------------
# Try to preprocess PROGRAM.  Requires that the preprocessor for the
# current language was checked for, hence do not use this macro in macros
# looking for a preprocessor.
AC_DEFUN([AC_PREPROC_IFELSE],
[AC_LANG_PREPROC_REQUIRE()dnl
_AC_PREPROC_IFELSE($@)])


# AC_TRY_CPP(INCLUDES, [ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ---------------------------------------------------------
# AC_TRY_CPP is used to check whether particular header files exist.
# (But it actually tests whether INCLUDES produces no CPP errors.)
#
# INCLUDES are not defaulted and are double quoted.
AC_DEFUN([AC_TRY_CPP],
[AC_PREPROC_IFELSE([AC_LANG_SOURCE([[$1]])], [$2], [$3])])


# AC_EGREP_CPP(PATTERN, PROGRAM,
#              [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# ------------------------------------------------------
# Because this macro is used by AC_PROG_GCC_TRADITIONAL, which must
# come early, it is not included in AC_BEFORE checks.
AC_DEFUN([AC_EGREP_CPP],
[AC_LANG_PREPROC_REQUIRE()dnl
AC_LANG_CONFTEST([AC_LANG_SOURCE([[$2]])])
dnl eval is necessary to expand ac_cpp.
dnl Ultrix and Pyramid sh refuse to redirect output of eval, so use subshell.
if (eval "$ac_cpp conftest.$ac_ext") 2>&AS_MESSAGE_LOG_FD |
dnl Quote $1 to prevent m4 from eating character classes
  egrep "[$1]" >/dev/null 2>&1; then
  m4_default([$3], :)
m4_ifvaln([$4], [else
  $4])dnl
fi
rm -f conftest*
])# AC_EGREP_CPP


# AC_EGREP_HEADER(PATTERN, HEADER-FILE,
#                 [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# ---------------------------------------------------------
AC_DEFUN([AC_EGREP_HEADER],
[AC_EGREP_CPP([$1],
[#include <$2>
], [$3], [$4])])




## ------------------ ##
## Examining syntax.  ##
## ------------------ ##


# _AC_COMPILE_IFELSE(PROGRAM, [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# ---------------------------------------------------------------------
# Try to compile PROGRAM.
# This macro can be used during the selection of a compiler.
m4_define([_AC_COMPILE_IFELSE],
[m4_ifvaln([$1], [AC_LANG_CONFTEST([$1])])dnl
rm -f conftest.$ac_objext
AS_IF([AC_TRY_EVAL(ac_compile) &&
         AC_TRY_COMMAND([test -s conftest.$ac_objext])],
      [$2],
      [echo "$as_me: failed program was:" >&AS_MESSAGE_LOG_FD
cat conftest.$ac_ext >&AS_MESSAGE_LOG_FD
m4_ifvaln([$3],[$3])dnl])dnl
rm -f conftest.$ac_objext m4_ifval([$1], [conftest.$ac_ext])[]dnl
])# _AC_COMPILE_IFELSE


# AC_COMPILE_IFELSE(PROGRAM, [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# --------------------------------------------------------------------
# Try to compile PROGRAM.  Requires that the compiler for the current
# language was checked for, hence do not use this macro in macros looking
# for a compiler.
AC_DEFUN([AC_COMPILE_IFELSE],
[AC_LANG_COMPILER_REQUIRE()dnl
_AC_COMPILE_IFELSE($@)])


# AC_TRY_COMPILE(INCLUDES, FUNCTION-BODY,
#                [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# --------------------------------------------------------
AC_DEFUN([AC_TRY_COMPILE],
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[$1]], [[$2]])], [$3], [$4])])



## --------------------- ##
## Examining libraries.  ##
## --------------------- ##


# _AC_LINK_IFELSE(PROGRAM, [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# ------------------------------------------------------------------
# Try to link PROGRAM.
# This macro can be used during the selection of a compiler.
m4_define([_AC_LINK_IFELSE],
[m4_ifvaln([$1], [AC_LANG_CONFTEST([$1])])dnl
rm -f conftest.$ac_objext conftest$ac_exeext
AS_IF([AC_TRY_EVAL(ac_link) &&
         AC_TRY_COMMAND([test -s conftest$ac_exeext])],
      [$2],
      [echo "$as_me: failed program was:" >&AS_MESSAGE_LOG_FD
cat conftest.$ac_ext >&AS_MESSAGE_LOG_FD
m4_ifvaln([$3], [$3])dnl])[]dnl
rm -f conftest.$ac_objext conftest$ac_exeext m4_ifval([$1], [conftest.$ac_ext])[]dnl
])# _AC_LINK_IFELSE


# AC_LINK_IFELSE(PROGRAM, [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# -----------------------------------------------------------------
# Try to link PROGRAM.  Requires that the compiler for the current
# language was checked for, hence do not use this macro in macros looking
# for a compiler.
AC_DEFUN([AC_LINK_IFELSE],
[AC_LANG_COMPILER_REQUIRE()dnl
_AC_LINK_IFELSE($@)])


# AC_TRY_LINK(INCLUDES, FUNCTION-BODY,
#             [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# -----------------------------------------------------
# Should the INCLUDES be defaulted here?
# Contrarily to AC_LINK_IFELSE, this macro double quote its first two args.
# FIXME: WARNING: The code to compile was different in the case of
# Fortran between AC_TRY_COMPILE and AC_TRY_LINK, though they should
# equivalent as far as I can tell from the semantics and the docs.  In
# the former, $[2] is used as is, in the latter, it is `call' ed.
# Remove these FIXME: once truth established.
AC_DEFUN([AC_TRY_LINK],
[AC_LINK_IFELSE([AC_LANG_PROGRAM([[$1]], [[$2]])], [$3], [$4])])


# AC_COMPILE_CHECK(ECHO-TEXT, INCLUDES, FUNCTION-BODY,
#                  ACTION-IF-FOUND, [ACTION-IF-NOT-FOUND])
# --------------------------------------------------------
AU_DEFUN([AC_COMPILE_CHECK],
[m4_ifvaln([$1], [AC_CHECKING([for $1])])dnl
AC_LINK_IFELSE([AC_LANG_PROGRAM([[$2]], [[$3]])], [$4], [$5])
])




## -------------------------------- ##
## Checking for run-time features.  ##
## -------------------------------- ##


# _AC_RUN_IFELSE(PROGRAM, [ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ------------------------------------------------------------
# Compile, link, and run.
# This macro can be used during the selection of a compiler.
# We also remove conftest.o as if the compilation fails, some compilers
# don't remove it.
m4_define([_AC_RUN_IFELSE],
[m4_ifvaln([$1], [AC_LANG_CONFTEST([$1])])dnl
rm -f conftest$ac_exeext
AS_IF([AC_TRY_EVAL(ac_link) && AC_TRY_COMMAND(./conftest$ac_exeext)],
      [$2],
      [echo "$as_me: program exited with status $ac_status" >&AS_MESSAGE_LOG_FD
echo "$as_me: failed program was:" >&AS_MESSAGE_LOG_FD
cat conftest.$ac_ext >&AS_MESSAGE_LOG_FD
m4_ifvaln([$3], [$3])dnl])[]dnl
rm -f core core.* *.core conftest$ac_exeext conftest.$ac_objext m4_ifval([$1],
                                                     [conftest.$ac_ext])[]dnl
])# _AC_RUN_IFELSE


# AC_RUN_IFELSE(PROGRAM,
#               [ACTION-IF-TRUE], [ACTION-IF-FALSE],
#               [ACTION-IF-CROSS-COMPILING = RUNTIME-ERROR])
# ----------------------------------------------------------
# Compile, link, and run. Requires that the compiler for the current
# language was checked for, hence do not use this macro in macros looking
# for a compiler.
AC_DEFUN([AC_RUN_IFELSE],
[AC_LANG_COMPILER_REQUIRE()dnl
m4_ifval([$4], [],
         [AC_DIAGNOSE([cross],
                     [$0 called without default to allow cross compiling])])dnl
if test "$cross_compiling" = yes; then
  m4_default([$4],
             [AC_MSG_ERROR([cannot run test program while cross compiling])])
else
  _AC_RUN_IFELSE($@)
fi])


# AC_TRY_RUN(PROGRAM,
#            [ACTION-IF-TRUE], [ACTION-IF-FALSE],
#            [ACTION-IF-CROSS-COMPILING = RUNTIME-ERROR])
# --------------------------------------------------------
AC_DEFUN([AC_TRY_RUN],
[AC_RUN_IFELSE([AC_LANG_SOURCE([[$1]])], [$2], [$3], [$4])])



## ------------------------------------- ##
## Checking for the existence of files.  ##
## ------------------------------------- ##

# AC_CHECK_FILE(FILE, [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# -------------------------------------------------------------
#
# Check for the existence of FILE.
AC_DEFUN([AC_CHECK_FILE],
[AC_DIAGNOSE([cross],
             [cannot check for file existence when cross compiling])dnl
AS_VAR_PUSHDEF([ac_File], [ac_cv_file_$1])dnl
AC_CACHE_CHECK([for $1], ac_File,
[test "$cross_compiling" = yes &&
  AC_MSG_ERROR([cannot check for file existence when cross compiling])
if test -r "$1"; then
  AS_VAR_SET(ac_File, yes)
else
  AS_VAR_SET(ac_File, no)
fi])
AS_IF([test AS_VAR_GET(ac_File) = yes], [$2], [$3])[]dnl
AS_VAR_POPDEF([ac_File])dnl
])# AC_CHECK_FILE


# AC_CHECK_FILES(FILE..., [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# -----------------------------------------------------------------
AC_DEFUN([AC_CHECK_FILES],
[AC_FOREACH([AC_FILE_NAME], [$1],
  [AC_CHECK_FILE(AC_FILE_NAME,
                 [AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_[]AC_FILE_NAME), 1,
                                   [Define if you have the file `]AC_File['.])
$2],
                 [$3])])])


## ------------------------------- ##
## Checking for declared symbols.  ##
## ------------------------------- ##


# AC_CHECK_DECL(SYMBOL,
#               [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#               [INCLUDES])
# -------------------------------------------------------
# Check if SYMBOL (a variable or a function) is declared.
AC_DEFUN([AC_CHECK_DECL],
[AS_VAR_PUSHDEF([ac_Symbol], [ac_cv_have_decl_$1])dnl
AC_CACHE_CHECK([whether $1 is declared], ac_Symbol,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT([$4])],
[#ifndef $1
  char *p = (char *) $1;
#endif
])],
                   [AS_VAR_SET(ac_Symbol, yes)],
                   [AS_VAR_SET(ac_Symbol, no)])])
AS_IF([test AS_VAR_GET(ac_Symbol) = yes], [$2], [$3])[]dnl
AS_VAR_POPDEF([ac_Symbol])dnl
])# AC_CHECK_DECL


# AC_CHECK_DECLS(SYMBOLS,
#                [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#                [INCLUDES])
# --------------------------------------------------------
# Defines HAVE_DECL_SYMBOL to 1 if declared, 0 otherwise.  See the
# documentation for a detailed explanation of this difference with
# other AC_CHECK_*S macros.  SYMBOLS is an m4 list.
AC_DEFUN([AC_CHECK_DECLS],
[m4_foreach([AC_Symbol], [$1],
  [AC_CHECK_DECL(AC_Symbol,
                 [AC_DEFINE_UNQUOTED(AS_TR_CPP([HAVE_DECL_]AC_Symbol), 1,
                                     [Define to 1 if you have the declaration
                                     of `]AC_Symbol[', and to 0 if you don't.])
$2],
                 [AC_DEFINE_UNQUOTED(AS_TR_CPP([HAVE_DECL_]AC_Symbol), 0)
$3],
                 [$4])])
])# AC_CHECK_DECLS


## -------------------------------- ##
## Checking for library functions.  ##
## -------------------------------- ##


# AC_LIBSOURCE(FILENAME)
# ----------------------
# Announce we might need the file `FILENAME'.
m4_define([AC_LIBSOURCE], [])


# AC_LIBSOURCES([FILENAME1, ...])
# -------------------------------
# Announce we might need these files.
m4_define([AC_LIBSOURCES],
[m4_foreach([_AC_FILENAME], [$1],
            [AC_LIBSOURCE(_AC_FILENAME)])])


# _AC_LIBOBJ(FILENAME-NOEXT, ACTION-IF-INDIR)
# -------------------------------------------
# We need `FILENAME-NOEXT.o', save this into `LIBOBJS'.
# We don't use AC_SUBST/2 because it forces an unneeded eol.
m4_define([_AC_LIBOBJ],
[AS_LITERAL_IF([$1],
               [AC_LIBSOURCE([$1.c])],
               [$2])dnl
AC_SUBST([LIBOBJS])dnl
LIBOBJS="$LIBOBJS $1.$ac_objext"])


# AC_LIBOBJ(FILENAME-NOEXT)
# -------------------------
# We need `FILENAME-NOEXT.o', save this into `LIBOBJS'.
# We don't use AC_SUBST/2 because it forces an unneeded eol.
m4_define([AC_LIBOBJ],
[_AC_LIBOBJ([$1],
            [AC_DIAGNOSE(syntax,
                         [$0($1): you should use literals])])dnl
])



## ----------------------------------- ##
## Checking compiler characteristics.  ##
## ----------------------------------- ##


# _AC_COMPUTE_INT_COMPILE(EXPRESSION, VARIABLE, [INCLUDES])
# ---------------------------------------------------------
# Compute the integer EXPRESSION and store the result in the VARIABLE.
# Works OK if cross compiling.
m4_define([_AC_COMPUTE_INT_COMPILE],
[# Depending upon the size, compute the lo and hi bounds.
AC_COMPILE_IFELSE([AC_LANG_BOOL_COMPILE_TRY([$3], [($1) >= 0])],
 [ac_lo=0 ac_mid=0
  while :; do
    AC_COMPILE_IFELSE([AC_LANG_BOOL_COMPILE_TRY([$3], [($1) <= $ac_mid])],
                   [ac_hi=$ac_mid; break],
                   [ac_lo=`expr $ac_mid + 1`; ac_mid=`expr 2 '*' $ac_mid + 1`])
  done],
 [ac_hi=-1 ac_mid=-1
  while :; do
    AC_COMPILE_IFELSE([AC_LANG_BOOL_COMPILE_TRY([$3], [($1) >= $ac_mid])],
                      [ac_lo=$ac_mid; break],
                      [ac_hi=`expr $ac_mid - 1`; ac_mid=`expr 2 '*' $ac_mid`])
  done])
# Binary search between lo and hi bounds.
while test "x$ac_lo" != "x$ac_hi"; do
  ac_mid=`expr '(' $ac_hi - $ac_lo ')' / 2 + $ac_lo`
  AC_COMPILE_IFELSE([AC_LANG_BOOL_COMPILE_TRY([$3], [($1) <= $ac_mid])],
                     [ac_hi=$ac_mid], [ac_lo=`expr $ac_mid + 1`])
done
$2=$ac_lo[]dnl
])# _AC_COMPUTE_INT_COMPILE


# _AC_COMPUTE_INT_RUN(EXPRESSION, VARIABLE, [INCLUDES], [IF-FAILS])
# -----------------------------------------------------------------
# Store the evaluation of the integer EXPRESSION in VARIABLE.
m4_define([_AC_COMPUTE_INT_RUN],
[AC_RUN_IFELSE([AC_LANG_INT_SAVE([$3], [$1])],
               [$2=`cat conftest.val`], [$4])])


# _AC_COMPUTE_INT(EXPRESSION, VARIABLE, INCLUDES, IF-FAILS)
# ---------------------------------------------------------
m4_define([_AC_COMPUTE_INT],
[if test "$cross_compiling" = yes; then
  _AC_COMPUTE_INT_COMPILE([$1], [$2], [$3])
else
  _AC_COMPUTE_INT_RUN([$1], [$2], [$3], [$4])
fi
rm -f conftest.val[]dnl
])# _AC_COMPUTE_INT


## ----------------------- ##
## Creating output files.  ##
## ----------------------- ##


# This section handles about all the preparation aspects for
# `config.status': registering the configuration files, the headers,
# the links, and the commands `config.status' will run.  There is a
# little mixture though of things actually handled by `configure',
# such as running the `configure' in the sub directories.  Minor
# detail.
#
# There are two kinds of commands:
#
# COMMANDS:
#
#   They are output into `config.status' via a quoted here doc.  These
#   commands are always associated to a tag which the user can use to
#   tell `config.status' what are the commands she wants to run.
#
# INIT-CMDS:
#
#   They are output via an *unquoted* here-doc.  As a consequence $var
#   will be output as the value of VAR.  This is typically used by
#   `configure' to give `config,.status' some variables it needs to run
#   the COMMANDS.  At the difference of `COMMANDS', the INIT-CMDS are
#   always run.
#
#
# Some uniformity exists around here, please respect it!
#
# A macro named AC_CONFIG_FOOS has three args: the `TAG...' (or
# `FILE...'  when it applies), the `COMMANDS' and the `INIT-CMDS'.  It
# first checks that TAG was not registered elsewhere thanks to
# AC_CONFIG_UNIQUE.  Then it registers `TAG...' in AC_LIST_FOOS, and for
# each `TAG', a special line in AC_LIST_FOOS_COMMANDS which is used in
# `config.status' like this:
#
# 	  case $ac_tag in
# 	    AC_LIST_FOOS_COMMANDS
# 	  esac
#
# Finally, the `INIT-CMDS' are dumped into a special diversion, via
# `_AC_CONFIG_COMMANDS_INIT'.  While `COMMANDS' are output once per TAG,
# `INIT-CMDS' are dumped only once per call to AC_CONFIG_FOOS.
#
# It also leave the TAG in the shell variable ac_config_foo which contains
# those which will actually be executed.  In other words:
#
#	if false; then
#	  AC_CONFIG_FOOS(bar, [touch bar])
#	fi
#
# will not create bar.
#
# AC_CONFIG_FOOS can be called several times (with different TAGs of
# course).
#
# Because these macros should not output anything, there should be `dnl'
# everywhere.  A pain my friend, a pain.  So instead in each macro we
# divert(-1) and restore the diversion at the end.
#
#
# Honorable members of this family are AC_CONFIG_FILES,
# AC_CONFIG_HEADERS, AC_CONFIG_LINKS and AC_CONFIG_COMMANDS.  Bad boys
# are AC_LINK_FILES, AC_OUTPUT_COMMANDS and AC_OUTPUT when used with
# arguments.  False members are AC_CONFIG_SRCDIR, AC_CONFIG_SUBDIRS
# and AC_CONFIG_AUX_DIR.  Cousins are AC_CONFIG_COMMANDS_PRE and
# AC_CONFIG_COMMANDS_POST.



# AC_CONFIG_IF_MEMBER(DEST, LIST, ACTION-IF-TRUE, ACTION-IF-FALSE)
# ----------------------------------------------------------------
# If DEST is member of LIST, expand to ACTION-IF-TRUE, else ACTION-IF-FALSE.
#
# LIST is an AC_CONFIG list, i.e., a list of DEST[:SOURCE], separated
# with spaces.
#
# FIXME: This macro is badly designed, but I'm not guilty: m4 is.  There
# is just no way to simply compare two strings in m4, but to use pattern
# matching.  The big problem is then that the active characters should
# be quoted.  Currently `+*.' are quoted.
m4_define([AC_CONFIG_IF_MEMBER],
[m4_if(m4_regexp($2, [\(^\| \)]m4_patsubst([$1],
                                           [\([+*.]\)], [\\\1])[\(:\| \|$\)]),
       -1, [$4], [$3])])


# AC_FILE_DEPENDENCY_TRACE(DEST, SOURCE1, [SOURCE2...])
# -----------------------------------------------------
# This macro does nothing, it's a hook to be read with `autoconf --trace'.
# It announces DEST depends upon the SOURCE1 etc.
m4_define([AC_FILE_DEPENDENCY_TRACE], [])


# _AC_CONFIG_DEPENDENCY(DEST, [SOURCE1], [SOURCE2...])
# ----------------------------------------------------
# Be sure that a missing dependency is expressed as a dependency upon
# `DEST.in'.
m4_define([_AC_CONFIG_DEPENDENCY],
[m4_ifval([$2],
          [AC_FILE_DEPENDENCY_TRACE($@)],
          [AC_FILE_DEPENDENCY_TRACE([$1], [$1.in])])])


# _AC_CONFIG_DEPENDENCIES(DEST[:SOURCE1[:SOURCE2...]]...)
# -------------------------------------------------------
# Declare the DESTs depend upon their SOURCE1 etc.
m4_define([_AC_CONFIG_DEPENDENCIES],
[m4_divert_push([KILL])
AC_FOREACH([AC_File], [$1],
  [_AC_CONFIG_DEPENDENCY(m4_patsubst(AC_File, [:], [,]))])
m4_divert_pop([KILL])dnl
])


# _AC_CONFIG_UNIQUE(DEST[:SOURCE]...)
# -----------------------------------
#
# Verify that there is no double definition of an output file
# (precisely, guarantees there is no common elements between
# CONFIG_HEADERS, CONFIG_FILES, CONFIG_LINKS, and CONFIG_SUBDIRS).
#
# Note that this macro does not check if the list $[1] itself
# contains doubles.
m4_define([_AC_CONFIG_UNIQUE],
[m4_divert_push([KILL])
AC_FOREACH([AC_File], [$1],
[m4_pushdef([AC_Dest], m4_patsubst(AC_File, [:.*]))
AC_CONFIG_IF_MEMBER(AC_Dest, [AC_LIST_HEADERS],
     [AC_FATAL(`AC_Dest' [is already registered with AC_CONFIG_HEADER or AC_CONFIG_HEADERS.])])
  AC_CONFIG_IF_MEMBER(AC_Dest, [AC_LIST_LINKS],
     [AC_FATAL(`AC_Dest' [is already registered with AC_CONFIG_LINKS.])])
  AC_CONFIG_IF_MEMBER(AC_Dest, [_AC_LIST_SUBDIRS],
     [AC_FATAL(`AC_Dest' [is already registered with AC_CONFIG_SUBDIRS.])])
  AC_CONFIG_IF_MEMBER(AC_Dest, [AC_LIST_COMMANDS],
     [AC_FATAL(`AC_Dest' [is already registered with AC_CONFIG_COMMANDS.])])
  AC_CONFIG_IF_MEMBER(AC_Dest, [AC_LIST_FILES],
     [AC_FATAL(`AC_Dest' [is already registered with AC_CONFIG_FILES or AC_OUTPUT.])])
m4_popdef([AC_Dest])])
m4_divert_pop([KILL])dnl
])


# _AC_CONFIG_COMMANDS_INIT([INIT-COMMANDS])
# -----------------------------------------
#
# Register INIT-COMMANDS as command pasted *unquoted* in
# `config.status'.  This is typically used to pass variables from
# `configure' to `config.status'.  Note that $[1] is not over quoted as
# was the case in AC_OUTPUT_COMMANDS.
m4_define([_AC_CONFIG_COMMANDS_INIT],
[m4_ifval([$1],
          [m4_append([_AC_OUTPUT_COMMANDS_INIT],
                     [$1
])])])

# Initialize.
m4_define([_AC_OUTPUT_COMMANDS_INIT])


# AC_CONFIG_COMMANDS(NAME...,[COMMANDS], [INIT-CMDS])
# ---------------------------------------------------
#
# Specify additional commands to be run by config.status.  This
# commands must be associated with a NAME, which should be thought
# as the name of a file the COMMANDS create.
AC_DEFUN([AC_CONFIG_COMMANDS],
[m4_divert_push([KILL])
_AC_CONFIG_UNIQUE([$1])
m4_append([AC_LIST_COMMANDS], [ $1])

m4_if([$2],,, [AC_FOREACH([AC_Name], [$1],
[m4_append([AC_LIST_COMMANDS_COMMANDS],
[    ]m4_patsubst(AC_Name, [:.*])[ ) $2 ;;
])])])
_AC_CONFIG_COMMANDS_INIT([$3])
m4_divert_pop([KILL])dnl
ac_config_commands="$ac_config_commands $1"
])dnl

# Initialize the lists.
m4_define([AC_LIST_COMMANDS])
m4_define([AC_LIST_COMMANDS_COMMANDS])


# AC_OUTPUT_COMMANDS(EXTRA-CMDS, INIT-CMDS)
# -----------------------------------------
#
# Add additional commands for AC_OUTPUT to put into config.status.
#
# This macro is an obsolete version of AC_CONFIG_COMMANDS.  The only
# difficulty in mapping AC_OUTPUT_COMMANDS to AC_CONFIG_COMMANDS is
# to give a unique key.  The scheme we have chosen is `default-1',
# `default-2' etc. for each call.
#
# Unfortunately this scheme is fragile: bad things might happen
# if you update an included file and configure.ac: you might have
# clashes :(  On the other hand, I'd like to avoid weird keys (e.g.,
# depending upon __file__ or the pid).
AU_DEFUN([AC_OUTPUT_COMMANDS],
[m4_define([_AC_OUTPUT_COMMANDS_CNT], m4_incr(_AC_OUTPUT_COMMANDS_CNT))dnl
dnl Double quoted since that was the case in the original macro.
AC_CONFIG_COMMANDS([default-]_AC_OUTPUT_COMMANDS_CNT, [[$1]], [[$2]])dnl
])

# Initialize.
AU_DEFUN([_AC_OUTPUT_COMMANDS_CNT], 0)


# AC_CONFIG_COMMANDS_PRE(CMDS)
# ----------------------------
# Commands to run right before config.status is created. Accumulates.
AC_DEFUN([AC_CONFIG_COMMANDS_PRE],
[m4_append([AC_OUTPUT_COMMANDS_PRE], [$1
])])

# Initialize.
m4_define([AC_OUTPUT_COMMANDS_PRE])


# AC_CONFIG_COMMANDS_POST(CMDS)
# -----------------------------
# Commands to run after config.status was created.  Accumulates.
AC_DEFUN([AC_CONFIG_COMMANDS_POST],
[m4_append([AC_OUTPUT_COMMANDS_POST], [$1
])])

# Initialize.
m4_define([AC_OUTPUT_COMMANDS_POST])


# AC_CONFIG_HEADERS(HEADERS..., [COMMANDS], [INIT-CMDS])
# ------------------------------------------------------
# Specify that the HEADERS are to be created by instantiation of the
# AC_DEFINEs.  Associate the COMMANDS to the HEADERS.  This macro
# accumulates if called several times.
#
# The commands are stored in a growing string AC_LIST_HEADERS_COMMANDS
# which should be used like this:
#
#      case $ac_file in
#        AC_LIST_HEADERS_COMMANDS
#      esac
AC_DEFUN([AC_CONFIG_HEADERS],
[m4_divert_push([KILL])
_AC_CONFIG_UNIQUE([$1])
_AC_CONFIG_DEPENDENCIES([$1])
m4_append([AC_LIST_HEADERS], [ $1])
dnl Register the commands
m4_ifval([$2], [AC_FOREACH([AC_File], [$1],
[m4_append([AC_LIST_HEADERS_COMMANDS],
[    ]m4_patsubst(AC_File, [:.*])[ ) $2 ;;
])])])
_AC_CONFIG_COMMANDS_INIT([$3])
m4_divert_pop([KILL])dnl
ac_config_headers="$ac_config_headers m4_normalize([$1])"
])dnl

# Initialize to empty.  It is much easier and uniform to have a config
# list expand to empty when undefined, instead of special casing when
# not defined (since in this case, AC_CONFIG_FOO expands to AC_CONFIG_FOO).
m4_define([AC_LIST_HEADERS])
m4_define([AC_LIST_HEADERS_COMMANDS])


# AC_CONFIG_HEADER(HEADER-TO-CREATE ...)
# --------------------------------------
# FIXME: Make it obsolete?
AC_DEFUN([AC_CONFIG_HEADER],
[AC_CONFIG_HEADERS([$1])])


# AC_CONFIG_LINKS(DEST:SOURCE..., [COMMANDS], [INIT-CMDS])
# --------------------------------------------------------
# Specify that config.status should establish a (symbolic if possible)
# link from TOP_SRCDIR/SOURCE to TOP_SRCDIR/DEST.
# Reject DEST=., because it is makes it hard for ./config.status
# to guess the links to establish (`./config.status .').
AC_DEFUN([AC_CONFIG_LINKS],
[m4_divert_push([KILL])
_AC_CONFIG_UNIQUE([$1])
_AC_CONFIG_DEPENDENCIES([$1])
m4_if(m4_regexp([$1], [^\.:\| \.:]), -1,,
      [AC_FATAL([$0: invalid destination: `.'])])
m4_append([AC_LIST_LINKS], [ $1])
dnl Register the commands
m4_ifval([$2], [AC_FOREACH([AC_File], [$1],
[m4_append([AC_LIST_LINKS_COMMANDS],
[    ]m4_patsubst(AC_File, [:.*])[ ) $2 ;;
])])])
_AC_CONFIG_COMMANDS_INIT([$3])
m4_divert_pop([KILL])dnl
ac_config_links="$ac_config_links m4_normalize([$1])"
])dnl


# Initialize the list.
m4_define([AC_LIST_LINKS])
m4_define([AC_LIST_LINKS_COMMANDS])


# AC_LINK_FILES(SOURCE..., DEST...)
# ---------------------------------
# Link each of the existing files SOURCE... to the corresponding
# link name in DEST...
#
# Unfortunately we can't provide a very good autoupdate service here,
# since in `AC_LINK_FILES($from, $to)' it is possible that `$from'
# and `$to' are actually lists.  It would then be completely wrong to
# replace it with `AC_CONFIG_LINKS($to:$from).  It is possible in the
# case of literal values though, but because I don't think there is any
# interest in creating config links with literal values, no special
# mechanism is implemented to handle them.
#
# _AC_LINK_CNT is used to be robust to multiple calls.
AU_DEFUN([AC_LINK_FILES],
[m4_if($#, 2, ,
       [m4_fatal([$0: incorrect number of arguments])])dnl
m4_define([_AC_LINK_FILES_CNT], m4_incr(_AC_LINK_FILES_CNT))dnl
ac_sources="$1"
ac_dests="$2"
while test -n "$ac_sources"; do
  set $ac_dests; ac_dest=$[1]; shift; ac_dests=$[*]
  set $ac_sources; ac_source=$[1]; shift; ac_sources=$[*]
  [ac_config_links_]_AC_LINK_FILES_CNT="$[ac_config_links_]_AC_LINK_FILES_CNT $ac_dest:$ac_source"
done
AC_CONFIG_LINKS($[ac_config_links_]_AC_LINK_FILES_CNT)dnl
],
[
  It is technically impossible to `autoupdate' cleanly from AC_LINK_FILES
  to AC_CONFIG_FILES.  `autoupdate' provides a functional but inelegant
  update, you should probably tune the result yourself.])# AC_LINK_FILES


# Initialize.
AU_DEFUN([_AC_LINK_FILES_CNT], 0)



# AC_CONFIG_FILES(FILE..., [COMMANDS], [INIT-CMDS])
# -------------------------------------------------
# Specify output files, as with AC_OUTPUT, i.e., files that are
# configured with AC_SUBST.  Associate the COMMANDS to each FILE,
# i.e., when config.status creates FILE, run COMMANDS afterwards.
#
# The commands are stored in a growing string AC_LIST_FILES_COMMANDS
# which should be used like this:
#
#      case $ac_file in
#        AC_LIST_FILES_COMMANDS
#      esac
AC_DEFUN([AC_CONFIG_FILES],
[m4_divert_push([KILL])
_AC_CONFIG_UNIQUE([$1])
_AC_CONFIG_DEPENDENCIES([$1])
m4_append([AC_LIST_FILES], [ $1])
dnl Register the commands.
m4_ifval([$2], [AC_FOREACH([AC_File], [$1],
[m4_append([AC_LIST_FILES_COMMANDS],
[    ]m4_patsubst(AC_File, [:.*])[ ) $2 ;;
])])])
_AC_CONFIG_COMMANDS_INIT([$3])
m4_divert_pop([KILL])dnl
ac_config_files="$ac_config_files m4_normalize([$1])"
])dnl

# Initialize the lists.
m4_define([AC_LIST_FILES])
m4_define([AC_LIST_FILES_COMMANDS])


# AC_CONFIG_SUBDIRS(DIR ...)
# --------------------------
# We define two variables:
# - ac_subdirs_all
#   is built in the `default' section, and should contain *all*
#   the arguments of AC_CONFIG_SUBDIRS.  It is used for --help=recursive.
#   It makes no sense for arguments which are sh variables.
# - subdirs
#   which is built at runtime, so some of these dirs might not be
#   included, if for instance the user refused a part of the tree.
#   This is used in _AC_OUTPUT_SUBDIRS.
# _AC_LIST_SUBDIRS is used only for _AC_CONFIG_UNIQUE.
AC_DEFUN([AC_CONFIG_SUBDIRS],
[_AC_CONFIG_UNIQUE([$1])dnl
AC_REQUIRE([AC_CONFIG_AUX_DIR_DEFAULT])dnl
m4_append([_AC_LIST_SUBDIRS], [ $1])dnl
AS_LITERAL_IF([$1], [],
              [AC_DIAGNOSE(syntax, [$0: you should use literals])])
m4_divert_text([DEFAULTS],
               [ac_subdirs_all="$ac_subdirs_all m4_normalize([$1])"])
AC_SUBST(subdirs, "$subdirs $1")dnl
])

# Initialize the list.
m4_define([_AC_LIST_SUBDIRS])


# autoupdate::AC_OUTPUT([CONFIG_FILES...], [EXTRA-CMDS], [INIT-CMDS])
# -------------------------------------------------------------------
#
# If there are arguments given to AC_OUTPUT, dispatch them to the
# proper modern macros.
AU_DEFUN([AC_OUTPUT],
[m4_ifvaln([$1],
           [AC_CONFIG_FILES([$1])])dnl
m4_ifvaln([$2$3],
          [AC_CONFIG_COMMANDS(default, [[$2]], [[$3]])])dnl
[AC_OUTPUT]])


# AC_OUTPUT([CONFIG_FILES...], [EXTRA-CMDS], [INIT-CMDS])
# -------------------------------------------------------
# The big finish.
# Produce config.status, config.h, and links; and configure subdirs.
# The CONFIG_HEADERS are defined in the m4 variable AC_LIST_HEADERS.
# Pay special attention not to have too long here docs: some old
# shells die.  Unfortunately the limit is not known precisely...
m4_define([AC_OUTPUT],
[dnl Dispatch the extra arguments to their native macros.
m4_ifval([$1],
         [AC_CONFIG_FILES([$1])])dnl
m4_ifval([$2$3],
         [AC_CONFIG_COMMANDS(default, [$2], [$3])])dnl
m4_ifval([$1$2$3],
         [AC_DIAGNOSE([obsolete],
                      [$0 should be used without arguments.
You should run autoupdate.])])dnl
AC_CACHE_SAVE

test "x$prefix" = xNONE && prefix=$ac_default_prefix
# Let make expand exec_prefix.
test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'

# VPATH may cause trouble with some makes, so we remove $(srcdir),
# ${srcdir} and @srcdir@ from VPATH if srcdir is ".", strip leading and
# trailing colons and then remove the whole line if VPATH becomes empty
# (actually we leave an empty line to preserve line numbers).
if test "x$srcdir" = x.; then
  ac_vpsub=['/^[ 	]*VPATH[ 	]*=/{
s/:*\$(srcdir):*/:/;
s/:*\${srcdir}:*/:/;
s/:*@srcdir@:*/:/;
s/^\([^=]*=[ 	]*\):*/\1/;
s/:*$//;
s/^[^=]*=[ 	]*$//;
}']
fi

m4_ifset([AC_LIST_HEADERS], [DEFS=-DHAVE_CONFIG_H], [AC_OUTPUT_MAKE_DEFS()])

dnl Commands to run before creating config.status.
AC_OUTPUT_COMMANDS_PRE()dnl

: ${CONFIG_STATUS=./config.status}
ac_clean_files_save=$ac_clean_files
ac_clean_files="$ac_clean_files $CONFIG_STATUS"
_AC_OUTPUT_CONFIG_STATUS()dnl
ac_clean_files=$ac_clean_files_save

dnl Commands to run after config.status was created
AC_OUTPUT_COMMANDS_POST()dnl

# configure is writing to config.log, and then calls config.status.
# config.status does its own redirection, appending to config.log.
# Unfortunately, on DOS this fails, as config.log is still kept open
# by configure, so config.status won't be able to write to it; its
# output is simply discarded.  So we exec the FD to /dev/null,
# effectively closing config.log, so it can be properly (re)opened and
# appended to by config.status.  When coming back to configure, we
# need to make the FD available again.
if test "$no_create" != yes; then
  ac_cs_success=:
  exec AS_MESSAGE_LOG_FD>/dev/null
  $SHELL $CONFIG_STATUS || ac_cs_success=false
  exec AS_MESSAGE_LOG_FD>>config.log
  # Use ||, not &&, to avoid exiting from the if with $? = 1, which
  # would make configure fail if this is the last instruction.
  $ac_cs_success || AS_EXIT([1])
fi
dnl config.status should not do recursion.
AC_PROVIDE_IFELSE([AC_CONFIG_SUBDIRS], [_AC_OUTPUT_SUBDIRS()])dnl
])# AC_OUTPUT


# _AC_OUTPUT_CONFIG_STATUS
# ------------------------
# Produce config.status.  Called by AC_OUTPUT.
# Pay special attention not to have too long here docs: some old
# shells die.  Unfortunately the limit is not known precisely...
m4_define([_AC_OUTPUT_CONFIG_STATUS],
[AC_MSG_NOTICE([creating $CONFIG_STATUS])
cat >$CONFIG_STATUS <<_ACEOF
#! $SHELL
# Generated automatically by configure.
# Run this file to recreate the current configuration.
# Compiler output produced by configure, useful for debugging
# configure, is in config.log if it exists.

debug=false
SHELL=\${CONFIG_SHELL-$SHELL}
ac_cs_invocation="\$[0] \$[@]"

_ACEOF

cat >>$CONFIG_STATUS <<\_ACEOF
AS_SHELL_SANITIZE
exec AS_MESSAGE_FD>&1

_ACEOF

# Files that config.status was made for.
if test -n "$ac_config_files"; then
  echo "config_files=\"$ac_config_files\"" >>$CONFIG_STATUS
fi

if test -n "$ac_config_headers"; then
  echo "config_headers=\"$ac_config_headers\"" >>$CONFIG_STATUS
fi

if test -n "$ac_config_links"; then
  echo "config_links=\"$ac_config_links\"" >>$CONFIG_STATUS
fi

if test -n "$ac_config_commands"; then
  echo "config_commands=\"$ac_config_commands\"" >>$CONFIG_STATUS
fi

cat >>$CONFIG_STATUS <<\EOF

ac_cs_usage="\
\`$as_me' instantiates files from templates according to the
current configuration.

Usage: $[0] [[OPTIONS]] [[FILE]]...

  -h, --help       print this help, then exit
  -V, --version    print version number, then exit
  -d, --debug      don't remove temporary files
      --recheck    update $as_me by reconfiguring in the same conditions
m4_ifset([AC_LIST_FILES],
[[  --file=FILE[:TEMPLATE]
                   instantiate the configuration file FILE
]])dnl
m4_ifset([AC_LIST_HEADERS],
[[  --header=FILE[:TEMPLATE]
                   instantiate the configuration header FILE
]])dnl

m4_ifset([AC_LIST_FILES],
[Configuration files:
$config_files

])dnl
m4_ifset([AC_LIST_HEADERS],
[Configuration headers:
$config_headers

])dnl
m4_ifset([AC_LIST_LINKS],
[Configuration links:
$config_links

])dnl
m4_ifset([AC_LIST_COMMANDS],
[Configuration commands:
$config_commands

])dnl
Report bugs to <bug-autoconf@gnu.org>."
EOF

cat >>$CONFIG_STATUS <<EOF
ac_cs_version="\\
m4_ifset([AC_PACKAGE_NAME], [AC_PACKAGE_NAME ])config.status[]dnl
m4_ifset([AC_PACKAGE_VERSION], [ AC_PACKAGE_VERSION])
configured by [$]0, generated by GNU Autoconf AC_ACVERSION,
  with options \\"`echo "$ac_configure_args" | sed 's/[[\\""\`\$]]/\\\\&/g'`\\"

Copyright 1992, 1993, 1994, 1995, 1996, 1998, 1999, 2000, 2001
Free Software Foundation, Inc.
This config.status script is free software; the Free Software Foundation
gives unlimited permission to copy, distribute and modify it."
srcdir=$srcdir
AC_PROVIDE_IFELSE([AC_PROG_INSTALL],
[dnl Leave those double quotes here: this $INSTALL is evaluated in a
dnl here document, which might result in `INSTALL=/bin/install -c'.
INSTALL="$INSTALL"
])dnl
EOF

cat >>$CONFIG_STATUS <<\EOF
# If no file are specified by the user, then we need to provide default
# value.  By we need to know if files were specified by the user.
ac_need_defaults=:
while test $[#] != 0
do
  case $[1] in
  --*=*)
    ac_option=`expr "x$[1]" : 'x\([[^=]]*\)='`
    ac_optarg=`expr "x$[1]" : 'x[[^=]]*=\(.*\)'`
    shift
    set dummy "$ac_option" "$ac_optarg" ${1+"$[@]"}
    shift
    ;;
  -*);;
  *) # This is not an option, so the user has probably given explicit
     # arguments.
     ac_need_defaults=false;;
  esac

  case $[1] in
  # Handling of the options.
EOF
cat >>$CONFIG_STATUS <<EOF
  -recheck | --recheck | --rechec | --reche | --rech | --rec | --re | --r)
    echo "running $SHELL $[0] " $ac_configure_args " --no-create --no-recursion"
    exec $SHELL $[0] $ac_configure_args --no-create --no-recursion ;;
EOF
cat >>$CONFIG_STATUS <<\EOF
  --version | --vers* | -V )
    echo "$ac_cs_version"; exit 0 ;;
  --he | --h)
    # Conflict between --help and --header
    AC_MSG_ERROR([ambiguous option: $[1]
Try `$[0] --help' for more information.]);;
  --help | --hel | -h )
    echo "$ac_cs_usage"; exit 0 ;;
  --debug | --d* | -d )
    debug=: ;;
  --file | --fil | --fi | --f )
    shift
    CONFIG_FILES="$CONFIG_FILES $[1]"
    ac_need_defaults=false;;
  --header | --heade | --head | --hea )
    shift
    CONFIG_HEADERS="$CONFIG_HEADERS $[1]"
    ac_need_defaults=false;;

  # This is an error.
  -*) AC_MSG_ERROR([unrecognized option: $[1]
Try `$[0] --help' for more information.]) ;;

  *) ac_config_targets="$ac_config_targets $[1]" ;;

  esac
  shift
done

exec AS_MESSAGE_LOG_FD>>config.log
cat >&AS_MESSAGE_LOG_FD << _ACEOF

## ----------------------- ##
## Running config.status.  ##
## ----------------------- ##

This file was extended by $as_me m4_ifset([AC_PACKAGE_STRING],
                            [(AC_PACKAGE_STRING) ])AC_ACVERSION, executed with
  CONFIG_FILES    = $CONFIG_FILES
  CONFIG_HEADERS  = $CONFIG_HEADERS
  CONFIG_LINKS    = $CONFIG_LINKS
  CONFIG_COMMANDS = $CONFIG_COMMANDS
  > $ac_cs_invocation
on `(hostname || uname -n) 2>/dev/null | sed 1q`

_ACEOF
EOF

dnl We output the INIT-CMDS first for obvious reasons :)
m4_ifset([_AC_OUTPUT_COMMANDS_INIT],
[cat >>$CONFIG_STATUS <<EOF
#
# INIT-COMMANDS section.
#

_AC_OUTPUT_COMMANDS_INIT()
EOF])


dnl Issue this section only if there were actually config files.
dnl This checks if one of AC_LIST_HEADERS, AC_LIST_FILES, AC_LIST_COMMANDS,
dnl or AC_LIST_LINKS is set.
m4_ifval(AC_LIST_HEADERS()AC_LIST_LINKS()AC_LIST_FILES()AC_LIST_COMMANDS(),
[
cat >>$CONFIG_STATUS <<\EOF
for ac_config_target in $ac_config_targets
do
  case "$ac_config_target" in
  # Handling of arguments.
AC_FOREACH([AC_File], AC_LIST_FILES,
[  "m4_patsubst(AC_File, [:.*])" )dnl
 CONFIG_FILES="$CONFIG_FILES AC_File" ;;
])dnl
AC_FOREACH([AC_File], AC_LIST_LINKS,
[  "m4_patsubst(AC_File, [:.*])" )dnl
 CONFIG_LINKS="$CONFIG_LINKS AC_File" ;;
])dnl
AC_FOREACH([AC_File], AC_LIST_COMMANDS,
[  "m4_patsubst(AC_File, [:.*])" )dnl
 CONFIG_COMMANDS="$CONFIG_COMMANDS AC_File" ;;
])dnl
AC_FOREACH([AC_File], AC_LIST_HEADERS,
[  "m4_patsubst(AC_File, [:.*])" )dnl
 CONFIG_HEADERS="$CONFIG_HEADERS AC_File" ;;
])dnl
  *) AC_MSG_ERROR([invalid argument: $ac_config_target]);;
  esac
done

# If the user did not use the arguments to specify the items to instantiate,
# then the envvar interface is used.  Set only those that are not.
# We use the long form for the default assignment because of an extremely
# bizarre bug on SunOS 4.1.3.
if $ac_need_defaults; then
m4_ifset([AC_LIST_FILES],
[  test "${CONFIG_FILES+set}" = set || CONFIG_FILES=$config_files
])dnl
m4_ifset([AC_LIST_HEADERS],
[  test "${CONFIG_HEADERS+set}" = set || CONFIG_HEADERS=$config_headers
])dnl
m4_ifset([AC_LIST_LINKS],
[  test "${CONFIG_LINKS+set}" = set || CONFIG_LINKS=$config_links
])dnl
m4_ifset([AC_LIST_COMMANDS],
[  test "${CONFIG_COMMANDS+set}" = set || CONFIG_COMMANDS=$config_commands
])dnl
fi

AS_TMPDIR(cs)

EOF
])[]dnl m4_ifval

dnl The following four sections are in charge of their own here
dnl documenting into $CONFIG_STATUS.
m4_ifset([AC_LIST_FILES],    [_AC_OUTPUT_FILES()])dnl
m4_ifset([AC_LIST_HEADERS],  [_AC_OUTPUT_HEADERS()])dnl
m4_ifset([AC_LIST_LINKS],    [_AC_OUTPUT_LINKS()])dnl
m4_ifset([AC_LIST_COMMANDS], [_AC_OUTPUT_COMMANDS()])dnl

cat >>$CONFIG_STATUS <<\EOF

AS_EXIT(0)
EOF
chmod +x $CONFIG_STATUS
])# _AC_OUTPUT_CONFIG_STATUS


# AC_OUTPUT_MAKE_DEFS
# -------------------
# Set the DEFS variable to the -D options determined earlier.
# This is a subroutine of AC_OUTPUT.
# It is called inside configure, outside of config.status.
# Using a here document instead of a string reduces the quoting nightmare.
m4_define([AC_OUTPUT_MAKE_DEFS],
[[# Transform confdefs.h into DEFS.
# Protect against shell expansion while executing Makefile rules.
# Protect against Makefile macro expansion.
#
# If the first sed substitution is executed (which looks for macros that
# take arguments), then we branch to the quote section.  Otherwise,
# look for a macro that doesn't take arguments.
cat >confdef2opt.sed <<\EOF
t clear
: clear
s,^[ 	]*#[ 	]*define[ 	][ 	]*\([^ 	(][^ 	(]*([^)]*)\)[ 	]*\(.*\),-D\1=\2,g
t quote
s,^[ 	]*#[ 	]*define[ 	][ 	]*\([^ 	][^ 	]*\)[ 	]*\(.*\),-D\1=\2,g
t quote
d
: quote
s,[ 	`~#$^&*(){}\\|;'"<>?],\\&,g
s,\[,\\&,g
s,\],\\&,g
s,\$,$$,g
p
EOF
# We use echo to avoid assuming a particular line-breaking character.
# The extra dot is to prevent the shell from consuming trailing
# line-breaks from the sub-command output.  A line-break within
# single-quotes doesn't work because, if this script is created in a
# platform that uses two characters for line-breaks (e.g., DOS), tr
# would break.
ac_LF_and_DOT=`echo; echo .`
DEFS=`sed -n -f confdef2opt.sed confdefs.h | tr "$ac_LF_and_DOT" ' .'`
rm -f confdef2opt.sed
]])# AC_OUTPUT_MAKE_DEFS


# _AC_OUTPUT_FILES
# ----------------
# Do the variable substitutions to create the Makefiles or whatever.
# This is a subroutine of AC_OUTPUT.
#
# It has to send itself into $CONFIG_STATUS (eg, via here documents).
# Upon exit, no here document shall be opened.
m4_define([_AC_OUTPUT_FILES],
[cat >>$CONFIG_STATUS <<EOF

#
# CONFIG_FILES section.
#

# No need to generate the scripts if there are no CONFIG_FILES.
# This happens for instance when ./config.status config.h
if test -n "\$CONFIG_FILES"; then
  # Protect against being on the right side of a sed subst in config.status.
dnl Please, pay attention that this sed code depends a lot on the shape
dnl of the sed commands issued by AC_SUBST.  So if you change one, change
dnl the other too.
[  sed 's/,@/@@/; s/@,/@@/; s/,;t t\$/@;t t/; /@;t t\$/s/[\\\\&,]/\\\\&/g;
   s/@@/,@/; s/@@/@,/; s/@;t t\$/,;t t/' >\$tmp/subs.sed <<\\CEOF]
dnl These here document variables are unquoted when configure runs
dnl but quoted when config.status runs, so variables are expanded once.
dnl Insert the sed substitutions of variables.
_AC_SUBST_SED_PROGRAM()dnl
CEOF

EOF

  cat >>$CONFIG_STATUS <<\EOF
  # Split the substitutions into bite-sized pieces for seds with
  # small command number limits, like on Digital OSF/1 and HP-UX.
dnl One cannot portably go further than 100 commands because of HP-UX.
dnl Here, there are 2 cmd per line, and two cmd are added later.
  ac_max_sed_lines=48
  ac_sed_frag=1 # Number of current file.
  ac_beg=1 # First line for current file.
  ac_end=$ac_max_sed_lines # Line after last line for current file.
  ac_more_lines=:
  ac_sed_cmds=
  while $ac_more_lines; do
    if test $ac_beg -gt 1; then
      sed "1,${ac_beg}d; ${ac_end}q" $tmp/subs.sed >$tmp/subs.frag
    else
      sed "${ac_end}q" $tmp/subs.sed >$tmp/subs.frag
    fi
    if test ! -s $tmp/subs.frag; then
      ac_more_lines=false
    else
      # The purpose of the label and of the branching condition is to
      # speed up the sed processing (if there are no `@' at all, there
      # is no need to browse any of the substitutions).
      # These are the two extra sed commands mentioned above.
      (echo [':t
  /@[a-zA-Z_][a-zA-Z_0-9]*@/!b'] && cat $tmp/subs.frag) >$tmp/subs-$ac_sed_frag.sed
      if test -z "$ac_sed_cmds"; then
  	ac_sed_cmds="sed -f $tmp/subs-$ac_sed_frag.sed"
      else
  	ac_sed_cmds="$ac_sed_cmds | sed -f $tmp/subs-$ac_sed_frag.sed"
      fi
      ac_sed_frag=`expr $ac_sed_frag + 1`
      ac_beg=$ac_end
      ac_end=`expr $ac_end + $ac_max_sed_lines`
    fi
  done
  if test -z "$ac_sed_cmds"; then
    ac_sed_cmds=cat
  fi
fi # test -n "$CONFIG_FILES"

EOF
cat >>$CONFIG_STATUS <<\EOF
for ac_file in : $CONFIG_FILES; do test "x$ac_file" = x: && continue
  # Support "outfile[:infile[:infile...]]", defaulting infile="outfile.in".
  case $ac_file in
  - | *:- | *:-:* ) # input from stdin
        cat >$tmp/stdin
        ac_file_in=`echo "$ac_file" | sed 's,[[^:]]*:,,'`
        ac_file=`echo "$ac_file" | sed 's,:.*,,'` ;;
  *:* ) ac_file_in=`echo "$ac_file" | sed 's,[[^:]]*:,,'`
        ac_file=`echo "$ac_file" | sed 's,:.*,,'` ;;
  * )   ac_file_in=$ac_file.in ;;
  esac

  # Compute @srcdir@, @top_srcdir@, and @INSTALL@ for subdirectories.
  ac_dir=`AS_DIRNAME(["$ac_file"])`
  if test "$ac_dir" != "$ac_file" && test "$ac_dir" != .; then
    AS_MKDIR_P(["$ac_dir"])
    ac_dir_suffix="/`echo $ac_dir|sed 's,^\./,,'`"
    # A "../" for each directory in $ac_dir_suffix.
    ac_dots=`echo "$ac_dir_suffix" | sed 's,/[[^/]]*,../,g'`
  else
    ac_dir_suffix= ac_dots=
  fi

  case $srcdir in
  .)  ac_srcdir=.
      if test -z "$ac_dots"; then
         ac_top_srcdir=.
      else
         ac_top_srcdir=`echo $ac_dots | sed 's,/$,,'`
      fi ;;
  [[\\/]]* | ?:[[\\/]]* )
      ac_srcdir=$srcdir$ac_dir_suffix;
      ac_top_srcdir=$srcdir ;;
  *) # Relative path.
    ac_srcdir=$ac_dots$srcdir$ac_dir_suffix
    ac_top_srcdir=$ac_dots$srcdir ;;
  esac

AC_PROVIDE_IFELSE([AC_PROG_INSTALL],
[  case $INSTALL in
  [[\\/$]]* | ?:[[\\/]]* ) ac_INSTALL=$INSTALL ;;
  *) ac_INSTALL=$ac_dots$INSTALL ;;
  esac
])dnl

  if test x"$ac_file" != x-; then
    AC_MSG_NOTICE([creating $ac_file])
    rm -f "$ac_file"
  fi
  # Let's still pretend it is `configure' which instantiates (i.e., don't
  # use $as_me), people would be surprised to read:
  #    /* config.h.  Generated automatically by config.status.  */
  configure_input="Generated automatically from `echo $ac_file_in |
                                                 sed 's,.*/,,'` by configure."

  # First look for the input files in the build tree, otherwise in the
  # src tree.
  ac_file_inputs=`IFS=:
    for f in $ac_file_in; do
      case $f in
      -) echo $tmp/stdin ;;
      [[\\/$]]*)
         # Absolute (can't be DOS-style, as IFS=:)
         test -f "$f" || AC_MSG_ERROR([cannot find input file: $f])
         echo $f;;
      *) # Relative
         if test -f "$f"; then
           # Build tree
           echo $f
         elif test -f "$srcdir/$f"; then
           # Source tree
           echo $srcdir/$f
         else
           # /dev/null tree
           AC_MSG_ERROR([cannot find input file: $f])
         fi;;
      esac
    done` || AS_EXIT([1])
EOF
cat >>$CONFIG_STATUS <<EOF
dnl Neutralize VPATH when `$srcdir' = `.'.
  sed "$ac_vpsub
dnl Shell code in configure.ac might set extrasub.
dnl FIXME: do we really want to maintain this feature?
$extrasub
EOF
cat >>$CONFIG_STATUS <<\EOF
:t
[/@[a-zA-Z_][a-zA-Z_0-9]*@/!b]
s,@configure_input@,$configure_input,;t t
s,@srcdir@,$ac_srcdir,;t t
s,@top_srcdir@,$ac_top_srcdir,;t t
AC_PROVIDE_IFELSE([AC_PROG_INSTALL], [s,@INSTALL@,$ac_INSTALL,;t t
])dnl
dnl The parens around the eval prevent an "illegal io" in Ultrix sh.
" $ac_file_inputs | (eval "$ac_sed_cmds") >$tmp/out
  rm -f $tmp/stdin
dnl This would break Makefile dependencies.
dnl  if cmp -s $ac_file $tmp/out 2>/dev/null; then
dnl    echo "$ac_file is unchanged"
dnl   else
dnl     rm -f $ac_file
dnl    mv $tmp/out $ac_file
dnl  fi
  if test x"$ac_file" != x-; then
    mv $tmp/out $ac_file
  else
    cat $tmp/out
    rm -f $tmp/out
  fi

m4_ifset([AC_LIST_FILES_COMMANDS],
[  # Run the commands associated with the file.
  case $ac_file in
AC_LIST_FILES_COMMANDS()dnl
  esac
])dnl
done
EOF
])# _AC_OUTPUT_FILES


# _AC_OUTPUT_HEADERS
# ------------------
#
# Output the code which instantiates the `config.h' files from their
# `config.h.in'.
#
# This is a subroutine of _AC_OUTPUT_CONFIG_STATUS.  It has to send
# itself into $CONFIG_STATUS (eg, via here documents).  Upon exit, no
# here document shall be opened.
#
#
# The code produced used to be extremely costly: there are was a
# single sed script (n lines) handling both `#define' templates,
# `#undef' templates with trailing space, and `#undef' templates
# without trailing spaces.  The full script was run on each of the m
# lines of `config.h.in', i.e., about n x m.
#
# Now there are two scripts: `conftest.defines' for the `#define'
# templates, and `conftest.undef' for the `#undef' templates.
#
# Optimization 1.  It is incredibly costly to run two `#undef'
# scripts, so just remove trailing spaces first.  Removes about a
# third of the cost.
#
# Optimization 2.  Since `#define' are rare and obsoleted,
# `conftest.defines' is built and run only if grep says there are
# `#define'.  Improves by at least a factor 2, since in addition we
# avoid the cost of *producing* the sed script.
#
# Optimization 3.  In each script, first check that the current input
# line is a template.  This avoids running the full sed script on
# empty lines and comments (divides the cost by about 3 since each
# template chunk is typically a comment, a template, an empty line).
#
# Optimization 4.  Once a substitution performed, since there can be
# only one per line, immediately restart the script on the next input
# line (using the `t' sed instruction).  Divides by about 2.
# *Note:* In the case of the AC_SUBST sed script (_AC_OUTPUT_FILES)
# this optimization cannot be applied as is, because there can be
# several substitutions per line.
#
#
# The result is about, hm, ... times blah... plus....  Ahem.  The
# result is about much faster.
m4_define([_AC_OUTPUT_HEADERS],
[cat >>$CONFIG_STATUS <<\EOF

#
# CONFIG_HEADER section.
#

# These sed commands are passed to sed as "A NAME B NAME C VALUE D", where
# NAME is the cpp macro being defined and VALUE is the value it is being given.
#
# ac_d sets the value in "#define NAME VALUE" lines.
dnl Double quote for the `[ ]' and `define'.
[ac_dA='s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)'
ac_dB='[ 	].*$,\1#\2'
ac_dC=' '
ac_dD=',;t'
# ac_u turns "#undef NAME" without trailing blanks into "#define NAME VALUE".
ac_uA='s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)'
ac_uB='$,\1#\2define\3'
ac_uC=' '
ac_uD=',;t']

for ac_file in : $CONFIG_HEADERS; do test "x$ac_file" = x: && continue
  # Support "outfile[:infile[:infile...]]", defaulting infile="outfile.in".
  case $ac_file in
  - | *:- | *:-:* ) # input from stdin
        cat >$tmp/stdin
        ac_file_in=`echo "$ac_file" | sed 's,[[^:]]*:,,'`
        ac_file=`echo "$ac_file" | sed 's,:.*,,'` ;;
  *:* ) ac_file_in=`echo "$ac_file" | sed 's,[[^:]]*:,,'`
        ac_file=`echo "$ac_file" | sed 's,:.*,,'` ;;
  * )   ac_file_in=$ac_file.in ;;
  esac

  test x"$ac_file" != x- && AC_MSG_NOTICE([creating $ac_file])

  # First look for the input files in the build tree, otherwise in the
  # src tree.
  ac_file_inputs=`IFS=:
    for f in $ac_file_in; do
      case $f in
      -) echo $tmp/stdin ;;
      [[\\/$]]*)
         # Absolute (can't be DOS-style, as IFS=:)
         test -f "$f" || AC_MSG_ERROR([cannot find input file: $f])
         echo $f;;
      *) # Relative
         if test -f "$f"; then
           # Build tree
           echo $f
         elif test -f "$srcdir/$f"; then
           # Source tree
           echo $srcdir/$f
         else
           # /dev/null tree
           AC_MSG_ERROR([cannot find input file: $f])
         fi;;
      esac
    done` || AS_EXIT([1])
  # Remove the trailing spaces.
  sed 's/[[ 	]]*$//' $ac_file_inputs >$tmp/in

EOF

# Transform confdefs.h into two sed scripts, `conftest.defines' and
# `conftest.undefs', that substitutes the proper values into
# config.h.in to produce config.h.  The first handles `#define'
# templates, and the second `#undef' templates.
# And first: Protect against being on the right side of a sed subst in
# config.status.  Protect against being in an unquoted here document
# in config.status.
rm -f conftest.defines conftest.undefs
# Using a here document instead of a string reduces the quoting nightmare.
# Putting comments in sed scripts is not portable.
#
# `end' is used to avoid that the second main sed command (meant for
# 0-ary CPP macros) applies to n-ary macro definitions.
# See the Autoconf documentation for `clear'.
cat >confdef2sed.sed <<\EOF
dnl Double quote for `[ ]' and `define'.
[s/[\\&,]/\\&/g
s,[\\$`],\\&,g
t clear
: clear
s,^[ 	]*#[ 	]*define[ 	][ 	]*\(\([^ 	(][^ 	(]*\)([^)]*)\)[ 	]*\(.*\)$,${ac_dA}\2${ac_dB}\1${ac_dC}\3${ac_dD},gp
t end
s,^[ 	]*#[ 	]*define[ 	][ 	]*\([^ 	][^ 	]*\)[ 	]*\(.*\)$,${ac_dA}\1${ac_dB}\1${ac_dC}\2${ac_dD},gp
: end]
EOF
# If some macros were called several times there might be several times
# the same #defines, which is useless.  Nevertheless, we may not want to
# sort them, since we want the *last* AC-DEFINE to be honored.
uniq confdefs.h | sed -n -f confdef2sed.sed >conftest.defines
sed 's/ac_d/ac_u/g' conftest.defines >conftest.undefs
rm -f confdef2sed.sed

# This sed command replaces #undef with comments.  This is necessary, for
# example, in the case of _POSIX_SOURCE, which is predefined and required
# on some systems where configure will not decide to define it.
cat >>conftest.undefs <<\EOF
[s,^[ 	]*#[ 	]*undef[ 	][ 	]*[a-zA-Z_][a-zA-Z_0-9]*,/* & */,]
EOF

# Break up conftest.defines because some shells have a limit on the size
# of here documents, and old seds have small limits too (100 cmds).
echo '  # Handle all the #define templates only if necessary.' >>$CONFIG_STATUS
echo '  if egrep ["^[ 	]*#[ 	]*define"] $tmp/in >/dev/null; then' >>$CONFIG_STATUS
echo '  # If there are no defines, we may have an empty if/fi' >>$CONFIG_STATUS
echo '  :' >>$CONFIG_STATUS
rm -f conftest.tail
while grep . conftest.defines >/dev/null
do
  # Write a limited-size here document to $tmp/defines.sed.
  echo '  cat >$tmp/defines.sed <<CEOF' >>$CONFIG_STATUS
  # Speed up: don't consider the non `#define' lines.
  echo ['/^[ 	]*#[ 	]*define/!b'] >>$CONFIG_STATUS
  # Work around the forget-to-reset-the-flag bug.
  echo 't clr' >>$CONFIG_STATUS
  echo ': clr' >>$CONFIG_STATUS
  sed ${ac_max_here_lines}q conftest.defines >>$CONFIG_STATUS
  echo 'CEOF
  sed -f $tmp/defines.sed $tmp/in >$tmp/out
  rm -f $tmp/in
  mv $tmp/out $tmp/in
' >>$CONFIG_STATUS
  sed 1,${ac_max_here_lines}d conftest.defines >conftest.tail
  rm -f conftest.defines
  mv conftest.tail conftest.defines
done
rm -f conftest.defines
echo '  fi # egrep' >>$CONFIG_STATUS
echo >>$CONFIG_STATUS

# Break up conftest.undefs because some shells have a limit on the size
# of here documents, and old seds have small limits too (100 cmds).
echo '  # Handle all the #undef templates' >>$CONFIG_STATUS
rm -f conftest.tail
while grep . conftest.undefs >/dev/null
do
  # Write a limited-size here document to $tmp/undefs.sed.
  echo '  cat >$tmp/undefs.sed <<CEOF' >>$CONFIG_STATUS
  # Speed up: don't consider the non `#undef'
  echo ['/^[ 	]*#[ 	]*undef/!b'] >>$CONFIG_STATUS
  # Work around the forget-to-reset-the-flag bug.
  echo 't clr' >>$CONFIG_STATUS
  echo ': clr' >>$CONFIG_STATUS
  sed ${ac_max_here_lines}q conftest.undefs >>$CONFIG_STATUS
  echo 'CEOF
  sed -f $tmp/undefs.sed $tmp/in >$tmp/out
  rm -f $tmp/in
  mv $tmp/out $tmp/in
' >>$CONFIG_STATUS
  sed 1,${ac_max_here_lines}d conftest.undefs >conftest.tail
  rm -f conftest.undefs
  mv conftest.tail conftest.undefs
done
rm -f conftest.undefs

dnl Now back to your regularly scheduled config.status.
cat >>$CONFIG_STATUS <<\EOF
  # Let's still pretend it is `configure' which instantiates (i.e., don't
  # use $as_me), people would be surprised to read:
  #    /* config.h.  Generated automatically by config.status.  */
  if test x"$ac_file" = x-; then
    echo "/* Generated automatically by configure.  */" >$tmp/config.h
  else
    echo "/* $ac_file.  Generated automatically by configure.  */" >$tmp/config.h
  fi
  cat $tmp/in >>$tmp/config.h
  rm -f $tmp/in
  if test x"$ac_file" != x-; then
    if cmp -s $ac_file $tmp/config.h 2>/dev/null; then
      AC_MSG_NOTICE([$ac_file is unchanged])
    else
      ac_dir=`AS_DIRNAME(["$ac_file"])`
      if test "$ac_dir" != "$ac_file" && test "$ac_dir" != .; then
        AS_MKDIR_P(["$ac_dir"])
      fi
      rm -f $ac_file
      mv $tmp/config.h $ac_file
    fi
  else
    cat $tmp/config.h
    rm -f $tmp/config.h
  fi
m4_ifset([AC_LIST_HEADERS_COMMANDS],
[  # Run the commands associated with the file.
  case $ac_file in
AC_LIST_HEADERS_COMMANDS()dnl
  esac
])dnl
done
EOF
])# _AC_OUTPUT_HEADERS


# _AC_OUTPUT_LINKS
# ----------------
# This is a subroutine of AC_OUTPUT.
#
# It has to send itself into $CONFIG_STATUS (eg, via here documents).
# Upon exit, no here document shall be opened.
m4_define([_AC_OUTPUT_LINKS],
[cat >>$CONFIG_STATUS <<\EOF

#
# CONFIG_LINKS section.
#

dnl Here we use : instead of .. because if AC_LINK_FILES was used
dnl with empty parameters (as in gettext.m4), then we obtain here
dnl `:', which we want to skip.  So let's keep a single exception: `:'.
for ac_file in : $CONFIG_LINKS; do test "x$ac_file" = x: && continue
  ac_dest=`echo "$ac_file" | sed 's,:.*,,'`
  ac_source=`echo "$ac_file" | sed 's,[[^:]]*:,,'`

  AC_MSG_NOTICE([linking $srcdir/$ac_source to $ac_dest])

  if test ! -r $srcdir/$ac_source; then
    AC_MSG_ERROR([$srcdir/$ac_source: File not found])
  fi
  rm -f $ac_dest

  # Make relative symlinks.
  ac_dest_dir=`AS_DIRNAME(["$ac_dest"])`
  if test "$ac_dest_dir" != "$ac_dest" && test "$ac_dest_dir" != .; then
    AS_MKDIR_P(["$ac_dest_dir"])
    ac_dest_dir_suffix="/`echo $ac_dest_dir|sed 's,^\./,,'`"
    # A "../" for each directory in $ac_dest_dir_suffix.
    ac_dots=`echo $ac_dest_dir_suffix|sed 's,/[[^/]]*,../,g'`
  else
    ac_dest_dir_suffix= ac_dots=
  fi

  case $srcdir in
  [[\\/$]]* | ?:[[\\/]]* ) ac_rel_source=$srcdir/$ac_source ;;
      *) ac_rel_source=$ac_dots$srcdir/$ac_source ;;
  esac

  # Make a symlink if possible; otherwise try a hard link.
  ln -s $ac_rel_source $ac_dest 2>/dev/null ||
    ln $srcdir/$ac_source $ac_dest ||
    AC_MSG_ERROR([cannot link $ac_dest to $srcdir/$ac_source])
m4_ifset([AC_LIST_LINKS_COMMANDS],
[  # Run the commands associated with the file.
  case $ac_file in
AC_LIST_LINKS_COMMANDS()dnl
  esac
])dnl
done
EOF
])# _AC_OUTPUT_LINKS


# _AC_OUTPUT_COMMANDS
# -------------------
# This is a subroutine of AC_OUTPUT, in charge of issuing the code
# related to AC_CONFIG_COMMANDS.
#
# It has to send itself into $CONFIG_STATUS (eg, via here documents).
# Upon exit, no here document shall be opened.
m4_define([_AC_OUTPUT_COMMANDS],
[cat >>$CONFIG_STATUS <<\EOF

#
# CONFIG_COMMANDS section.
#
for ac_file in : $CONFIG_COMMANDS; do test "x$ac_file" = x: && continue
  ac_dest=`echo "$ac_file" | sed 's,:.*,,'`
  ac_source=`echo "$ac_file" | sed 's,[[^:]]*:,,'`

dnl FIXME: Until Automake uses the new features of config.status, we
dnl should keep this silent.  Otherwise, because Automake runs this in
dnl each directory, it quickly becomes annoying.
dnl  echo "executing commands of $ac_dest"
  case $ac_dest in
AC_LIST_COMMANDS_COMMANDS()dnl
  esac
done
EOF
])# _AC_OUTPUT_COMMANDS


# _AC_OUTPUT_SUBDIRS
# ------------------
# This is a subroutine of AC_OUTPUT, but it does not go into
# config.status, rather, it is called after running config.status.
m4_define([_AC_OUTPUT_SUBDIRS],
[
#
# CONFIG_SUBDIRS section.
#
if test "$no_recursion" != yes; then

  # Remove --cache-file and --srcdir arguments so they do not pile up.
  ac_sub_configure_args=
  ac_prev=
  for ac_arg in $ac_configure_args; do
    if test -n "$ac_prev"; then
      ac_prev=
      continue
    fi
    case $ac_arg in
    -cache-file | --cache-file | --cache-fil | --cache-fi \
    | --cache-f | --cache- | --cache | --cach | --cac | --ca | --c)
      ac_prev=cache_file ;;
    -cache-file=* | --cache-file=* | --cache-fil=* | --cache-fi=* \
    | --cache-f=* | --cache-=* | --cache=* | --cach=* | --cac=* | --ca=* \
    | --c=*)
      ;;
    --config-cache | -C)
      ;;
    -srcdir | --srcdir | --srcdi | --srcd | --src | --sr)
      ac_prev=srcdir ;;
    -srcdir=* | --srcdir=* | --srcdi=* | --srcd=* | --src=* | --sr=*)
      ;;
    *) ac_sub_configure_args="$ac_sub_configure_args $ac_arg" ;;
    esac
  done

  for ac_subdir in : $subdirs; do test "x$ac_subdir" = x: && continue

    # Do not complain, so a configure script can configure whichever
    # parts of a large source tree are present.
    test -d $srcdir/$ac_subdir || continue

    AC_MSG_NOTICE([configuring in $ac_subdir])
    case $srcdir in
    .) ;;
    *) AS_MKDIR_P(["./$ac_subdir"])
       if test -d ./$ac_subdir; then :;
       else
         AC_MSG_ERROR([cannot create `pwd`/$ac_subdir])
       fi
       ;;
    esac

    ac_popdir=`pwd`
    cd $ac_subdir

    # A "../" for each directory in /$ac_subdir.
    ac_dots=`echo $ac_subdir |
             sed 's,^\./,,;s,[[^/]]$,&/,;s,[[^/]]*/,../,g'`

    case $srcdir in
    .) # No --srcdir option.  We are building in place.
      ac_sub_srcdir=$srcdir ;;
    [[\\/]]* | ?:[[\\/]]* ) # Absolute path.
      ac_sub_srcdir=$srcdir/$ac_subdir ;;
    *) # Relative path.
      ac_sub_srcdir=$ac_dots$srcdir/$ac_subdir ;;
    esac

    # Check for guested configure; otherwise get Cygnus style configure.
    if test -f $ac_sub_srcdir/configure.gnu; then
      ac_sub_configure="$SHELL '$ac_sub_srcdir/configure.gnu'"
    elif test -f $ac_sub_srcdir/configure; then
      ac_sub_configure="$SHELL '$ac_sub_srcdir/configure'"
    elif test -f $ac_sub_srcdir/configure.in; then
      ac_sub_configure=$ac_configure
    else
      AC_MSG_WARN([no configuration information is in $ac_subdir])
      ac_sub_configure=
    fi

    # The recursion is here.
    if test -n "$ac_sub_configure"; then
      # Make the cache file name correct relative to the subdirectory.
      case $cache_file in
      [[\\/]]* | ?:[[\\/]]* ) ac_sub_cache_file=$cache_file ;;
      *) # Relative path.
        ac_sub_cache_file=$ac_dots$cache_file ;;
      esac

      AC_MSG_NOTICE([running $ac_sub_configure $ac_sub_configure_args --cache-file=$ac_sub_cache_file --srcdir=$ac_sub_srcdir])
      # The eval makes quoting arguments work.
      eval $ac_sub_configure $ac_sub_configure_args \
           --cache-file=$ac_sub_cache_file --srcdir=$ac_sub_srcdir ||
        AC_MSG_ERROR([$ac_sub_configure failed for $ac_subdir])
    fi

    cd $ac_popdir
  done
fi
])# _AC_OUTPUT_SUBDIRS


# AC_LINKER_OPTION(LINKER-OPTIONS, SHELL-VARIABLE)
# ------------------------------------------------
#
# Specifying options to the compiler (whether it be the C, C++ or
# Fortran 77 compiler) that are meant for the linker is compiler
# dependent.  This macro lets you give options to the compiler that
# are meant for the linker in a portable, compiler-independent way.
#
# This macro take two arguments, a list of linker options that the
# compiler should pass to the linker (LINKER-OPTIONS) and the name of
# a shell variable (SHELL-VARIABLE).  The list of linker options are
# appended to the shell variable in a compiler-dependent way.
#
# For example, if the selected language is C, then this:
#
#   AC_LINKER_OPTION([-R /usr/local/lib/foo], foo_LDFLAGS)
#
# will expand into this if the selected C compiler is gcc:
#
#   foo_LDFLAGS="-Xlinker -R -Xlinker /usr/local/lib/foo"
#
# otherwise, it will expand into this:
#
#   foo_LDFLAGS"-R /usr/local/lib/foo"
#
# You are encouraged to add support for compilers that this macro
# doesn't currently support.
# FIXME: Get rid of this macro.
AC_DEFUN([AC_LINKER_OPTION],
[if test "$ac_compiler_gnu" = yes; then
  for ac_link_opt in $1; do
    $2="[$]$2 -Xlinker $ac_link_opt"
  done
else
  $2="[$]$2 $1"
fi])


# AC_LIST_MEMBER_OF(ELEMENT, LIST, [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# --------------------------------------------------------------------------
#
# Processing the elements of a list is tedious in shell programming,
# as lists tend to be implemented as space delimited strings.
#
# This macro searches LIST for ELEMENT, and executes ACTION-IF-FOUND
# if ELEMENT is a member of LIST, otherwise it executes
# ACTION-IF-NOT-FOUND.
AC_DEFUN([AC_LIST_MEMBER_OF],
[dnl Do some sanity checking of the arguments.
m4_if([$1], , [AC_FATAL([$0]: missing argument 1)])dnl
m4_if([$2], , [AC_FATAL([$0]: missing argument 2)])dnl

  ac_exists=false
  for ac_i in $2; do
    if test x"$1" = x"$ac_i"; then
      ac_exists=true
      break
    fi
  done

  AS_IF([test x"$ac_exists" = xtrue], [$3], [$4])[]dnl
])
