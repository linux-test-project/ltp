#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## Copyright (c) International Business Machines  Corp., 2003		      ##
##									      ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or	      ##
## (at your option) any later version.					      ##
##									      ##
## This program is distributed in the hope that it will be useful, but	      ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.							      ##
##									      ##
## You should have received a copy of the GNU General Public License	      ##
## along with this program;  if not, write to the Free Software		      ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##									      ##
################################################################################
#
# File :	ngpt-posix.sh
#
# Description:	test the new POSIX semantics of ngpt. 
#
# Author:	Helen Pang. hpang@us.ibm.com
#
# History:	Sept. 02 2003 - Created. Helen Pang. hpang@us.ibm.com
#		16 Dec 2003 - (hpang) updated to tc_utils.source
#		19 Feb 2003 (rcp) BUG 6432: added ngptinit.
#
###############################################################################
#source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

############################################################################
# the testcase functions
###########################################################################

#
# test01
#
function test01()
{
	tc_register	"installation check and initialization"
	tc_exec_or_break ngptinit

	ngptinit &>$stdout	# good message goes to stderr
	tc_pass_or_fail $? "could not initialize ngpt threading"
}

#
# test02	test getpid() and compare the pids
#
function test02()
{
	tc_register "test getpid and compare its pids"
	
	# get and compare the pids before and after the posix thread created
	t_getpid >$stdout 2>$stderr
	tc_pass_or_fail $? "failed to get and compare the pids" 
}


###########################################################################
# main
###########################################################################

TST_TOTAL=2

# standard tc_setup
tc_setup		

test01 &&
test02
