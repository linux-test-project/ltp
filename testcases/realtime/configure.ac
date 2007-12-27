/******************************************************************************/
/*                                                                            */
/* Copyright (c) International Business Machines  Corp., 2007                 */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/******************************************************************************/

# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.59)

AC_INIT([rt-test],[0.3],[linux-rt-users@vger.kernel.org])
AC_CONFIG_HEADERS([include/rttests_config.h])
#AC_CONFIG_SRCDIR([internal/examples/libstats_test.c])
AC_CONFIG_MACRO_DIR([config/m4])
AC_CONFIG_AUX_DIR([config/autoconf])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AM_MAINTAINER_MODE

CFLAGS="${CFLAGS} -Wall -I/usr/include/nptl"
LDFLAGS="-L/usr/lib/nptl -lpthread"

AM_INIT_AUTOMAKE([foreign no-exeext dist-bzip2])

AC_PROG_RANLIB

#
# Checks for header files.
#
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
dnl AC_CHECK_HEADERS([limits.h])


#
# Checks for typedefs, structures, and compiler characteristics.
#
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME


#
# Checks for library functions.
#

AC_CHECK_FUNCS([pthread_mutexattr_setprotocol])
AM_CONDITIONAL(
    [HAVE_PTHREAD_MUTEXATTR_SETPROTOCOL],
    [test "$ac_cv_func_pthread_mutexattr_setprotocol" = "yes"])

AC_CHECK_FUNCS([pthread_mutexattr_setrobust_np])
AM_CONDITIONAL(
    [HAVE_PTHREAD_MUTEXATTR_SETROBUST_NP],
    [test "$ac_cv_func_pthread_mutexattr_setrobust_np" = "yes"])

#
# Debugging
#
AC_MSG_CHECKING([whether to enable debugging])
AC_ARG_ENABLE(debug,
    AS_HELP_STRING([--enable-debug], [enable debugging @<:@default=yes@:>@]),
	[case "$enableval" in
	y | yes) CONFIG_DEBUG=yes ;;
        *) CONFIG_DEBUG=no ;;
    esac],
    [CONFIG_DEBUG=yes])
AC_MSG_RESULT([${CONFIG_DEBUG}])
if test "${CONFIG_DEBUG}" = "yes"; then
    CFLAGS="${CFLAGS} -Wsign-compare -Wfloat-equal -Wformat-security -Werror -g -O1"
    AC_DEFINE(DEBUG, 1, [debugging])
else
    CFLAGS="${CFLAGS} -O2"
fi

AC_CONFIG_FILES([
		include/GNUmakefile
                lib/GNUmakefile
		config/GNUmakefile
		stress/pi-tests/GNUmakefile
		stress/GNUmakefile
		GNUmakefile
		perf/latency/GNUmakefile
		perf/GNUmakefile
		func/sched_latency/GNUmakefile
		func/async_handler/GNUmakefile
		func/hrtimer-prio/GNUmakefile
		func/sched_jitter/GNUmakefile
		func/sched_football/GNUmakefile
		func/matrix_mult/GNUmakefile
		func/pi-tests/GNUmakefile
		func/prio-wake/GNUmakefile
		func/measurement/GNUmakefile
		func/gtod_latency/GNUmakefile
		func/pthread_kill_latency/GNUmakefile
		func/periodic_cpu_load/GNUmakefile
		func/pi_perf/GNUmakefile
		func/GNUmakefile
		func/prio-preempt/GNUmakefile
		func/thread_clock/GNUmakefile
])

AC_OUTPUT
