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
# File :       utempter_tests.sh
#
# Description: This program tests basic functionality of utemp and utempter 
#              commands.
#
# Author:      Manoj Iyer  manjo@mail.utexas.edu
#
# History:     Aug 18 2003 - created - Manoj Iyer
#              Sep 30 2003 - Modified - utmp utility was missing added that
#                            to the testsuite.
#		08 Jan 2004 - (RR) updated to tc_utils.source


# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source


#
# Function:    test01
#
# Description: - Test that utmp will display user accounting information.
#              - execute command utmp and look for keywords LOGIN,
#                LOGIN_PROCESS USER_PROCESS etc.
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test01()
{
    local RC=0       # return code
    tc_register    "utmp functionality"
    
    tc_info "executing command utmp for user accounting info"
    utmp >$TCTMP/tst_utmp.out 2>&1
    tc_fail_if_bad $? "utemp failed to gather required information" || return

    # create a list of things that must be in the output. 
    for string in  "RUN_LVL" "USER_PROCESS" "LOGIN" 
    do
        grep -i $string $TCTMP/tst_utmp.out 2>$stderr 1>$stdout || \
        { RC=$? ; break ; }
    done
    tc_pass_or_fail $RC "failed to find entry for $string in output"
    
}


#
# Function:    test02
#
# Description: - Test that utempter command gives statistics of users logged in
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test02()
{
    tc_register    "utempter functionality"
    
    tc_info "creating dummy device."
    mknod $TCTMP/tst_utempter.dev b 28 0 2>$stderr 1>$stdout
    tc_fail_if_bad $> "failed to create a dummy device"

    utempter -a $TCTMP/tst_utempter.dev $(hostname) &>$TCTMP/tst_utempter.out \
    && { tst_resm TFAIL "utempter worked unexpectedly" ; }

    cat <<-EOF > $TCTMP/tst_utempter.exp
    the device must begin with /dev/
	EOF

    diff -iqwB $TCTMP/tst_utempter.exp $TCTMP/tst_utempter.out 2>$stderr 
    tc_pass_or_fail $? "expected message differ from actual message from untempter"
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
tc_root_or_break || exit
tc_exec_or_break  utmp utempter diff mknod cat || exit

test01 &&\
test02
