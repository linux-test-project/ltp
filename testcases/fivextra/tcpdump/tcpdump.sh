#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003						      ##
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
# File :	tcpdump.sh
#
# Description:	Test tcpdump package
#
# Author:	Andrew Pham, apham@austin.ibm.com
#
# History:	June 09 2003 - Created - Andrew Pham
#		July 07 2003 - separated the long testcase into smaller ones.
#		Aug  19 2003 - Fixed [ -n $dn ] -> [ -n "$dn" ]
#		Oct  24 2003 - Optimized some and enable all TCs to be executed
#			       even when one TC failec.	
#
#		07 Jan 2004 - (apham) updated to tc_utils.source
#		26 Feb 2004 (rcp) Changed -c count from 3 to 9. Test 2
#				sometimes failed at 3. Not sure if this is the
#				fix, but it looked like it wasn't reading
#				enough packets to see the expected data.
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

REQUIRED="tcpdump grep wc ping cat"
################################################################################
# testcase functions
################################################################################
function test01()
{	
	tc_register "tcpdump -c 2"

	tcpdump -c 2 >$stdout 2>$stdout
	tc_fail_if_bad $? "is not working." || return

	wc -l $stdout | grep -q 2
	tc_pass_or_fail $? "Unexpected output.  Expected 2 lines (only) in stdout"
}

function test02()
{
	tc_register "tcpdump -c 9 -a"
	
	tcpdump -c 9 -a &>$stdout
	tc_fail_if_bad $? "is not working." || return

	grep -q $dn $stdout 2>$stderr
	tc_pass_or_fail $? "Expected $dn in stdout." || return  
}

function test03()
{
	tc_register "tcpdump -c 9 -n"
	
	tcpdump -c 9 -n &>$stdout
	tc_fail_if_bad $? "is not working." || return 

	grep -q $dn $stdout 2>$stderr
	[ $? -ne 0 ]
	tc_pass_or_fail $? "Expected $dn NOT in stdout" || return  
}

function test04()
{
	tc_register "tcpdump -c 9 -t"
	
	tcpdump -c 9 -t &>$stdout
	tc_fail_if_bad $? "is not working." || return  

	grep -E "^[0-9]{2}:[0-9]{2}:[0-9]{2}." $stdout &> /dev/null
	[ $? -ne 0 ]
	tc_pass_or_fail $? "NOT expected time stamp in stdout"
}

function test05()
{
	tc_register "tcpdump -c 9 -X"
	
	tcpdump -c 9 -X &>$stdout
	tc_fail_if_bad $? "is not working." || return  

	grep -E "^[0-9]x[0-9]{4}" $stdout &> /dev/null 
	tc_pass_or_fail $? "Expected HEX in stdout"
}

function test06()
{
	tc_register "tcpdump -c 9 -i lo"
	ping -c 10 localhost >/dev/null &
	killit=$!
	
	tcpdump -c 9 -i lo &>$stdout
	tc_fail_if_bad $? "is not working." || return  

	grep icmp $stdout &>/dev/null
	tc_pass_or_fail $? "Expected icmp in stdout"

 	# just in case ping doesn't terminate by itself.	
	kill $killit &>/dev/null
}
################################################################################
# main
################################################################################
TST_TOTAL=6
tc_setup

# Check if supporting utilities are available
tc_exec_or_break $REQUIRED || exit

tc_root_or_break || exit

dn=`hostname`
[ "$dn" ]
tc_break_if_bad $? "networking not configured properly -- can't get hostname" \
	|| exit

i=1
while [ $i -lt 7 ]
do
	test0$i
	let i+=1;
done
