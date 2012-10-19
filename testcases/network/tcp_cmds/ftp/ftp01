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
#  FILE   : ftp
#
#  PURPOSE: To test the basic functionality of the `ftp` command.
#
#  SETUP: The home directory of root on the machine exported as "RHOST"
#         MUST have a ".rhosts" file with the hostname of the machine
#         where the test is executed. Also, both machines MUST have
#         the same path configuration for the test for proper test data
#         file transfers. The PASSWD variable should also be set to root's
#	  login password.
#
#  HISTORY:
#    06/06/03 Manoj Iyer manjo@mail.utexas.edu
#      - Modified to use LTP test harness APIs
#    03/01 Robbie Williamson (robbiew@us.ibm.com)
#      -Ported
#
#
#-----------------------------------------------------------------------
#
#----------------------------------------------------------------------

#-----------------------------------------------------------------------
#
# FUNCTION:  do_setup
#
#-----------------------------------------------------------------------

do_setup()
{

    TC=ftp
    TCtmp=${TCtmp:-$LTPROOT/$TC${EXEC_SUFFIX}$$}
    TCdat=${TCdat:-$LTPROOT/datafiles}
    SLEEPTIME=${SLEEPTIME:-0}
    ASCII_FILES=${ASCII_FILES:-"ascii.sm ascii.med ascii.lg ascii.jmb"}
    BIN_FILES=${BIN_FILES:-"bin.sm bin.med bin.lg bin.jmb"}

    RHOST=${RHOST:-`hostname`}
    RUSER=${RUSER:-root}
    PASSWD=${PASSWD:-.pasroot}

    tst_setup

    exists awk ftp rsh

    cd "$TCtmp"

    rsh -n -l root $RHOST mkdir -p "$TCtmp"
    rsh -n -l root $RHOST chown -R ${RUSER} "$TCtmp"
    [ $? = 0 ] || end_testcase "Check .rhosts file on remote machine."

}

#-----------------------------------------------------------------------
#
# FUNCTION:  do_test
#
#-----------------------------------------------------------------------

do_test()
{

    for i in binary ascii; do

        if [ $i = "binary" ]; then
            FILES=$BIN_FILES
        fi
        if [ $i = "ascii" ]; then
            FILES=$ASCII_FILES
        fi
        for j in $FILES; do

            for a in get put; do
                if [ $a = "get" ]; then
                    {
                        echo user $RUSER $PASSWD
                        echo lcd $TCtmp
                        echo $i
                        echo cd $TCdat
                        echo $a $j
                        echo quit
                    } | ftp -nv $RHOST
                    SUM1=`ls -l $TCtmp/$j  | awk '{print $5}'`
                    SUM2=`ls -l $TCdat/$j | awk '{print $5}'`
                    rm -f $TCtmp/$j
                else
                    {
                        echo user $RUSER $PASSWD
                        echo lcd $TCdat
                        echo $i
                        echo cd $TCtmp
                        echo $a $j
                        echo quit
                    } | ftp -nv $RHOST
                    SUM1=`rsh -n -l root $RHOST sum $TCtmp/$j | awk '{print $1}'`
                    SUM2=`sum $TCdat/$j | awk '{print $1}'`
                    rsh -n -l root $RHOST rm -f $TCtmp/$j
                fi

                if [ $SUM1 = $SUM2 ]; then
                    tst_resm TINFO "Test Successful doing ftp $a $j $i"
                else
                    end_testcase "Test Fail: Wrong sum while performing ftp $a $j $i"
                fi
                sleep $SLEEPTIME
            done
        done
    done
}


#-----------------------------------------------------------------------
#
# FUNCTION:  do_cleanup
#
#-----------------------------------------------------------------------

do_cleanup()
{
    rsh -n -l root $RHOST rmdir "$TCtmp"
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
