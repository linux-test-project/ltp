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
# File :	hdparm.sh
#
# Description:	Test hdparm package
#
# Author:	Robb Romans <robb@austin.ibm.com>
#
# History:	Jul 31 2003 - created - RR
#		Aug 14 2003 - reviewed by RCP.
#		Oct 07 2003 - modify to work with ppc -RR
#		06 Jan 2004 - (RR) updated to tc_utils.source,
#				add TST_TOTAL
#		08 Apr 2004 (rcp) removed dependency on awk
#		09 Apr 2004 (rcp) BUG:7433 - more robust check for root fs dev.
#
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

REQUIRED="cat grep mount"
device=""  # set by find_disk()

################################################################################
# testcase functions
################################################################################

function test01 {
	tc_register "Is hdparm installed?"
	tc_executes hdparm
	tc_pass_or_fail $? "Hdparm is not properly installed"
}

function find_disk {
	set $(mount | grep '^/.* / '); device=$1
	[ -b "$device" ]
	tc_fail_if_bad $? "Unable to determine roof fs device. ($device)"
	tc_info "hdparm to run on $device"
}

function test02 {
	tc_register "Does hdparm execute properly?"
	hdparm $device >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected response from hdparm command" || exit
	[ -s $stdout ]
	tc_pass_or_fail $? "Hdparm did not write anything to stdout"
}

function test03 {
	tc_register "Get sector count for filesystem read-ahead"
	hdparm -a $device >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected response from hdparm command" || return
	[ -s $stdout ]
	tc_pass_or_fail $? "Hdparm did not write anything to stdout"
}

function test04 {
	tc_register "Get bus state"
	hdparm -b $device >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected response from hdparm command" || return
	[ -s $stdout ]
	tc_pass_or_fail $? "Hdparm did not write anything to stdout"
}

function test05 {
	tc_register "Display the drive geometry"
	hdparm -g $device >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected response from hdparm command" || return
	[ -s $stdout ]
	tc_pass_or_fail $? "Hdparm did not write anything to stdout"
}

function test06 {
	tc_register "Request identification info directly"
	hdparm -I $device >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected response from hdparm command" || return
	[ -s $stdout ]
	tc_pass_or_fail $? "Hdparm did not write anything to stdout"
}

function test07 {
	tc_register "Get the keep_settings_over_reset flag"
	hdparm -k $device >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected response from hdparm command" || return
	[ -s $stdout ]
	tc_pass_or_fail $? "Hdparm did not write anything to stdout"
}

function test08 {
	tc_register "Get read-only flag for device"
	hdparm -r $device >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected response from hdparm command" || return
	[ -s $stdout ]
	tc_pass_or_fail $? "Hdparm did not write anything to stdout"
}

function test09 {
	tc_register "Perform  timings  of  cache + device  reads (please wait)"
	hdparm -Tt $device >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected response from hdparm command" || return
	[ -s $stdout ]
	tc_pass_or_fail $? "Hdparm did not write anything to stdout"
}

function test10 {
	tc_register "Query (E)IDE 32-bit I/O support"
	hdparm -c $device >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected response from hdparm command" || return
	[ -s $stdout ]
	tc_pass_or_fail $? "Hdparm did not write anything to stdout"
}

function test11 {
	tc_register "Check the current IDE power mode status"
	hdparm -C $device >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected response from hdparm command" || return
	[ -s $stdout ]
	tc_pass_or_fail $? "Hdparm did not write anything to stdout"
}

function test12 {
	tc_register "Get sector count for multiple sector IDE I/O"
	hdparm -m $device >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected response from hdparm command" || return
	[ -s $stdout ]
	tc_pass_or_fail $? "Hdparm did not write anything to stdout"
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
TST_TOTAL=3
tc_setup
tc_exec_or_break $REQUIRED || exit
tc_root_or_break || exit
test01 &&
find_disk &&
test02 &&
test03

# IDE?
if echo $device | grep 'hd' &>/dev/null || \
	echo $device | grep 'ide' &>/dev/null ; then
	TST_TOTAL=12
	test04 &&
	test05 &&
	test06 &&
	test07 &&
	test08 &&
	test09 &&
	test10 &&
	test11 &&
	test12
else
	tc_info "hdparm does not support further operations on non-IDE systems"
fi

