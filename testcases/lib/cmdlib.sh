#!/bin/sh
#
#    Command library that provides a boilerplate set of functions and variables
#    required for all bourne shell based scripts.
#
#    Copyright (C) 2009, Cisco Systems Inc.
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Ngie Cooper, August 2009
#

set -u

export SHELL_DEBUG=${SHELL_DEBUG:=0}
if [ "x$SHELL_DEBUG" = x1 ] ; then
    set -x
fi

#=============================================================================
# FUNCTION:   tst_cleanup
# PURPOSE:    Clean up after a testcase.
#=============================================================================
tst_cleanup()
{
    # Disable the trap EXIT handler.
    trap '' EXIT
    # To ensure set -u passes...
    TCtmp=${TCtmp:=}
    tst_resm TINFO "Cleaning up."
    # Nuke the testcase temporary directory if it exists.
    [ -d "$TCtmp" ] && rm -rf "$TCtmp"
}

#=============================================================================
# FUNCTION:  setup
# PURPOSE:   Setup the test environment.
#=============================================================================
tst_setup()
{

    TST_COUNT=1
    TST_TOTAL=${TST_TOTAL:=1}
    export TCID TST_COUNT TST_TOTAL

    for varname in TST_TOTAL; do
        if eval "test -z \"\$${varname}\""; then
            end_testcase "You must set ${varname} before calling setup()."
        fi
    done

    LTPROOT=${LTPROOT:="../../../../"}
    TEMPDIR=${TEMPDIR:=/tmp}

    TCtmp=${TCtmp:=$TEMPDIR/$TC$$}
    # User wants a temporary sandbox to play with.
    if [ -n "$TCtmp" -a "$TCtmp" != "$TEMPDIR/$$" ] ; then
        test -d "$TCtmp" || mkdir -p "$TCtmp"
        # Clean up on exit.
        trap tst_cleanup EXIT
    fi

}

#=============================================================================
# FUNCTION NAME:        end_testcase
#
# FUNCTION DESCRIPTION: Print out whether or not a test failed. Do not use
#			this when TBROK messages should be applied.
#
# PARAMETERS:           Failure message, or "" / unset if passed.
#
# RETURNS:              None.
#=============================================================================
end_testcase()
{
    if [ $# -eq 0 ]; then
        tst_resm TPASS "Test successful"
        exit 0
    else
        tst_resm TFAIL "Test broken: $*"
        exit 1
    fi
}

#=============================================================================
# FUNCTION:  exists
# PURPOSE:   Check if command(s) used by this test script exist.
#=============================================================================
exists()
{
    for cmd in $*; do
        if ! command -v $cmd >/dev/null 2>&1; then
            end_testcase "$cmd: command not found"
            exit 1
        fi
    done
}

incr_tst_count()
{
    : $(( TST_COUNT += 1 ))
}

tst_require_root()
{
	if [ "x$(id -u)" != "x0" ]; then
		tst_resm TCONF "You must be root to execute this test"
		exit 0
	fi
}

#
# $0 is maintained by the caller script; tested on FreeBSD (ash) and Gentoo
# GNU/Linux (bash) -- assuming you aren't calling this from another function
# or command:
#
# foo.sh:
# echo ${0##*/}
# . ${$0%%*}/bar.sh
# bar.sh:
# echo ${0##*/}
# echo $SHELL
#
# Gentoo:
# gcooper@orangebox ~/Desktop $ ./foo.sh
# foo.sh
# foo.sh
# /bin/bash
# gcooper@orangebox ~/Desktop $ uname -sr
# Linux 2.6.29-gentoo-r5
#
# FreeBSD:
# $ ./foo.sh
# foo.sh
# foo.sh
# /bin/sh
# $ uname -sr
# FreeBSD 8.0-BETA2
#
TCID=${TCID:=}
[ -z "$TCID" ] && TCID=${0##*/}
TC=$(echo "$TCID" | awk '{ sub( /[0-9]+$/,""); print; }')

. daemonlib.sh
