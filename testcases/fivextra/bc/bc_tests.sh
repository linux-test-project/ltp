#!/bin/bash
################################################################################
#                                                                              #
#  (C) Copyright IBM Corp. 2003						       #
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
# File :       bc_tests.sh
#
# Description: This program tests basic functionality of the bc program
#
# Author:      Paul Washington - wshp@us.ibm.com 
#
# History:     Sept 08 2003 - created - Paul Washington
#              Sept 18 2003 - removed "\'s" - Paul Washington
#		05 Jan 2004 - (rcp) updated to tc_utils.source


# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source


# Function:     Test for installation of commands 
#
# Description:  - If required commands are not install, terminate test
#                
#
# Return:       - zero on success.
#               - non-zero on failure.
test01()
{
	tc_register "Installation check"
	tc_executes bc
	tc_pass_or_fail $? "bc not properly installed"
}


#
# Function:    test02
#
# Description: - Test that bc will successfuly ADD two values 
#                the local system for a number of common system faults
#
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test02()
{
	tc_register    "Adding 8 + 8 expect 16"

	local OUTPUT=0
	OUTPUT=$(echo "8+8" | bc) 2>$stderr
	tc_fail_if_bad $? "Bad response from bc" || return
	[ "$OUTPUT" -eq "16" ]
	tc_pass_or_fail $? "Bad argument from bc 8 + 8"
}


#
# Function:    test03
#
# Description: - Test that bc will successfully SUBTRACT two values
#
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test03()
{
	tc_register    "Subtract 60 - 12 expect 48"
	local OUTPUT=0
	OUTPUT=$(echo "60-12" | bc) 2>$stderr
	[ "$OUTPUT" -eq "48" ]

	tc_pass_or_fail $? "Bad argument from bc 60 - 12"
}

#
# Function   test 04
#
# Description: - Test that bc will successfully MULTIPLY three numbers
#
#
# Inputs:	NONE
#
# Exit 		0 - on success
#		non-zero on failure
test04()
{
	tc_register	    "Multiply 60*12*50 expect 36000"
	local OUTPUT=0
	OUTPUT=$(echo "60*12*50" | bc) 2>$stderr
	[ "$OUTPUT" -eq "36000" ]
	tc_pass_or_fail $? "Bad argument from 60*12*50"
}

#
# Function	test 05
#
# Description: - Test that bc will successfully DIVIDE two numbers
#
#
# Inputs:	NONE
#
# Exit		0 - on success
#		non-zero on failure
test05()
{
	tc_register	  "Divide 300/2 expect 150"
	local OUTPUT=0
	OUTPUT=$(echo "300/2" | bc) 2>$stderr 
	[ "$OUTPUT" -eq "150" ]
	tc_pass_or_fail $? "Bad argument from 300/2"
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

TST_TOTAL=5
tc_setup
test01  &&
test02  && 
test03  &&
test04  &&
test05
