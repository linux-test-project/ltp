#!/bin/sh
#***********************************************************************
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
#  PURPOSE: To test the basic functionality of the `netstat` command.
#
#  HISTORY:
#    06/06/03 Manoj Iyer manjo@mail.utexas.edu
#      - Modified test to use LTP harness API
#      - Fixed bugs.
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

    exists netstat
}

#-------------------------------------------------------------------------
# FUNCTION:  do_test
# PURPOSE:
#            To loop for LOOPCOUNT times
#            If MAXCOUNT is "-1" the "while"
#            loop will execute until terminated by "intr" signal.
# INPUT:     None.
# OUTPUT:    Informational messages are logged into the run log.
#
#-----------------------------------------------------------------------

do_test() {

    while [ $TST_COUNT -le $NUMLOOPS ]; do

        for flag in "-s" "-rn" "-i" "-gn" "-apn"; do
            if ! netstat $flag 1>/dev/null 2>&1; then
                end_testcase "netstat $flag failed"
            fi
        done

        incr_tst_count

        sleep $SLEEPTIME

   done

}

#-----------------------------------------------------------------------
#
# FUNCTION:  MAIN
# PURPOSE:   To invoke functions that perform the tasks as described in
#        the design in the prolog above.
# INPUT:     See SETUP in the prolog above.
# OUTPUT:    Logged run results written to testcase run log
#
#-----------------------------------------------------------------------
. net_cmdlib.sh

read_opts $*
do_setup
do_test
end_testcase
