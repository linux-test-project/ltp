#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003		                                      ##
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
# File :	pciutils.sh
#
# Description:	Test pciutils package
#
# Author:	Andrew Pham, apham@austin.ibm.com
#
# History:	Sept 09 2003 - Created AP
#
#		07 Jan 2004 - (apham) updated to tc_utils.source
################################################################################
# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

TST_TOTAL=9
REQUIRED="lspci setpci wc"
################################################################################
	
################################################################################
# testcase functions
################################################################################
function test1()
{
	tc_register "lspci -v"
	lspci -v >$TCTMP/v.res 2>$stderr
	tc_fail_if_bad $? "Not available" || return

	[ -s $TCTMP/v.res ]
	tc_pass_or_fail $? "Unexpected output."
}

function test2()            
{
	tc_register "lspci -vv"
	lspci -v >$TCTMP/vv.res 2>$stderr
	tc_fail_if_bad $? "Not available" || return

	read size1 x < <(wc -l $TCTMP/v.res)
	read size2 x < <(wc -l $TCTMP/vv.res)

	[ -z $size1 -o -z $size2 ] && {
		tc_break_if_bad $? "Unable to get the sizes of $TCTMP/v.res \
		and $TCTMP/vv.res"
		return
	}	
	[ $size1 -le $size2 ]
	tc_pass_or_fail $? "Unexpected output."
}

function test3()            
{
	tc_register "lspci -x"
	lspci -x >$TCTMP/x.res 2>$stderr
	tc_fail_if_bad $? "Not available" || return

	grep -q "00:" $TCTMP/x.res &&
	grep -q "10:" $TCTMP/x.res &&
	grep -q "20:" $TCTMP/x.res &&
	grep -q "30:" $TCTMP/x.res 
	tc_pass_or_fail $? "Unexpected output."
}

function test4()            
{
	tc_register "lspci -m"
	lspci -m >$TCTMP/m.res 2>$stderr
	tc_fail_if_bad $? "Not available" || return

	[ -s $TCTMP/m.res ]
	tc_pass_or_fail $? "Unexpected output."
}

function test5()            
{
	tc_register "lspci -b"
	lspci -b >$TCTMP/b.res 2>$stderr
	tc_fail_if_bad $? "Not available" || return

	[ -s $TCTMP/b.res ]
	tc_pass_or_fail $? "Unexpected output."
}

function test6()            
{
	tc_register "lspci -n"
	lspci -n >$TCTMP/n.res 2>$stderr
	tc_fail_if_bad $? "Not available" || return

	[ -s $TCTMP/n.res ]
	tc_pass_or_fail $? "Unexpected output."
}

function test7()            
{
	tc_register "setpci -d"
	setpci -d *:* latency_timer device_id vendor_id >$TCTMP/d.res 2>$stderr
	tc_fail_if_bad $? "Not available" || return

	[ -s $TCTMP/d.res ]
	tc_pass_or_fail $? "Unexpected output."
}

function test8()            
{
	tc_register "setpci -s"
	setpci -s *:*.* latency_timer device_id vendor_id >$TCTMP/x.res 2>$stderr
	tc_fail_if_bad $? "Not available" || return

	[ -s $TCTMP/x.res ]
	tc_pass_or_fail $? "Unexpected output."
}

function test9()            
{
	tc_register "setpci latency_timer=40"
	local RC=0

	read Vid x < <(setpci -d *:* vendor_id)
	read Did z < <(setpci -d *:* device_id)

# 	echo "Vid = $Vid; Did = $Did"

	if [ -z $Vid -o -z $Did ]; then
		tst_resm TBROK "Unable to get vendor_id and device_id"
		return
	fi
	
	local old=`setpci -d $Vid:$Did latency_timer`
	setpci -d $Vid:$Did latency_timer=40 >$stdout 2>$stderr
	tc_fail_if_bad $? "Not available" || return

	setpci -d $Vid:$Did latency_timer | grep -q 40 >&/dev/null
	tc_pass_or_fail $? "Unexpected output." || return 

	setpci -d $Vid:$Did latency_timer=$old >&/dev/null
}

################################################################################
# main
################################################################################
tc_setup

# Check if supporting utilities are available
tc_exec_or_break  $REQUIRED || exit

E_value=0
i=1
while [ $i -lt 10 ]
do
	test$i || E_value=1
	let i+=1
done
exit $E_value
