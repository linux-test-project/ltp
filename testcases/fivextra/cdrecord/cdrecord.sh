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
# File :	cdrecord.sh
#
# Description:	Test cdrecord package
#
# Author:	Andrew Pham, apham@austin.ibm.com
#
# History:	Jul 02 2003 - Created - Andrew Pham
#		Aug 01 2003 - Fixed tc scgcheck.  AH. Pham
#
#		06 Jan 2004 - (apham) updated to tc_utils.source
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

TST_TOTAL=1
REQUIRED="expect grep cdrecord awk "

################################################################################
# testcase functions
function test_manually()
{
        tc_info "$TCNAME must be tested manually."
}

################################################################################
function TC_cdrecord()
{	
	TCNAME=cdrecord
	test_manually	
	return 0
}

function TC_readcd()
{	
	TCNAME=readcd
	test_manually	
	return 0
}
function TC_scgcheck()
{	
	local MyDev="`cdrecord -scanbus | grep -v '*' | \
		grep -E '[0-9],[0-9],[0-9]' | awk '{print $1}'`"
	local RC=0



	[ -z "$MyDev" ] && echo "There is no cd-w device or scci driver" && return 1

	set $MyDev
	MyDev=$1
	
        # create an expect file
	local expcmd=`which expect`

	cat > $TCTMP/exp_script <<-EOF
		#!$expcmd -f
		proc abort {} { exit 1 }
		set timeout 10
		spawn scgcheck -vv -f $TCTMP/scgcheck.res.txt
		expect {
				timeout abort
				"\[0,6,0\]:" { send "$MyDev\r" }
			}
		expect {
				timeout abort
				"continue:" { send "\r" }
			}
		expect {
				timeout abort
				"continue:" { send "\r" }
			}
		expect {
				timeout abort
				"continue:" { send "\r" }
			}
		expect {
				timeout abort
				"continue:" { send "\r" }
			}
		expect {
				timeout abort
				"continue:" { send "\r" }
			}
		expect {
				timeout abort
				"continue:" { send "\r" }
			}
		expect {
				timeout abort
				"continue:" { send "\r" }
			}
	EOF
	
	chmod +x $TCTMP/exp_script
	$TCTMP/exp_script >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected response from scgcheck" \
		"scgcheck output =======================================" \
		"`cat $TCTMP/scgcheck.res.txt`" \
		"=======================================================" \
		|| return

	#below are the failures I found on all machine I tested.	
	cat > $TCTMP/scgcheck.knownfails.txt <<-EOF
		sense count
		residual count
		DMA overrun
		EOF
	
	grep FAILED $TCTMP/scgcheck.res.txt > $TCTMP/scgcheck.err.txt 2>/dev/null
	grep -v -f $TCTMP/scgcheck.knownfails.txt $TCTMP/scgcheck.err.txt \
		>$stderr 2>/dev/null
	
	[ -s $stderr ]  && RC=1 
	tc_pass_or_fail $RC "scgcheck output =======================================" \
	                "`cat $TCTMP/scgcheck.res.txt`" \
			"======================================================="
	 return
}

################################################################################
# main
################################################################################
tc_setup

tc_root_or_break || exit 1

[ "$TCTMP" ] && rm -rf $TCTMP/*

TC_cdrecord
TC_readcd

# Check if supporting utilities are available
tc_exec_or_break  $REQUIRED || exit 1

tc_register "scgcheck"
TC_scgcheck
