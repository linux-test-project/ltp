#!/bin/sh
#
#   Copyright (c) International Business Machines  Corp., 2000
#
#   This program is free software;  you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY;  without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#   the GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program;  if not, write to the Free Software
#   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
#
#
#  FILE   : host
#
#  PURPOSE: To test the basic functionality of the `host` command.
#
#  SETUP: If "RHOST" is not exported, then the local hostname is used.
#
#  HISTORY:
#    06/06/03 Manoj Iyer manjo@mail.utexas.edu
#      - Modified to use LTP tests APIs
#    03/01 Robbie Williamson (robbiew@us.ibm.com)
#      -Ported
#
#
#-----------------------------------------------------------------------

do_setup()
{
    NUMLOOPS=${NUMLOOPS:-1}
    SLEEPTIME=${SLEEPTIME:-0}

    tst_setup

    exists awk host hostname

    RHOST=${RHOST:-`hostname`}

}

#-----------------------------------------------------------------------
#
# FUNCTION:  do_test
#
#-----------------------------------------------------------------------

do_test()
{

    tst_resm TINFO "test basic functionality of the \`$TC' command."

    while [ $TST_COUNT -lt $NUMLOOPS ]; do

        if rhost_addr=$(host $RHOST); then
            rhost_addr=$(echo "$rhost_addr" | awk -F, '{print $NF}') >/dev/null 2>&1
            if ! host $rhost_addr >/dev/null 2>&1; then
                end_testcase "reverse lookup with host failed"
            fi

        else
            end_testcase "host $RHOST on local machine failed"
        fi

        incr_tst_count
        sleep $SLEEPTIME

    done

}

#-----------------------------------------------------------------------
# FUNCTION: MAIN
#-----------------------------------------------------------------------
. net_cmdlib.sh

read_opts $*
do_setup
do_test
end_testcase
