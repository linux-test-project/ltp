#! /bin/sh

# Build some of the Autoconf test files.
# Copyright 2000 Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.

# If we fail, clean up, but touch the output files.  We probably failed
# because we used some non portable tool, but we just don't care: this
# shell script is a maintainer tool, and we do expect good tools.

as_me=`echo "$0" | sed 's,.*[\\/],,'`

trap 'echo "'"$as_me"': failed.  To proceed run make check." >&2
      rm -f acdefuns audefuns requires *.tat
      for file in "$@"
      do
        touch `echo "$file" | sed "s,.*[\\/],,;s/\..*/.at/"`
      done
      trap 0
      exit 1' \
     0 1 2 15

# If ever something goes wrong, fail, so that the trap be launched.
set -e

# We need arguments.
test $# != 0

# We need these arguments.
src="$@"

# Set locale to C so that `sort' behaves in a uniform way.
export LANGUAGE; LANGUAGE=C
export LANG; LANG=C
export LC_ALL; LC_ALL=C


# requires
# --------
# Get the list of macros that are required: there is little interest
# in testing them since they will be run but the guy who requires
# them.
cat $src |
  sed -n 's/dnl.*//;s/.*AC_REQUIRE(\[*\([a-zA-Z0-9_]*\).*$/\1/p' |
  sort |
  uniq >requires


# exclude_list
# ------------
# Macros which must not be checked at all (not by ac-macros.at, nor
# au-macros.at).
# The trailing new line is meant.
#
# - ac_cv_prog_gcc, gxx, g77
#   Not macros, just mapping from old variable name to a new one.
exclude_list='^ac_cv_prog_(gcc|gxx|g77)$
'


# ac_exclude_list
# ---------------
# The test `ac-macros.at' tries to run all the macros of Autoconf to check
# for syntax problems, etc.  Not all the macros can be run without argument,
# and some are already tested elsewhere.  EGREP_EXCLUDE must filter out
# the macros we don't want to test in ac-macros.at.
#
# - AC_CANONICALIZE, AC_PREFIX_PROGRAM, AC_PREREQ
#   Need an argument.
# - AC_CHECK decl, file, func, header, lib, member, prog, sizeof, type
#   Performed in the semantics tests.
# - AC_CONFIG
#   They fail when the source does not exist.
# - AC_INIT
#   AC_INIT includes all the AC_INIT macros.  Note that there is an
#   infinite m4 recursion if AC_INIT it used twice.
# - AC_LANG*
#   Heavily used by other macros.
# - AC_PATH_PROGS?, AC_F77_FUNC
#   They produce `= val' because $1, the variable used to store the result,
#   is empty.
# - AC_TRY, AC_.*_IFELSE, AC_RUN_LOG.
#   Used in many places.
# - _AC_
#   Internal macros are used elsewhere.
# - AC_OUTPUT
#   Already tested by `AT_CHECK_MACRO'.
# - AC_FD_CC
#   Is a number.
# - AC_PROG_CC, AC_C_(CONST|INLINE|VOLATILE), AC_PATH_XTRA
#   Checked in semantics.
# - AC_CYGWIN, AC_CYGWIN32, AC_EMXOS2, AC_MING32, AC_EXEEXT, AC_OBJEXT
#   AU defined to nothing.
# - AC_PATH_XTRA
#   Checked in semantics.
# - AC_SYS_RESTARTABLE_SYSCALLS, AC_FUNC_WAIT3
#   Obsolete, checked in semantics.
#
ac_exclude_list='^AC_ARG_VAR$
^AC_CANONICALIZE|AC_PREFIX_PROGRAM|AC_PREREQ$
^AC_CHECK_(DECL|FILE|FUNC|HEADER|LIB|MEMBER|PROG|SIZEOF|TOOL|TYPE)S?$
^AC_CONFIG
^AC_F77_FUNC$
^AC_INIT
^AC_LANG
^AC_LINKER_OPTION$
^AC_LINK_FILES$
^AC_LIST_MEMBER_OF$
^AC_OUTPUT$
^AC_PATH_(TOOL|PROG)S?$
^AC_REPLACE_FUNCS$
^AC_SEARCH_LIBS$
^(AC_TRY.*|AC_RUN_LOG)$
^AC_.*_IFELSE$
^AC_FD_CC$
^(AC_(PROG_CC|C_CONST|C_INLINE|C_VOLATILE))$
^AC_(CYGWIN|CYGWIN32|EMXOS2|MING32|EXEEXT|OBJEXT)$
^AC_PATH_XTRA$
^AC_SYS_RESTARTABLE_SYSCALLS$
^AC_FUNC_WAIT3$
_AC_'


# ac_exclude_egrep
# ----------------
# Build a single egrep pattern out of filter_macros_list.
# Sed is used to get rid of the trailing `|' coming from the trailing
# `\n' from `echo'.
ac_exclude_egrep=`echo "$exclude_list$ac_exclude_list" | tr '
' '|' | sed 's/.$//'`


# au_exclude_list
# ---------------
# AC_LANG_RESTORE
#    cannot be used alone.
# AC_LINK_FILES, AC_PREREQ
#    need arguments and are tested elsewhere.
# AC_INIT and AC_OUTPUT
#    are already in `configure.ac'.
# AC_CYGWIN, AC_MINGW32, AC_EMXOS2
#    are using AC_REQUIRE.
au_exclude_list='^AC_LANG_RESTORE$
^AC_LINK_FILES|AC_PREREQ$
^AC_(INIT|OUTPUT)$
^AC_(CYGWIN|MINGW32|EMXOS2)$'


# au_exclude_egrep
# ----------------
# Build a single egrep pattern out of filter_macros_list.
# Sed is used to get rid of the trailing `|' coming from the trailing
# `\n' from `echo'.
au_exclude_egrep=`echo "$exclude_list$au_exclude_list" | tr '
' '|' | sed 's/.$//'`



## ------------------------- ##
## Creating the test files.  ##
## ------------------------- ##

for file in $src
do
  base=`echo "$file" | sed 's,.*[\\/],,;s/\..*//'`
  # Get the list of macros which are defined in Autoconf level.
  # Get rid of the macros we are not interested in.
  cat $file |
    sed -n -e 's/^AC_DEFUN(\[*\([a-zA-Z0-9_]*\).*$/\1/p' \
  	   -e 's/^AC_DEFUN_ONCE(\[*\([a-zA-Z0-9_]*\).*$/\1/p' |
    sort |
    uniq |
    # Watch out we are `set -e': don't fail.
    ( egrep -v "$ac_exclude_egrep" || true) >acdefuns

  # Get the list of macros which are defined in Autoupdate level.
  cat $file |
    sed -n 's/^AU_DEFUN(\[*\([a-zA-Z][a-zA-Z0-9_]*\).*$/\1/p' |
    sort |
    uniq |
    ( egrep -v "$au_exclude_egrep" || true) > audefuns

  # Filter out required macros.
  {
    sed 's/^ *//' <<MK_EOF
    # Generated by $as_me, do not edit by hand.	-*- autoconf -*-

    AT_BANNER([Testing $base macros.])

MK_EOF

    echo "# Modern macros."
    for macro in `cat acdefuns`; do
      if fgrep "$macro" requires >/dev/null 2>&1; then :; else
  	echo "AT_CHECK_MACRO([$macro])"
      fi
    done
    echo
    echo "# Obsolete macros."
    for macro in `cat audefuns`; do
      if fgrep "$macro" requires >/dev/null 2>&1; then :; else
  	echo "AT_CHECK_AU_MACRO([$macro])"
      fi
    done
  } >$base.tat

  # In one atomic step so that if something above fails, the trap
  # preserves the old version of the file.
  mv $base.tat $base.at
done

rm -f acdefuns audefuns requires

trap 0
exit 0
