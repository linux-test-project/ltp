#!/bin/bash
################################################################################
#                                                                              #
#  Copyright (c) International Business Machines  Corp., 2003                  #
#                                                                              #
#  This program is free software;  you can redistribute it and/or modify       #
#  it under the terms of the GNU General Public License as published by        #
#  the Free Software Foundation; either version 2 of the License, or           #
#  (at your option) any later version.                                         #
#                                                                              #
#  This program is distributed in the hope that it will be useful, but         #
#  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY  #
#  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    #
#  for more details.                                                           #
#                                                                              #
#  You should have received a copy of the GNU General Public License           #
#  along with this program;  if not, write to the Free Software                #
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA     #
#                                                                              #
################################################################################
#
# File :       ipsec_tests.sh
#
# Description: This program tests basic functionality of ipsec program
#
# Author:      Manoj Iyer  manjo@mail.utexas.edu
#
# History:     Sept 04 2003 - created - Manoj Iyer
#		08 Jan 2004 - (RR) updated to tc_utils.source


# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

RES=0 # flag indicates that ipsec was already running.


# Function:     tc_local_cleanup
#
# Description:  - restart ipsec if it was already running before the test
#                 was executed.
#
# Return:       - zero on success.
#               - non-zero on failure.
tc_local_cleanup()
{
    # if ipsec was already running restart it before exit.
    # cant do much about it if it fails!
    [ $RES -eq 1 ] && \
    { /etc/init.d/ipsec start 2>$stderr 1>$stdout  ; }
}


#
# Function:    test01
#
# Description: - Test that ipsec verify without argument, verify examines 
#                the local system for a number of common system faults
#
#              - execute the ipsec verfy command with no arguments and
#                look for certain key words.
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test01()
{
    tc_register    "ipsec functionality"
    
    tc_info "executing command 'ipsec verify' on local system"

    ipsec verify >$TMP/tst_ipsec.out 2>&1
    tc_fail_if_bad $? "failed verifying local system $(cat $TMP/tst_ipsec.out)" \
    || return

    tc_info "checking for certain key words in output"
    grep -i "Checking for KLIPS support in kernel" $TMP/tst_ipsec.out \
    2>$stderr 1>$stdout &&\
    grep -i "Checking for RSA private key" $TMP/tst_ipsec.out \
    2>$stderr 1>$stdout && \
    grep -i "Checking that pluto is running" $TMP/tst_ipsec.out \
    2>$stderr 1>$stdout &&\
    grep -i "Looking for forward key" $TMP/tst_ipsec.out \
    2>$stderr 1>$stdout

    tc_pass_or_fail $? "output did not contain certain key phrases" || return
}


#
# Function:    test02
#
# Description: - Test ipsec setup (start|stop|restart|reload|version|status)
#                options, these options allow user to start and stop
#                ipsec on local system
#
#              - ipsec setup status to check if ipsec is already running
#              - ipsec setup stop to stop it
#              - ipsec setup start to start it.
#              - ipsec setup restart to stop and start it.
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test02()
{
    tc_register    "ipsec functionality"

    tc_info "check if ipsec is already running"

    /etc/init.d/ipsec status | grep -i "IPsec running" && \
    {
        tc_info "ipsec is already running"
	/etc/init.d/ipsec stop | grep -i "ipsec_setup: Stopping FreeS/WAN IPsec" \
	 2>&1
	tc_fail_if_bad $? "failed to stop ipsec already running" || return
        RES=1 ;
        tc_info "stopping ipsec"
    }
    
    tc_info "starting ipsec using command 'ipsec setup start'"
    /etc/init.d/ipsec start &>$TMP/tst_ipsec.out
    tc_fail_if_bad $? "failed to start ipsec $(cat $TMP/tst_ipsec.out)" || return
    
    grep -i "ipsec_setup: Starting FreeS/WAN IPsec" $TMP/tst_ipsec.out \
    2>$stderr 1>$stdout
    tc_fail_if_bad $? "failed to start ipsec" || return

    tc_info "restarting ipsec using command 'ipsec setup restart'"
    /etc/init.d/ipsec restart &>$TMP/tst_ipsec.out 
    tc_fail_if_bad $? "failed restarting ipsec $(cat $TMP/tst_ipsec.out)" || return

    grep -i "ipsec_setup: Stopping FreeS/WAN IPsec" $TMP/tst_ipsec.out && \
    grep -i "ipsec_setup: Starting FreeS/WAN IPsec" $TMP/tst_ipsec.out \
    2>$stderr 1>$stdout 
    tc_fail_if_bad $? "failed to restart ipsec" || return

    [ $RES -eq 0 ] && \
    {
        tc_info "stopping ipsec using command 'ipsec setup stop'"
	/etc/init.d/ipsec stop | grep -i "ipsec_setup: Stopping FreeS/WAN IPsec" \
	>/dev/null 2>&1
	tc_fail_if_bad $? "failed to stop ipsec" || return ;
    }

    # if we reached here we passed this testcase.
    tc_pass_or_fail 0 "failed to pass ipsec setup"
}    


#
# Function: main
# 
# Description: - call setup function.
#              - execute each test.
#
# Inputs:      NONE
#
# Exit:        zero - success
#              non_zero - failure
#

TST_TOTAL=2
tc_setup

test01  &&\
test02 
