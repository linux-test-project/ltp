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
# File :	dos2unix.sh
#
# Description:	Test dos2unix package
#
# Author:	Andrew Pham, apham@austin.ibm.com
#
# History:	Aug 06 2003 - Created - Andrew Pham
#
#
#		07 Jan 2004 - (apham) updated to tc_utils.source
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

TST_TOTAL=4
REQUIRED="dos2unix mac2unix ls cat"
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
function TC_dos2unix-n()
{	
	tc_register "dos2unix -n"
	
	# create a test file
	cat > $TCTMP/tstfile.txt <<-EOF
	A test file for dos2unix.
	There should be 3 ^M removed by dos2unix.
	
	
	
	That's all folks!!!
	EOF

	dos2unix -n $TCTMP/tstfile.txt $TCTMP/dos_out.txt > $stdout 2>/dev/null
	tc_fail_if_bad $? "Not available." || return

	dos2unix_chk dos $TCTMP/dos_out.txt 
	tc_pass_or_fail $? "Unexpected output: cr still there in output." 
}

function TC_dos2unix-k()
{	
	tc_register "dos2unix -nk"
	
	dos2unix -nk $TCTMP/tstfile.txt $TCTMP/dos_out.txt > $stdout 2>/dev/null
	tc_fail_if_bad $? "Not available." || return

	getdate 1 $TCTMP/tstfile.txt
	getdate 2 $TCTMP/dos_out.txt
	
	[ "$date1" == "$date2" ]
	tc_pass_or_fail $? "Unexpected output: dates are not the same:$date1, $date2."
}

function TC_mac2unix-n()
{	
	tc_register "mac2unix -n"

	# create a test file
	cat > $TCTMP/mac_tstfile.txt <<-EOF
	A test file for dos2unix.
	There should be 3 ^M removed by dos2unix.
	
	
	
	That's all folks!!!
	EOF

	mac2unix -n $TCTMP/mac_tstfile.txt $TCTMP/mac_out.txt >$stdout 2>/dev/null
	tc_fail_if_bad $? "Not available." || return
	
	dos2unix_chk mac $TCTMP/mac_out.txt 
	tc_pass_or_fail $? "Unexpected output: cr still there or cr not converted to newline." 
}


function TC_mac2unix-k()
{	
	tc_register "mac2unix -nk"
	
	mac2unix -nk $TCTMP/tstfile.txt $TCTMP/mac_out.txt > $stdout 2>/dev/null
	tc_fail_if_bad $? "Not available." || return
	
	getdate 1 $TCTMP/mac_tstfile.txt
	getdate 2 $TCTMP/mac_out.txt
	
	[ "$date1" == "$date2" ]
	tc_pass_or_fail $? "Unexpected output: the dates are not the same."
}

################################################################################
# main
################################################################################
tc_setup

# Check if supporting utilities are available
tc_exec_or_break  $REQUIRED || exit

# Test dos2unix
E_value=0
TC_dos2unix-n || E_value=$?
TC_dos2unix-k || E_value=$? 
TC_mac2unix-n || E_value=$?
TC_mac2unix-k || E_value=$?
exit $E_value

