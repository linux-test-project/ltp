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
# File :	minicom.sh
#
# Description:	Test minicom package
#
# Author:	Andrew Pham, apham@austin.ibm.com
#
# History:	July 25 2003 - Created - Andrew Pham
#
#
#		07 Jan 2004 - (apham) updated to tc_utils.source
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

TST_TOTAL=2
REQUIRED="minicom ascii-xfr runscript xminicom"
MYSRCHSTRG="lookingforthis$$"
################################################################################
# testcase functions
################################################################################
function TC_minicom()
{	
	tc_info "minicom must be tested  manually."
	tc_info "xminicom must be tested  manually."
}
function TC_ascii-xfr()
{
	tc_register "ascii-xfr -s"
	echo "a test file for ascii-xfr" > $TCTMP/tstfile.txt
	echo "a test file for ascii-xfr" >> $TCTMP/tstfile.txt
	echo "$MYSRCHSTRG" >> $TCTMP/tstfile.txt
	echo "a test file for ascii-xfr" >> $TCTMP/tstfile.txt

	ascii-xfr -s $TCTMP/tstfile.txt >$stdout 2>/dev/null
	tc_fail_if_bad $? "ascii-xfr cann't send a file." || return
	
	grep $MYSRCHSTRG $stdout &>/dev/null
	tc_pass_or_fail $? "Unexpected output: $MYSRCHSTRG is not in output"
}

function TC_ascii-xfr-r()
{
	tc_register "ascii-xfr -r"
	ascii-xfr -r $TCTMP/tstfile2.txt < $TCTMP/tstfile.txt >$stdout 2>/dev/null
	tc_fail_if_bad $? "ascii-xfr cann't receive a file." || return

	grep $MYSRCHSTRG $TCTMP/tstfile2.txt &>/dev/null
	tc_pass_or_fail $? "Unexpected output: $MYSRCHSTRG is not in output"
}
################################################################################
# main
################################################################################
tc_setup

# Check if supporting utilities are available
tc_exec_or_break  $REQUIRED || exit
TC_minicom

E_value=0
TC_ascii-xfr || E_value=1
TC_ascii-xfr-r || E_value=1
exit $E_value
