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
# File :	unix2dos.sh
#
# Description:	Test unix2dos package
#
# Author:	Andrew Pham, apham@austin.ibm.com
#
# History:	Aug 13 2003 - Created - Andrew Pham
#
#		07 Jan 2004 - (apham) updated to tc_utils.source
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

TST_TOTAL=2
REQUIRED="unix2dos ls cat"
date1="no"
date2="yes"

# a function to return the date of a given file.
function getdate()
{
	local cnt=0
	for i in `ls -l $2`
	do
		let cnt+=1
		[ $cnt -eq 6 ] && d=$i && break
	done
	[ $1 -eq 1 ] && date1=$d
	[ $1 -eq 2 ] && date2=$d
}
################################################################################
# testcase functions
################################################################################
function TC_unix2dos-n()
{	
	tc_register "unix2dos -n"
	
	# create a test file
	cat > $TCTMP/tstfile.txt <<-EOF
	A test file for unix2dos.
	Newline should be converted to cr plus newline by unix2dos.
	That's all folks!!!
	EOF

	unix2dos -n $TCTMP/tstfile.txt $stdout 2>/dev/null
	tc_fail_if_bad $? "Not available." || return

	unix2dos_chk $stdout
	tc_pass_or_fail $? "Unexpected output: newline is not converted to cr." 
}

function TC_unix2dos-k()
{	
	tc_register "unix2dos -nk"
	
	unix2dos -k -n $TCTMP/tstfile.txt $TCTMP/k_out.txt > $stdout 2>/dev/null
	tc_fail_if_bad $? "Not available." || return

	getdate 1 $TCTMP/tstfile.txt
	getdate 2 $TCTMP/k_out.txt
	
	[ "$date1" == "$date2" ]
	tc_pass_or_fail $? "Unexpected output: dates are not the same:$date1, $date2."
}
################################################################################
# main
################################################################################
tc_setup

# Check if supporting utilities are available
tc_exec_or_break  $REQUIRED || exit

FRC=0
# Test unix2dos
TC_unix2dos-n || FRC=1 
TC_unix2dos-k || FRC=1 
exit $FRC
