include(atgeneral.m4)					-*- Autoconf -*-
# M4 macros used in building Autoconf test suites.

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


## ---------------------------------------- ##
## Macros specialized in testing Autoconf.  ##
## ---------------------------------------- ##


# AT_CONFIGURE_AC(BODY)
# ---------------------
# Create a full configure.ac running BODY, with a config header set up,
# AC_OUTPUT, and environement checking hooks.
m4_define([AT_CONFIGURE_AC],
[AT_CLEANUP_FILES(env-after state*)dnl
AT_DATA([configure.ac],
[[AC_INIT
AC_CONFIG_AUX_DIR($top_srcdir/config)
AC_CONFIG_HEADER(config.h:config.hin)
AC_STATE_SAVE(before)]
$1
[AC_OUTPUT
AC_STATE_SAVE(after)
]])])


# AT_CHECK_AUTOCONF(FLAGS, [EXIT-STATUS = 0], STDOUT, STDERR)
# -----------------------------------------------------------
# Also remove `configure.in', just in case one remained from a previous
# run.
m4_define([AT_CHECK_AUTOCONF],
[AT_CLEANUP_FILES(configure.in configure)dnl
AT_CHECK([autoconf --autoconf-dir .. -l $at_srcdir $1],
         m4_default([$2], [0]), [$3], [$4])])


# AT_CHECK_AUTOHEADER
# -------------------
m4_define([AT_CHECK_AUTOHEADER],
[AT_CLEANUP_FILES(config.hin)dnl
AT_CHECK([autoheader --autoconf-dir .. -l $at_srcdir], 0, [], [])])


# AT_CHECK_CONFIGURE(END-COMMAND,
#                    [EXIT-STATUS = 0],
#                    [SDOUT = IGNORE], STDERR)
# --------------------------------------------
# `top_srcdir' is needed so that `./configure' finds install-sh.
# Using --srcdir is more expensive.
m4_define([AT_CHECK_CONFIGURE],
[AT_CLEANUP_FILE_IFELSE([config.hin], [AT_CLEANUP_FILE(config.h)])dnl
AT_CLEANUP_FILE_IFELSE([defs.in], [AT_CLEANUP_FILE(defs)])dnl
AT_CLEANUP_FILES(config.log config.status config.cache)dnl
AT_CHECK([top_srcdir=$top_srcdir ./configure $1],
         [$2],
         m4_default([$3], [ignore]), [$4],
         [test $at_verbose = echo && echo "$srcdir/AT_LINE: config.log" && cat config.log])])


# AT_CHECK_ENV
# ------------
# Check that the full configure run remained in its variable name space,
# and cleaned up tmp files.
# me tests might exit prematurely when they find a problem, in
# which case `env-after' is probably missing.  Don't check it then.
m4_define([AT_CHECK_ENV],
[if test -f state-env.before -a -f state-env.after; then
  mv -f state-env.before expout
  AT_CHECK([cat state-env.after], 0, expout)
fi
if test -f state-ls.before -a -f state-ls.after; then
  mv -f state-ls.before expout
  AT_CHECK([cat state-ls.after], 0, expout)
fi
])


# AT_CHECK_DEFINES(CONTENT)
# -------------------------
# Verify that config.h, once stripped is CONTENT.
# Stripping consists of keeping CPP lines (i.e. containing a hash),
# but those of automatically checked features (STDC_HEADERS etc.).
# AT_CHECK_HEADER is a better name, but too close from AC_CHECK_HEADER.
m4_define([AT_CHECK_DEFINES],
[AT_CHECK([[fgrep '#' config.h |
 egrep -v 'STDC_HEADERS|STD(INT|LIB)|INTTYPES|MEMORY|STRING|UNISTD|SYS_(TYPES|STAT)']],,
          [$1])])


# AT_CHECK_AUTOUPDATE
# -------------------
m4_define([AT_CHECK_AUTOUPDATE],
[AT_CHECK([autoupdate --version || exit 77], ignore, ignore, ignore)
 AT_CHECK([autoupdate --autoconf-dir $at_top_srcdir], 0,
          [], [autoupdate: `configure.ac' is updated
])])


# _AT_CHECK_AC_MACRO(AC-BODY, PRE-TESTS)
# --------------------------------------
# Create a minimalist configure.ac running the macro named
# NAME-OF-THE-MACRO, check that autoconf runs on that script,
# and that the shell runs correctly the configure.
# TOP_SRCDIR is needed to set the auxdir (some macros need `install-sh',
# `config.guess' etc.).
m4_define([_AT_CHECK_AC_MACRO],
[AT_CONFIGURE_AC([$1])
$2
AT_CHECK_AUTOCONF
AT_CHECK_AUTOHEADER
AT_CHECK_CONFIGURE
AT_CHECK_ENV
])# _AT_CHECK_AC_MACRO


# AT_CHECK_MACRO(MACRO, [MACRO-USE], [ADDITIONAL-CMDS], [AUTOCONF-FLAGS])
# -----------------------------------------------------------------------
# Create a minimalist configure.ac running the macro named
# NAME-OF-THE-MACRO, check that autoconf runs on that script,
# and that the shell runs correctly the configure.
# TOP_SRCDIR is needed to set the auxdir (some macros need `install-sh',
# `config.guess' etc.).
#
# New macros are not expected to depend upon obsolete macros.
m4_define([AT_CHECK_MACRO],
[AT_SETUP([$1])
AT_CONFIGURE_AC([m4_default([$2], [$1])])

AT_CHECK_AUTOCONF([m4_default([$4], [-W obsolete])])
AT_CHECK_AUTOHEADER
AT_CHECK_CONFIGURE
AT_CHECK_ENV
$3
AT_CLEANUP()dnl
])# AT_CHECK_MACRO


# AT_CHECK_AU_MACRO(MACRO)
# ------------------------
# Create a minimalist configure.ac running the macro named
# NAME-OF-THE-MACRO, autoupdate this script, check that autoconf runs
# on that script, and that the shell runs correctly the configure.
# TOP_SRCDIR is needed to set the auxdir (some macros need
# `install-sh', `config.guess' etc.).
#
# Updated configure.ac shall not depend upon obsolete macros, which votes
# in favor of `-W obsolete', but since many of these macros leave a message
# to be removed by the user once her code ajusted, let's not check.
#
# Remove config.hin to avoid `autoheader: config.hin is unchanged'.
m4_define([AT_CHECK_AU_MACRO],
[AT_SETUP([$1])
AT_CONFIGURE_AC([$1])

AT_CHECK_AUTOCONF
AT_CHECK_AUTOHEADER
AT_CHECK_CONFIGURE
AT_CHECK_ENV

rm config.hin
AT_CHECK_AUTOUPDATE

AT_CHECK_AUTOCONF
AT_CHECK_AUTOHEADER
AT_CHECK_CONFIGURE
AT_CHECK_ENV

AT_CLEANUP()dnl
])# AT_CHECK_UPDATE
