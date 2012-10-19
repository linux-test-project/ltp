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
#  FILE   : rsh
#
#  PURPOSE: To test the basic functionality of the `rsh` command.
#
#  SETUP: The home directory of root on the machine exported as "RHOST"
#         MUST have a ".rhosts" file with the hostname of the machine
#         where the test is executed.
#
#  HISTORY:
#    03/01 Robbie Williamson (robbiew@us.ibm.com)
#      -Ported
#
#
#
#----------------------------------------------------------------------

do_setup()
{

    FAIL_IMMEDIATELY=${FAIL_IMMEDIATELY:-1}
    SLEEPTIME=${SLEEPTIME:-0}
    NUMLOOPS=${NUMLOOPS:-1}

    export TST_TOTAL=$NUMLOOPS

    tst_setup

    exists awk hostname rsh

    RHOST=${RHOST:-`hostname`}

}

#-----------------------------------------------------------------------
#
# FUNCTION:  do_test
#
#-----------------------------------------------------------------------

do_test()
{

    while [ $TST_COUNT -le $NUMLOOPS ]; do

        PASSED=0

        if OUT=$(rsh -n -l root $RHOST 'ls -l /etc/hosts'); then

            #
            # Successful output would be something of the form:
            #
            # gcooper@orangebox ~ $ ls -l /etc/hosts
            # -rw-r--r-- 1 root root 463 Jul  5 09:26 /etc/hosts
            #
            echo "$OUT" |
                awk 'BEGIN { RET=1 } NF == 9 && $NF == "/etc/hosts" { RET=0 } END { exit RET }' \
                > /dev/null 2>&1

            if [ $? -eq 0 ] ; then
                tst_resm TPASS "rsh to $RHOST test succeeded"
                PASSED=1
            fi

        fi

        if [ $PASSED -ne 1 ] ; then
            tst_resm TFAIL "rsh to $RHOST failed"
            # If the first rsh failed, the likelihood that the rest will
            # succeed is low.
            if [ "$FAIL_IMMEDIATELY" = "1" ] && [ $TST_COUNT -eq 1 ]; then
                exit 2
            fi
        fi

        sleep $SLEEPTIME
        incr_tst_count

    done

}

#----------------------------------------------------------------------
# FUNCTION: MAIN
# PURPOSE:  To invoke the functions to perform the tasks described in
#           the prologue.
# INPUT:    None.
# OUTPUT:   A testcase run log with the results of the execution of this
#           test.
#----------------------------------------------------------------------
. net_cmdlib.sh

read_opts $*
do_setup
do_test
end_testcase
