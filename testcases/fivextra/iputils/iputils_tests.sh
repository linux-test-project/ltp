#!/bin/bash
################################################################################
#                                                                              #
#  (C) Copyright IBM Corp. 2003                                                #
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
# File :       iputils_tests.sh
#
# Description: This program tests basic functionality of iputils program
#              This testcase does not include tests for traceroute and 
#              ping as they alreay exists.
#
# Author:      Manoj Iyer  manjo@mail.utexas.edu
#
# History:     Sept 09 2003 - created - Manoj Iyer
#              Sept 29 2003 - Modified, suse localhost does not respond to 
#                             arping and tracepath, use valid ip addresses
#                             instead. Suggestion from Ryan Harper.
#              Oct 24 2003 - Modified, arping will ping the default route.
#              Dec 03 2003 - Andrew Pham
#                          - Default DEFROUTE=$iface 
#		08 Jan 2004 - (RR) updated to tc_utils.source
#		11 Feb 2004 (rcp) find interface dynamically; don't assume eth0

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

#
# setup
#
function tc_local_setup()
{
	set $(route | grep default)
	iface=$8
	[ "$iface" ]
	tc_break_if_bad $? "can't find network interface" || return
}

#
# Function:    test01
#
# Description: - Test that 'arping -c 4 localhost' will send an ARP request
#                to host.
#              - execute command arping -c 4 localhost
#              - capture all messages to a file
#              - look for certain key words/phrases that will indicate
#                expected behaviour.
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test01()
{
    tc_register    "arping functionality"
    
    arping -c 4 -I $iface `/sbin/route -n | grep UG | awk '{print $2}'` \
    &>$TCTMP/tst_iputils.out 
    tc_fail_if_bad $? \
        "Failed sending ARP request $(cat $TCTMP/tst_iputils.out)" || return ;

    # check for key-words/phrases that will indicated expected 
    # behaviour.
    tc_info "check for key-words/phrases"
    grep -i "ARPING" $TCTMP/tst_iputils.out \
    2>$stderr 1>$stdout && \
    grep -i "Unicast reply from" $TCTMP/tst_iputils.out \
    2>$stderr 1>$stdout && \
    grep -i "Sent 4 probes" $TCTMP/tst_iputils.out  2>$stderr 1>$stdout && \
    grep -i "Received 4 response" $TCTMP/tst_iputils.out \
    2>$stderr 1>$stdout
    tc_pass_or_fail $? "did not get expected response."
}


#
# Function:    test02
#
# Description: - Test that 'tracepath hostname' will  traces path to a network 
#                host.
#              - execute command tracepath localhost
#              - capture all messages to a file
#              - look for certain key words/phrases that will indicate
#                expected behaviour.
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test02()
{
    tc_register    "tracepath functionality"
    
    # execute iputils command 'tracepath self'
    tc_info "executing 'tracepath self'"
    tracepath $(hostname -i) &>$TCTMP/tst_iputils.out 
    tc_fail_if_bad $? \
        "tracing route to localhost $(cat $TCTMP/tst_iputils.out)" || return ;

    # check for key-words/phrases that will indicated expected 
    # behaviour.
    tc_info "check for key-words/phrases"
    grep -i $(hostname -i) $TCTMP/tst_iputils.out | grep -i "reached" \
    2>$stderr 1>$stdout 
    tc_pass_or_fail $? "did not get expected response."
}


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
# check dependencies
tc_root_or_break || exit
tc_exec_or_break  arping tracepath grep || exit

test01  &&\
test02
