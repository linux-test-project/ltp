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
# File :	ethtool.sh
#
# Description:	Test ethtool package
#
# Author:	Andrew Pham, apham@austin.ibm.com
#
# History:	Sept 02 2003 - Created - Andrew Pham
#		07 Jan 2004 - (apham) updated to tc_utils.source
#		07 May 2004 (rcp) turn off autoneg when setting speed/duplex.
#			Save original attributes and restore at the end.
#			Other cleanup.
#
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

REQUIRED="ifconfig grep"
INTERFACE="none"
################################################################################
# utility functions
################################################################################

function get_intr()
{
	local cnt=0
	while [ $cnt -lt 3 ]
	do
		if ifconfig eth$cnt >&/dev/null ; then
			INTERFACE=eth$cnt
			break
		fi
	done
	
	[ "$INTERFACE" != "none" -a -n "$INTERFACE" ]
	tc_break_if_bad $? "Unable to get an interface"
}

function get_attributes()
{
	ethtool $INTERFACE >$stdout 2>$stderr
	tc_break_if_bad $? || return
	while read attribute value junk; do
		[ "$attribute" = "Speed:" ] && SPEED=${value/Mb\/s/} && continue
		[ "$attribute" = "Duplex:" ] && DUPLEX=$value && continue
		[ "$attribute" = "Auto-negotiation:" ] && AUTONEG=$value && continue
	done < $stdout
	[ "$SPEED" -a "$DUPLEX" -a "AUTONEG" ]
	tc_break_if_bad $? "Can't get current attributes for $INTERFACE"
	DUPLEX=${DUPLEX/F/f} # Full -> full
	DUPLEX=${DUPLEX/H/h} # Half -> half
	tc_info "original $INTERFACE attributes: SPEED=$SPEED DUPLEX=$DUPLEX AUTONEG=$AUTONEG"
}

function tc_local_setup()
{
	get_intr || retutrn
	get_attributes || return
	tc_exec_or_break $REQUIRED || return
}

function tc_local_cleanup()
{
	[ "$SPEED" -a "$DUPLEX" -a "AUTONEG" ] &&
	tc_info "restoring: ethtool -s $INTERFACE speed $SPEED duplex $DUPLEX autoneg $AUTONEG"
	ethtool -s $INTERFACE speed $SPEED duplex $DUPLEX autoneg $AUTONEG >&/dev/null
}	
################################################################################
# testcase functions
################################################################################

function test01()
{
	tc_register "installation check"
	tc_executes ethtool
	tc_pass_or_fail $? "ethtool not installed"
}

function test02()
{
        tc_register "ethtool"
	

	ethtool $INTERFACE >$stdout 2>$stderr
	tc_fail_if_bad $? "Not available." || return

	grep -q "Supported" $stdout &&
	grep -q "Speed" $stdout &&
	grep -q "Wake-on" $stdout &&
	grep -q "Auto-negotiation" $stdout 
	tc_pass_or_fail $? "Unexpected output."
}

function test03()
{
	tc_register "ethtool -i"
	ethtool -i $INTERFACE >$stdout 2>$stderr
	tc_fail_if_bad $? "Not available." || return

	grep -q "driver" $stdout &&
	grep -q "firmware" $stdout
	tc_pass_or_fail $? "Unexpected output." || return
}

function test04()
{
	tc_register "ethtool -d"
	ethtool -d $INTERFACE >$stdout 2>$stderr
	tc_fail_if_bad $? "Not available." || return

	grep -q "SCB Status Word" $stdout &&
	grep -q "SCB Command Word" $stdout
	tc_pass_or_fail $? "Unexpected output." || return
}

function test05()
{
	tc_register "ethtool -e"
	ethtool -e $INTERFACE >$stdout 2>$stderr
	tc_fail_if_bad $? "Not available." || return

	[ -s $stdout ]
	tc_pass_or_fail $? "Unexpected output." || return
}

function test06()
{
	tc_register "ethtool -t online"
	ethtool -t $INTERFACE online >$stdout 2>$stderr
	tc_fail_if_bad $? "Not available." || return

	grep -q "PASS" $stdout
	tc_pass_or_fail $? "Unexpected output." || return
}

function test07()
{
	tc_register "ethtool -s $INTERFACE speed 10 duplex half autoneg off"
	tc_info "Possible one minute delay ..."
	ethtool -s $INTERFACE speed 10 duplex half autoneg off >$stdout 2>$stderr
	tc_fail_if_bad $? "Not available." || return

	ethtool $INTERFACE >$stdout 2>$stderr
	grep "Speed" $stdout | grep -q "10Mb" &&
	grep "Duplex" $stdout | grep -q Half 
	tc_pass_or_fail $? "Unexpected output." || return
}

################################################################################
# main
################################################################################
TST_TOTAL=7
tc_setup

test01 &&
test02 &&
test03 &&
test04 &&
test05 &&
test06 &&
test07
