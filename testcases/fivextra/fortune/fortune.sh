#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003		      ##
##									      ##
## This program is free software;  you can redistribute it and#or modify      ##
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
# File :	fortune.sh
#
# Description:	Test fortune package
#
# Author:	Robb Romans <robb@austin.ibm.com>
#
# History:	Jun 6 2003 - created - RR
#		Jun 24 2003 - RC Paulsen <rpaulsen@us.ibm.com>
#			- Does not hard-code path to the fortune command.
#			- Checks that it is installed.
#			- Checks that it gives some output.
#		06 Jan 2004 - (RR) updated to tc_utils.source
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# testcase functions
################################################################################

function test01 {
	tc_register "Is fortune installed?"
	tc_exec_or_break fortune
	tc_pass_or_fail $? "Fortune is not properly installed"
}

function test02 {
	tc_register "Does fortune execute properly"
	fortune >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected response from fortune command" || return
	[ -s $stdout ]
	tc_pass_or_fail $? "Fortune did not write anything to stdout"
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
tc_setup
test01 && \
test02
