include(m4sugar.m4)#                                        -*- Autoconf -*-
# This file is part of Autoconf.
# M4 sugar for common shell constructs.
# Requires GNU M4 and M4sugar.
# Copyright 2000, 2001 Free Software Foundation, Inc.
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
# Written by Akim Demaille, Pavel Roskin, Alexandre Oliva, Lars J. Aas
# and many other people.


## ------------------------- ##
## 1. Sanitizing the shell.  ##
## ------------------------- ##

# AS_SHELL_SANITIZE
# -----------------
# Try to be as Bourne and/or POSIX as possible.
m4_defun([AS_SHELL_SANITIZE],
[# Be Bourne compatible
if test -n "${ZSH_VERSION+set}" && (emulate sh) >/dev/null 2>&1; then
  emulate sh
  NULLCMD=:
elif test -n "${BASH_VERSION+set}" && (set -o posix) >/dev/null 2>&1; then
  set -o posix
fi

# Name of the executable.
dnl Moved here because the tests below can use AC_MSG_ERROR, which uses $as_me
as_me=`AS_BASENAME($[0])`

_AS_EXPR_PREPARE
_AS_LN_S_PREPARE
_AS_TEST_PREPARE
_AS_UNSET_PREPARE
_AS_TR_PREPARE

# NLS nuisances.
AS_UNSET([LANG],        [C])
AS_UNSET([LC_ALL],      [C])
AS_UNSET([LC_TIME],     [C])
AS_UNSET([LC_CTYPE],    [C])
AS_UNSET([LANGUAGE],    [C])
AS_UNSET([LC_COLLATE],  [C])
AS_UNSET([LC_NUMERIC],  [C])
AS_UNSET([LC_MESSAGES], [C])

# IFS
# We need space, tab and new line, in precisely that order.
as_nl='
'
IFS=" 	$as_nl"

# CDPATH.
AS_UNSET([CDPATH], [:])
])


## ----------------------------- ##
## 2. Wrappers around builtins.  ##
## ----------------------------- ##

# This section is lexicographically sorted.


# AS_EXIT([EXIT-CODE = 1])
# ------------------------
# Exit and set exit code to EXIT-CODE in the way that it's seen
# within "trap 0".
#
# We cannot simply use "exit N" because some shells (zsh and Solaris sh)
# will not set $? to N while running the code set by "trap 0"
# So we set $? by executing "exit N" in the subshell and then exit.
# Other shells don't use `$?' as default for `exit', hence just repeating
# the exit value can only help improving portability.
m4_define([AS_EXIT],
[{ (exit m4_default([$1], 1)); exit m4_default([$1], 1); }])


# AS_IF(TEST, [IF-TRUE], [IF-FALSE])
# ----------------------------------
# Expand into
# | if TEST; then
# |   IF-TRUE
# | else
# |   IF-FALSE
# | fi
# with simplifications is IF-TRUE and/or IF-FALSE is empty.
m4_define([AS_IF],
[m4_ifval([$2$3],
[if $1; then
  m4_ifval([$2], [$2], :)
m4_ifvaln([$3],
[else
  $3])dnl
fi
])dnl
])# AS_IF


# _AS_UNSET_PREPARE
# -----------------
# AS_UNSET depends upon $as_unset: compute it.
m4_defun([_AS_UNSET_PREPARE],
[# Support unset when possible.
if (FOO=FOO; unset FOO) >/dev/null 2>&1; then
  as_unset=unset
else
  as_unset=false
fi
])


# AS_UNSET(VAR, [VALUE-IF-UNSET-NOT-SUPPORTED = `'])
# --------------------------------------------------
# Try to unset the env VAR, otherwise set it to
# VALUE-IF-UNSET-NOT-SUPPORTED.  `as_unset' must have been computed.
m4_defun([AS_UNSET],
[m4_require([_AS_UNSET_PREPARE])dnl
$as_unset $1 || test "${$1+set}" != set || { $1=$2; export $1; }])






## ------------------------------------------ ##
## 3. Error and warnings at the shell level.  ##
## ------------------------------------------ ##

# If AS_MESSAGE_LOG_FD is defined, shell messages are duplicated there
# too.


# AS_ESCAPE(STRING, [CHARS = $"'\])
# ---------------------------------
# Escape the CHARS in STRING.
m4_define([AS_ESCAPE],
[m4_patsubst([$1],
             m4_ifval([$2], [[\([$2]\)]], [[\([\"$`]\)]]),
             [\\\1])])


# _AS_QUOTE_IFELSE(STRING, IF-MODERN-QUOTATION, IF-OLD-QUOTATION)
# ---------------------------------------------------------------
# Compatibility glue between the old AS_MSG suite which did not
# quote anything, and the modern suite which quotes the quotes.
# If STRING contains `\\' or `\$', it's modern.
# If STRING contains `\"' or `\`', it's old.
# Otherwise it's modern.
# We use two quotes in the pattern to keep highlighting tools at peace.
m4_define([_AS_QUOTE_IFELSE],
[m4_if(m4_regexp([$1], [\\[\\$]]),
       [-1], [m4_if(m4_regexp([$1], [\\[`""]]),
                    [-1], [$2],
                    [$3])],
       [$2])])


# _AS_ECHO_UNQUOTED(STRING, [FD = AS_MESSAGE_FD])
# -----------------------------------------------
# Perform shell expansions on STRING and echo the string to FD.
m4_define([_AS_ECHO_UNQUOTED],
[echo "$1" >&m4_default([$2], [AS_MESSAGE_FD])])


# _AS_QUOTE(STRING)
# -----------------
# If there are quoted (via backslash) backquotes do nothing, else
# backslash all the quotes.
# FIXME: In a distant future (2.51 or +), this warning should be
# classified as `syntax'.  It is classified as `obsolete' to ease
# the transition (for Libtool for instance).
m4_define([_AS_QUOTE],
[_AS_QUOTE_IFELSE([$1],
                  [AS_ESCAPE([$1], [`""])],
                  [m4_warn([obsolete],
           [back quotes and double quotes should not be escaped in: $1])dnl
$1])])


# _AS_ECHO(STRING, [FD = AS_MESSAGE_FD])
# --------------------------------------
# Protect STRING from backquote expansion, echo the result to FD.
m4_define([_AS_ECHO],
[_AS_ECHO_UNQUOTED([_AS_QUOTE([$1])], [$2])])


# AS_MESSAGE(STRING, [FD = AS_MESSAGE_FD])
# ----------------------------------------
m4_define([AS_MESSAGE],
[m4_ifset([AS_MESSAGE_LOG_FD],
          [{ _AS_ECHO([$as_me:__oline__: $1], [AS_MESSAGE_LOG_FD])
_AS_ECHO([$as_me: $1], [$2]);}],
          [_AS_ECHO([$as_me: $1], [$2])])[]dnl
])


# AS_WARN(PROBLEM)
# ----------------
m4_define([AS_WARN],
[AS_MESSAGE([WARNING: $1], [2])])# AS_WARN


# AS_ERROR(ERROR, [EXIT-STATUS = 1])
# ----------------------------------
m4_define([AS_ERROR],
[{ AS_MESSAGE([error: $1], [2])
   AS_EXIT([$2]); }[]dnl
])# AS_ERROR



## -------------------------------------- ##
## 4. Portable versions of common tools.  ##
## -------------------------------------- ##

# This section is lexicographically sorted.


# AS_DIRNAME(PATHNAME)
# --------------------
# Simulate running `dirname(1)' on PATHNAME, not all systems have it.
# This macro must be usable from inside ` `.
#
# Prefer expr to echo|sed, since expr is usually faster and it handles
# backslashes and newlines correctly.  However, older expr
# implementations (e.g. SunOS 4 expr and Solaris 8 /usr/ucb/expr) have
# a silly length limit that causes expr to fail if the matched
# substring is longer than 120 bytes.  So fall back on echo|sed if
# expr fails.
#
# FIXME: Please note the following m4_require is quite wrong: if the first
# occurrence of AS_DIRNAME_EXPR is in a backquoted expression, the
# shell will be lost.  We might have to introduce diversions for
# setting up an M4sh script: required macros will then be expanded there.
m4_defun([AS_DIRNAME_EXPR],
[m4_require([_AS_EXPR_PREPARE])dnl
$as_expr X[]$1 : 'X\(.*[[^/]]\)//*[[^/][^/]]*/*$' \| \
         X[]$1 : 'X\(//\)[[^/]]' \| \
         X[]$1 : 'X\(//\)$' \| \
         X[]$1 : 'X\(/\)' \| \
         .     : '\(.\)'])

m4_defun([AS_DIRNAME_SED],
[echo X[]$1 |
    sed ['/^X\(.*[^/]\)\/\/*[^/][^/]*\/*$/{ s//\1/; q; }
  	  /^X\(\/\/\)[^/].*/{ s//\1/; q; }
  	  /^X\(\/\/\)$/{ s//\1/; q; }
  	  /^X\(\/\).*/{ s//\1/; q; }
  	  s/.*/./; q']])

m4_defun([AS_DIRNAME],
[AS_DIRNAME_EXPR([$1]) 2>/dev/null ||
AS_DIRNAME_SED([$1])])


# AS_BASENAME(PATHNAME)
# --------------------
# Simulate running `basename(1)' on PATHNAME, not all systems have it.
# This macro must be usable from inside ` `.
m4_defun([AS_BASENAME],
[echo "$1" |sed 's,.*[[\\/]],,'])

# AS_EXECUTABLE_P
# ---------------
# Check whether a file is executable.
m4_defun([AS_EXECUTABLE_P],
[m4_require([_AS_TEST_PREPARE])dnl
$as_executable_p $1[]dnl
])# AS_EXECUTABLE_P


# _AS_EXPR_PREPARE
# ----------------
# Some expr work properly (i.e. compute and issue the right result),
# but exit with failure.  When a fall back to expr (as in AS_DIRNAME)
# is provided, you get twice the result.  Prevent this.
m4_defun([_AS_EXPR_PREPARE],
[if expr a : '\(a\)' >/dev/null 2>&1; then
  as_expr=expr
else
  as_expr=false
fi
])# _AS_EXPR_PREPARE


# _AS_LN_S_PREPARE
# ----------------
# Don't use conftest.sym to avoid filename issues on DJGPP, where this
# would yield conftest.sym.exe for DJGPP < 2.04.  And don't use `conftest'
# as base name to avoid prohibiting concurrency (e.g., concurrent
# config.statuses).
m4_defun([_AS_LN_S_PREPARE],
[rm -f conf$$ conf$$.exe conf$$.file
echo >conf$$.file
if ln -s conf$$.file conf$$ 2>/dev/null; then
  # We could just check for DJGPP; but this test a) works b) is more generic
  # and c) will remain valid once DJGPP supports symlinks (DJGPP 2.04).
  if test -f conf$$.exe; then
    # Don't use ln at all; we don't have any links
    as_ln_s='cp -p'
  else
    as_ln_s='ln -s'
  fi
elif ln conf$$.file conf$$ 2>/dev/null; then
  as_ln_s=ln
else
  as_ln_s='cp -p'
fi
rm -f conf$$ conf$$.exe conf$$.file
])# _AS_LN_S_PREPARE


# AS_LN_S(FILE, LINK)
# -------------------
# FIXME: Should we add the glue code to handle properly relative symlinks
# simulated with `ln' or `cp'?
m4_defun([AS_LN_S],
[m4_require([_AS_LN_S_PREPARE])dnl
$as_ln_s $1 $2
])


# AS_MKDIR_P(PATH)
# ----------------
# Emulate `mkdir -p' with plain `mkdir'.
#
# Don't set IFS to '\\/' (see the doc): you would end up with
# directories called foo\bar and foo?az and others depending upon the
# shell.
m4_define([AS_MKDIR_P],
[{ case $1 in
  [[\\/]]* | ?:[[\\/]]* ) as_incr_dir=;;
  *)                      as_incr_dir=.;;
esac
as_dummy=$1
for as_mkdir_dir in `IFS='/\\'; set X $as_dummy; shift; echo "$[@]"`; do
  case $as_mkdir_dir in
    # Skip DOS drivespec
    ?:) as_incr_dir=$as_mkdir_dir ;;
    *)
      as_incr_dir=$as_incr_dir/$as_mkdir_dir
      test -d "$as_incr_dir" || mkdir "$as_incr_dir"
    ;;
  esac
done; }
])# AS_MKDIR_P


# _AS_BROKEN_TEST_PREPARE
# -----------------------
# FIXME: This does not work and breaks way too many things.
#
# Find out ahead of time whether we want test -x (preferred) or test -f
# to check whether a file is executable.
m4_defun([_AS_BROKEN_TEST_PREPARE],
[# Find out how to test for executable files. Don't use a zero-byte file,
# as systems may use methods other than mode bits to determine executability.
cat >conf$$.file <<_ASEOF
@%:@! /bin/sh
exit 0
_ASEOF
chmod +x conf$$.file
if test -x conf$$.file >/dev/null 2>&1; then
  as_executable_p="test -x"
elif test -f conf$$.file >/dev/null 2>&1; then
  as_executable_p="test -f"
else
  AS_ERROR([cannot check whether a file is executable on this system])
fi
rm -f conf$$.file
])# _AS_BROKEN_TEST_PREPARE


# _AS_TEST_PREPARE
# ----------------
m4_defun([_AS_TEST_PREPARE],
[as_executable_p="test -f"
])# _AS_BROKEN_TEST_PREPARE






## ------------------ ##
## 5. Common idioms.  ##
## ------------------ ##

# This section is lexicographically sorted.


# AS_BOX(MESSAGE, [FRAME-CHARACTER = `='])
# ----------------------------------------
# Output MESSAGE, a single line text, framed with FRAME-CHARACTER (which
# must not be `/').
m4_define([AS_BOX],
[AS_LITERAL_IF([$1],
               [_AS_BOX_LITERAL($@)],
               [_AS_BOX_INDIR($@)])])

# _AS_BOX_LITERAL(MESSAGE, [FRAME-CHARACTER = `='])
# -------------------------------------------------
m4_define([_AS_BOX_LITERAL],
[cat <<\_ASBOX
m4_patsubst([$1], [.], m4_if([$2], [], [[=]], [[$2]]))
$1
m4_patsubst([$1], [.], m4_if([$2], [], [[=]], [[$2]]))
_ASBOX])

# _AS_BOX_INDIR(MESSAGE, [FRAME-CHARACTER = `='])
# -----------------------------------------------
m4_define([_AS_BOX_INDIR],
[sed 'h;s/./m4_default([$2], [=])/g;p;x;p;x' <<_ASBOX
$1
_ASBOX])


# AS_LITERAL_IF(EXPRESSION, IF-LITERAL, IF-NOT-LITERAL)
# -----------------------------------------------------
# If EXPRESSION has shell indirections ($var or `expr`), expand
# IF-INDIR, else IF-NOT-INDIR.
# This is an *approximation*: for instance EXPRESSION = `\$' is
# definitely a literal, but will not be recognized as such.
m4_define([AS_LITERAL_IF],
[m4_if(m4_regexp([$1], [[`$]]),
       -1, [$2],
       [$3])])


# AS_TMPDIR(PREFIX)
# -----------------
# Create as safely as possible a temporary directory which name is
# inspired by PREFIX (should be 2-4 chars max), and set trap
# mechanisms to remove it.
m4_define([AS_TMPDIR],
[# Create a temporary directory, and hook for its removal unless debugging.
$debug ||
{
  trap 'exit_status=$?; rm -rf $tmp && exit $exit_status' 0
  trap 'AS_EXIT([1])' 1 2 13 15
}

# Create a (secure) tmp directory for tmp files.
: ${TMPDIR=/tmp}
{
  tmp=`(umask 077 && mktemp -d -q "$TMPDIR/$1XXXXXX") 2>/dev/null` &&
  test -n "$tmp" && test -d "$tmp"
}  ||
{
  tmp=$TMPDIR/$1$$-$RANDOM
  (umask 077 && mkdir $tmp)
} ||
{
   echo "$me: cannot create a temporary directory in $TMPDIR" >&2
   AS_EXIT
}dnl
])# AS_TMPDIR


# AS_UNAME
# --------
# Try to describe this machine.  Meant for logs.
m4_define([AS_UNAME],
[{
cat <<_ASUNAME
## ---------- ##
## Platform.  ##
## ---------- ##

hostname = `(hostname || uname -n) 2>/dev/null | sed 1q`
uname -m = `(uname -m) 2>/dev/null || echo unknown`
uname -r = `(uname -r) 2>/dev/null || echo unknown`
uname -s = `(uname -s) 2>/dev/null || echo unknown`
uname -v = `(uname -v) 2>/dev/null || echo unknown`

/usr/bin/uname -p = `(/usr/bin/uname -p) 2>/dev/null || echo unknown`
/bin/uname -X     = `(/bin/uname -X) 2>/dev/null     || echo unknown`

/bin/arch              = `(/bin/arch) 2>/dev/null              || echo unknown`
/usr/bin/arch -k       = `(/usr/bin/arch -k) 2>/dev/null       || echo unknown`
/usr/convex/getsysinfo = `(/usr/convex/getsysinfo) 2>/dev/null || echo unknown`
hostinfo               = `(hostinfo) 2>/dev/null               || echo unknown`
/bin/machine           = `(/bin/machine) 2>/dev/null           || echo unknown`
/usr/bin/oslevel       = `(/usr/bin/oslevel) 2>/dev/null       || echo unknown`
/bin/universe          = `(/bin/universe) 2>/dev/null          || echo unknown`

PATH = $PATH

_ASUNAME
}])



## ------------------------------------ ##
## Common m4/sh character translation.  ##
## ------------------------------------ ##

# The point of this section is to provide high level macros comparable
# to m4's `translit' primitive, but m4/sh polymorphic.
# Transliteration of literal strings should be handled by m4, while
# shell variables' content will be translated at runtime (tr or sed).


# _AS_CR_PREPARE
# --------------
# Output variables defining common character ranges.
# See m4_cr_letters etc.
m4_defun([_AS_CR_PREPARE],
[# Avoid depending upon Character Ranges.
as_cr_letters='abcdefghijklmnopqrstuvwxyz'
as_cr_LETTERS='ABCDEFGHIJKLMNOPQRSTUVWXYZ'
as_cr_Letters=$as_cr_letters$as_cr_LETTERS
as_cr_digits='0123456789'
as_cr_alnum=$as_cr_Letters$as_cr_digits
])


# _AS_TR_SH_PREPARE
# -----------------
m4_defun([_AS_TR_SH_PREPARE],
[m4_require([_AS_CR_PREPARE])dnl
# Sed expression to map a string onto a valid variable name.
as_tr_sh="sed y%*+%pp%;s%[[^_$as_cr_alnum]]%_%g"
])


# AS_TR_SH(EXPRESSION)
# --------------------
# Transform EXPRESSION into a valid shell variable name.
# sh/m4 polymorphic.
# Be sure to update the definition of `$as_tr_sh' if you change this.
m4_defun([AS_TR_SH],
[m4_require([_$0_PREPARE])dnl
AS_LITERAL_IF([$1],
              [m4_patsubst(m4_translit([[$1]], [*+], [pp]),
                           [[^a-zA-Z0-9_]], [_])],
              [`echo "$1" | $as_tr_sh`])])


# _AS_TR_CPP_PREPARE
# ------------------
m4_defun([_AS_TR_CPP_PREPARE],
[m4_require([_AS_CR_PREPARE])dnl
# Sed expression to map a string onto a valid CPP name.
as_tr_cpp="sed y%*$as_cr_letters%P$as_cr_LETTERS%;s%[[^_$as_cr_alnum]]%_%g"
])


# AS_TR_CPP(EXPRESSION)
# ---------------------
# Map EXPRESSION to an upper case string which is valid as rhs for a
# `#define'.  sh/m4 polymorphic.  Be sure to update the definition
# of `$as_tr_cpp' if you change this.
m4_defun([AS_TR_CPP],
[m4_require([_$0_PREPARE])dnl
AS_LITERAL_IF([$1],
              [m4_patsubst(m4_translit([[$1]],
                                       [*abcdefghijklmnopqrstuvwxyz],
                                       [PABCDEFGHIJKLMNOPQRSTUVWXYZ]),
                           [[^A-Z0-9_]], [_])],
              [`echo "$1" | $as_tr_cpp`])])


# _AS_TR_PREPARE
# --------------
m4_defun([_AS_TR_PREPARE],
[m4_require([_AS_TR_SH_PREPARE])dnl
m4_require([_AS_TR_CPP_PREPARE])dnl
])




## --------------------------------------------------- ##
## Common m4/sh handling of variables (indirections).  ##
## --------------------------------------------------- ##


# The purpose of this section is to provide a uniform API for
# reading/setting sh variables with or without indirection.
# Typically, one can write
#   AS_VAR_SET(var, val)
# or
#   AS_VAR_SET(as_$var, val)
# and expect the right thing to happen.


# AS_VAR_SET(VARIABLE, VALUE)
# ---------------------------
# Set the VALUE of the shell VARIABLE.
# If the variable contains indirections (e.g. `ac_cv_func_$ac_func')
# perform whenever possible at m4 level, otherwise sh level.
m4_define([AS_VAR_SET],
[AS_LITERAL_IF([$1],
               [$1=$2],
               [eval "$1=$2"])])


# AS_VAR_GET(VARIABLE)
# --------------------
# Get the value of the shell VARIABLE.
# Evaluates to $VARIABLE if there are no indirection in VARIABLE,
# else into the appropriate `eval' sequence.
m4_define([AS_VAR_GET],
[AS_LITERAL_IF([$1],
               [$[]$1],
               [`eval echo '${'m4_patsubst($1, [[\\`]], [\\\&])'}'`])])


# AS_VAR_TEST_SET(VARIABLE)
# -------------------------
# Expands into the `test' expression which is true if VARIABLE
# is set.  Polymorphic.  Should be dnl'ed.
m4_define([AS_VAR_TEST_SET],
[AS_LITERAL_IF([$1],
               [test "${$1+set}" = set],
               [eval "test \"\${$1+set}\" = set"])])


# AS_VAR_SET_IF(VARIABLE, IF-TRUE, IF-FALSE)
# ------------------------------------------
# Implement a shell `if-then-else' depending whether VARIABLE is set
# or not.  Polymorphic.
m4_define([AS_VAR_SET_IF],
[AS_IF([AS_VAR_TEST_SET([$1])], [$2], [$3])])


# AS_VAR_PUSHDEF and AS_VAR_POPDEF
# --------------------------------
#

# Sometimes we may have to handle literals (e.g. `stdlib.h'), while at
# other moments, the same code may have to get the value from a
# variable (e.g., `ac_header').  To have a uniform handling of both
# cases, when a new value is about to be processed, declare a local
# variable, e.g.:
#
#   AS_VAR_PUSHDEF([header], [ac_cv_header_$1])
#
# and then in the body of the macro, use `header' as is.  It is of
# first importance to use `AS_VAR_*' to access this variable.  Don't
# quote its name: it must be used right away by m4.
#
# If the value `$1' was a literal (e.g. `stdlib.h'), then `header' is
# in fact the value `ac_cv_header_stdlib_h'.  If `$1' was indirect,
# then `header's value in m4 is in fact `$ac_header', the shell
# variable that holds all of the magic to get the expansion right.
#
# At the end of the block, free the variable with
#
#   AS_VAR_POPDEF([header])


# AS_VAR_PUSHDEF(VARNAME, VALUE)
# ------------------------------
# Define the m4 macro VARNAME to an accessor to the shell variable
# named VALUE.  VALUE does not need to be a valid shell variable name:
# the transliteration is handled here.  To be dnl'ed.
m4_define([AS_VAR_PUSHDEF],
[AS_LITERAL_IF([$2],
               [m4_pushdef([$1], [AS_TR_SH($2)])],
               [as_$1=AS_TR_SH($2)
m4_pushdef([$1], [$as_[$1]])])])


# AS_VAR_POPDEF(VARNAME)
# ----------------------
# Free the shell variable accessor VARNAME.  To be dnl'ed.
m4_define([AS_VAR_POPDEF],
[m4_popdef([$1])])



## ----------------- ##
## Setting M4sh up.  ##
## ----------------- ##


# AS_INIT
# -------
m4_define([AS_INIT],
[m4_init

# Forbidden tokens and exceptions.
m4_pattern_forbid([^_?AS_])
])
