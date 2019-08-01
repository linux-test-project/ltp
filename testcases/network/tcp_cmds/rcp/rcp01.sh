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
#  FILE   : rcp
#
#  PURPOSE: To test the basic functionality of the `rcp` command.
#
#  SETUP: The home directory of root on the machine exported as "RHOST"
#         MUST have a ".rhosts" file with the hostname of the machine
#         where the test is executed.
#
#  HISTORY:
#    06/06/03 Manoj Iyer manjo@mail.utexas.edu
#    - Modified testcases to use test APIS and fixed bugs
#    03/01 Robbie Williamson (robbiew@us.ibm.com)
#      -Ported
#
#
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
#
# FUNCTION:  do_setup
#
#-----------------------------------------------------------------------

do_setup()
{

    TCtmp=${TCtmp:-$LTPROOT/testcases/bin/$TC${EXEC_SUFFIX}$$}
    TCdat=${TCdat:-$LTPROOT/testcases/bin/datafiles}
    LHOST=`hostname`
    RHOST=${RHOST:-$LHOST}
    SLEEPTIME=${SLEEPTIME:-0}
    FILES=${FILES:-"bin.sm bin.med bin.lg bin.jmb"}

    tst_setup

    exists awk rcp rsh sum

    if ! rsh -n -l root $RHOST mkdir -p $TCtmp >/dev/null 2>&1; then
        end_testcase "Remote mkdir failed"
    fi

    trap do_cleanup EXIT

}

#-----------------------------------------------------------------------
#
# FUNCTION:  do_test
#
#-----------------------------------------------------------------------

do_test()
{

    for j in $FILES; do

        if ! rcp $TCdat/$j $RHOST:$TCtmp/$j; then
            end_testcase "Failed to rcp file."
        fi

        SUM1=`sum $TCdat/$j | awk '{print $1}'`
        SUM2=`rsh -n -l root $RHOST sum $TCtmp/$j | awk '{print $1}'`
        rsh -n -l root $RHOST "rm -f $TCtmp/$j"
        sleep $SLEEPTIME

        if [ "$SUM1" = "$SUM2" ]; then
            tst_resm TINFO "rcp $TCdat/$j $RHOST:$TCtmp/$j successful"
        else
            end_testcase "FAILED: wrong sum in transfer to $RHOST"
        fi

        sleep $SLEEPTIME

    done

    for j in $FILES; do

        tst_resm TINFO "remote copying $RHOST:$TCdat/$j to $TCtmp/$j"
        if ! rcp $RHOST:$TCdat/$j $TCtmp/$j >/dev/null 2>&1; then
            tst_resm TFAIL "Failed to rcp file."; continue
        fi

        SUM1=`sum $TCtmp/$j | awk '{print $1}'`
        SUM2=`rsh -n -l root $RHOST sum $TCdat/$j | awk '{print $1}'`
        rm -f $TCtmp/$j
        sleep $SLEEPTIME

        if [ "$SUM1" = "$SUM2" ]; then
            tst_resm TINFO "rcp $RHOST:$TCdat/$j $TCtmp/$j successful"
        else
            end_testcase "FAILED: wrong sum in transfer to $LHOST from $RHOST"
        fi
        sleep $SLEEPTIME

    done

}

#-----------------------------------------------------------------------
#
# FUNCTION:  do_cleanup
#
#-----------------------------------------------------------------------

do_cleanup()
{
    rsh -n -l root $RHOST rmdir $TCtmp
    tst_cleanup
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
