#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003						      ##
##									      ##
## This program is free software;  you can redistribute it and/or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or	      ##
## (at your option) any later version.					      ##
##									      ##
## This program is distributed in the hope that it will be useful, but	      ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MEECHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.							      ##
##									      ##
## You should have received a copy of the GNU General Public License	      ##
## along with this program;  if not, write to the Free Software		      ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##									      ##
################################################################################
#
# File :	testpycrypt.sh
#
# Description:	Test pycrypt package
#
# Author:	Robb Romans <robb@austin.ibm.com>
#
# History:	Jun 17 2003 - created - RR
#
#		07 Jan 2004 - (RR) updated to tc_utils.source
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

# global variables

################################################################################
# testcase functions
################################################################################

function test01 {

	tc_register "pycrypt_01"
	pycrypt.py 1>$stdout 2>$stderr
	tc_pass_or_fail $? "pycrypt_01 test failed "

}
####################################################################################
# MAIN
####################################################################################

# Function:	main
#

#
# Exit:		- zero on success
#		- non-zero on failure
#
TST_TOTAL=1
tc_setup
test01
