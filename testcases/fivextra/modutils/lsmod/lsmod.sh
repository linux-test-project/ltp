#!/bin/bash
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2003						      ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File :        lsmod.sh
#
# Author:       Yu-Pao Lee (yplee@us.ibm.com)
#
# Description:  Test basic functionality of lsmod command
#		lsmod - list loaded modules
#
# History:      Feb 19 2003 - Created. Yu-Pao Lee (yplee@us.ibm.com) 
#		17 Dec 2003 - (rcp) updated to tc_utils.source

################################################################################
# source the standard utility functions 
################################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# the testcase functions
################################################################################

#
# test01	installation check
#
function test01()
{
	tc_register "installation check"
	tc_executes lsmod
	tc_pass_or_fail $? "insmod not installed properly"
}

#
# test02	lsmod - list loaded modules 
#
function test02()
{
	tc_register "lsmod - list loaded modules"	
	
	tc_exec_or_break cat sed diff || return
	tc_exist_or_break "/proc/modules" || return 
	cat /proc/modules > $TCTMP/modules1
	/sbin/lsmod 2>$stderr | sed 1d > $TCTMP/modules2
	diff -bB $TCTMP/modules1 $TCTMP/modules2 > $TCTMP/diffout
	tc_pass_or_fail $? "Unexpected result" \
			"expected"$'\n'"`cat $TCTMP/modules1`" \
			"actual"$'\n'"`cat $TCTMP/modules2`" \
			"diff"$'\n'"`cat $TCTMP/diffout`"
}  

################################################################################
# main
################################################################################

TST_TOTAL=2

tc_setup			# standard setup

tc_root_or_break || exit

test01 &&
test02
