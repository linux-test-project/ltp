#!/bin/bash
################################################################################
#                                                                              #
#  (C) Copyright IBM Corp. 2003                  #
#                                                                              #
#  This program is free software;  you can redistribute it and#or modify       #
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
# File :       bridge-utils_tests.sh
#
# Description: This program tests basic functionality of bridge utils commands.
#
# Author:      Manoj Iyer  manjo@mail.utexas.edu
#
# History:     July 14 2003 - created - Manoj Iyer
#              Aug  11 2003 - Modified - Manoj Iyer
#              - fix for defect 3901 Moved trap cleanup after setup.
#              Aug  18 2003 - Modified - Manoj Iyer
#              - cleaned up error message "[: too many arguments"
#		08 Jan 2004 - (RR) updated to tc_utils.source


# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source


#
# Function:    tc_local_cleanup
#
# Description: - burn any bridges that were created.
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
tc_local_cleanup()
{
    local BRIDGES=" "
    BRIDGES=$(brctl show | grep tst_br | awk '{print $1}')
    if ! [ -z "$BRIDGES" ]
    then
        for bridges in $BRIDGES
        do
            brctl delbr $bridges 2>$stderr 1>$stdout
        done
    fi
}


#
# Function:    test01
#
# Description: - Test that brctl command can create a bridge port group
#              - create bridge port group tst_br0
#              - check if this group was created.
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test01()
{
    tc_register    "brctl addbr functionality"
    
    tc_info "create a bridge port group tst_br0"

    brctl addbr tst_br0 2>$stderr 1>$stdout
    tc_pass_or_fail $? "Failed to add new bridge port group tst_br0"
}


#
# Function:    test02
#
# Description: - Test that brctl command can display bridge port groups
#              - create bridge port group tst_br0
#              - check if this group was created.
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test02()
{
    tc_register    "brctl show functionality"
    
    tc_info "list all bridges that were created."

    brctl addbr tst_br02 2>$stderr 1>$stdout
    tc_fail_if_bad $? "Failed to add new bridge port group tst_br02" || return

    brctl show | grep tst_br02 2>$stderr 1>$stdout
    tc_pass_or_fail $? "no bridges were listed by brctl command."
}


#
# Function:    test03
#
# Description: - Test that brctl command can delete a bridge port group
#              - create bridge port group tst_br0
#              - check if this group was created.
#              - delete this group.
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test03()
{
    tc_register    "brctl delbr functionality"
    
    tc_info "delete a bridge port group tst_br03"

    brctl addbr tst_br03 2>$stderr 1>$stdout
    tc_fail_if_bad $? "Failed to add new bridge port group tst_br03" || return

    brctl show | grep tst_br03 2>$stderr 1>$stdout
    tc_fail_if_bad $? "tst_br03 bridge was not listed by brctl command." || return

    brctl delbr tst_br03 2>$stderr 1>$stdout
    tc_pass_or_fail $? "unable to delete tst_br03"    
}



#
# Function:    test04
#
# Description: - Test that brctl command can show macaddress of
#                 a bridge port group
#              - create bridge port group tst_br0
#              - check for mac address. If no interfaces are added it will
#                display message indicating no mac address. 
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test04()
{
    tc_register    "brctl showmacs functionality"
    
    tc_info "show macaddr of a bridge port group tst_br04"

    brctl addbr tst_br04 2>$stderr 1>$stdout
    tc_fail_if_bad  $? "Failed to add new bridge port group tst_br0" || return

    brctl show | grep tst_br04 2>$stderr 1>$stdout
    tc_fail_if_bad $? "tst_br04 bridge was not listed by brctl command." || return

    brctl showmacs tst_br04 &>$TCTMP/tst_bridgeutils.out 
    tc_fail_if_bad $? \
    "showmacs option to brctl failed. Reason: $(cat $TCTMP/tst_bridgeutils.out)"
   
    cat <<-EOF > $TCTMP/tst_bridgeutils.exp
    port no mac addr                is local?       ageing timer
	EOF

    diff -wiqB $TCTMP/tst_bridgeutils.exp $TCTMP/tst_bridgeutils.out
    tc_pass_or_fail $? "showmacs did not return expected information"
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
TST_TOTAL=4
tc_setup        # exits on failure
tc_root_or_break || exit
tc_exec_or_break  brctl diff awk cat || exit

test01 &&\
test02 &&\
test03 &&\
test04
