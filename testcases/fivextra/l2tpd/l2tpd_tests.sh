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
# File :       l2tpd_tests.sh
#
# Description: This program tests basic functionality of l2tpd demon
#
# Author:      Manoj Iyer  manjo@mail.utexas.edu
#
# History:     July 28 2003 - created - Manoj Iyer
#              July 29 2003 - Review changes - R C Paulsen
#              July 29 2003 - more sensible messages when test fails.
#              Oct  13 2003 - check for /var/log/messages not syslog.
#              Oct  14 2003 - fixes for suse. and added clean up function
#		08 Jan 2004 - (RR) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

#
# Function:    tc_local_cleanup
#
# Description: - kill l2tpd that was started
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
tc_local_cleanup()
{
    ps -C l2tpd 2>&1 1>$stdout && { killall -9 l2tpd ; } 
}


#
# Function:    test01
#
# Description: - Test the functionality of l2tpd demon
#              - check if it output appropriate messages to /var/log/messages
#
# Inputs:        NONE
#
# Exit         0 - on success
#              non-zero on failure.
test01()
{
    tc_register    "l2tpd functionality"

    tc_exec_or_break  echo grep sleep l2tpd || return

    tc_exist_or_break  /var/log/messages 
    tc_fail_if_bad $? "unable to find required file(s)" || return

    # create a dummy l2tpd.conf file.
    cat <<-EOF > $TCTMP/l2tpd.conf
    [global]
    port = 1701
	EOF

    # stop any previous execution of l2tpd
    ps -C l2tpd && { killall -9 l2tpd ; }

    # execute l2tpd
    l2tpd -c $TCTMP/l2tpd.conf 2>&1 1>$stdout
    tc_fail_if_bad $? "l2tpd failed to start" || return

    # check if certain messages are logged in messages.
    # get l2tpd related messages from /var/log/messages
    sleep 2s    # wait for logging to occur
    tc_info "checking if messages are written to messages"
    grep l2tpd /var/log/messages > $TCTMP/tst_l2tpd.out 2>$stderr
    tc_fail_if_bad $? "no messages from to l2tpd" || return

    # check for certain messages from l2tpd demon
    tc_info "look for certain messages "
    grep "This binary does not support kernel L2TP" $TCTMP/tst_l2tpd.out \
    >$TCTMP/tst_l2tp.stdout 2>&1 || \
    grep " Written by Mark Spencer" $TCTMP/tst_l2tpd.out \
    >>$TCTMP/tst_l2tp.stdout 2>&1
    tc_pass_or_fail $? "expected messages not found $(cat $TCTMP/tst_l2tpd.out)"
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

TST_TOTAL=1
tc_setup        # exits on failure
tc_root_or_break || exit

test01
