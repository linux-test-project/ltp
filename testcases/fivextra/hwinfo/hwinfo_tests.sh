#!/bin/bash
################################################################################
#                                                                              #
#  (C) Copyright IBM Corp. 2003                  				#
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
# File :       hwinfo_tests.sh
#
# Description: This program tests basic functionality of hwinfo command.
#
# Author:      Manoj Iyer  manjo@mail.utexas.edu
#
# History:     Aug 15 2003 - created - Manoj Iyer
#		08 Jan 2004 - (RR) updated to tc_utils.source


# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source


#
# Function:    test01
#
# Description: - Test that hwinfo --short will display the list of 
#                hardware that is available on the system.
#              - execute command hwinfo and look for keywords like
#                cpu, graphics card, network etc
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test01()
{
    local string="NONE"
    tc_register    "hwinfo functionality"
    
    tc_info "executing command hwinfo and gathering system info"
    tc_info "this might take a few mts..."
    hwinfo --short  &>$TCTMP/tst_hwinfo.out

    # create a list of things that must be in the output. Infact if any of the
    # following is found hwinfo is successful.
    # should contain atleast disk, cpu and memory!!!!
    for string in "cpu:" "memory:" "disk:"
    do
        grep -i $string $TCTMP/tst_hwinfo.out 2>$stderr 1>$stdout 
        tc_pass_or_fail $? "failed to find entry for $string in output"
    done
    
}


#
# Function:    test02
#
# Description: - Test that hwinfo command gives out the correct information.
#              - use hwinfo with --hw_item option and check if the info
#                is correct.
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test02()
{
    tc_register    "hwinfo functionality"
    local CPU=" "
    local VIDEO=" "
    local BRIDGE=" "
    
    CPU=$(cat /proc/cpuinfo | grep -i "model name" | awk '{print $4}')
    VIDEO=$(cat /proc/pci | grep -i "VGA compatible" | awk '{print $4}')
    BRIDGE=$(cat /proc/pci | grep -i "PCI bridge:" | awk '{print $5}')

    hwinfo --cpu | grep -i "$CPU" 2>$stderr 1>$stdout ||
    hwinfo --gfxcard | grep -i "$VIDEO" 2>$stderr 1>$stdout ||
    hwinfo --bridge | grep -i "$BRIDGE" 2>$stderr 1>$stdout ||
    tc_pass_or_fail $? "Failed to output required information"
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
tc_exec_or_break  hwinfo grep awk || exit

test01 &&
test02
