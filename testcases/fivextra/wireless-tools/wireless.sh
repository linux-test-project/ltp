#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003		                                      ##
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
# File :	wireless.sh
#
# Description:	Test the wireless-tools package
#
# Author:	Robb Romans <robb@austin.ibm.com>
#
# History:	Oct 14 2003 - created - RR
#		08 Jan 2004 - (RR) updated to tc_utils.source
#		11 Feb 2004 (rcp) fixed test for wireless executables.
#
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

# global variables
#
REQUIRED="grep"

INTERFACE=""
ADDRESS="192.168.2.88"
ESSID="FIVTEST"
CHANNEL="7"
MODE="Ad-Hoc"
AP=""
KEY="01:01:02:03:04"

function tc_local_setup()
{
	tc_exec_or_break route || return
	set $(route | grep default)
	INTERFACE=$8
	[ "$INTERFACE" ]
	tc_break_if_bad $? "can't find network interface" || return
}

################################################################################
# testcase functions
################################################################################

function test_01 {

	tc_register "Test that required wireless-tools programs exist"
	tc_executes iwconfig iwgetid iwlist iwpriv iwspy
	tc_pass_or_fail "$?" "wireless-tools executables not properly installed" || exit
}

function test_02 {

	tc_register "Test operation of iwconfig on $INTERFACE"
	iwconfig $INTERFACE | grep -v "no wireless extensions" &>/dev/null
	tc_fail_if_bad "$?" "$INTERFACE does not appear to be a wireless device" || exit
	ifconfig $INTERFACE $ADDRESS up 1>$stdout 2>$stderr
	tc_pass_or_fail "$?" "Setting address $ADDRESS on $INTERFACE failed"
}

function test_03a {

	tc_register "Test setting essid $ESSID"
	iwconfig $INTERFACE essid $ESSID 1>$stdout 2>$stderr
	tc_pass_or_fail "$?" "Setting essid on $INTERFACE failed"
}

function test_03b {

	tc_register "Test setting channel $CHANNEL"
	iwconfig $INTERFACE channel $CHANNEL 1>$stdout 2>$stderr
	tc_pass_or_fail "$?" "Setting channel on $INTERFACE failed."
}

function test_03c {

	tc_register "Test setting sensitivity"
	iwconfig $INTERFACE sens 2/3 1>$stdout 2>$stderr
	tc_pass_or_fail "$?" "Setting sensitivity on $INTERFACE failed."
}

function test_03d {

	tc_register "Test setting mode $MODE"
	iwconfig $INTERFACE mode $MODE 1>$stdout 2>$stderr
	tc_pass_or_fail "$?" "Setting mode on $INTERFACE failed."
}

function test_03e {

	tc_register "Test setting access point $AP"
	iwconfig $INTERFACE ap $AP 1>$stdout 2>$stderr
	tc_pass_or_fail "$?" "Setting access point on $INTERFACE failed."
}

function test_03g {

	tc_register "Test setting bitrate"
	iwconfig $INTERFACE rate auto 1>$stdout 2>$stderr
	tc_pass_or_fail "$?" "Setting bitrate on $INTERFACE failed."
}

function test_03h {

	tc_register "Test setting fragmentation_threshold"
	iwconfig $INTERFACE frag off 1>$stdout 2>$stderr
	tc_pass_or_fail "$?" "Setting fragmentation threshold on $INTERFACE failed."
}

function test_03i {

	tc_register "Test setting encryption key"
	iwconfig $INTERFACE key $KEY 1>$stdout 2>$stderr
	tc_pass_or_fail "$?" "Setting encryption key on $INTERFACE failed."
}

function test_03j {

	tc_register "Test setting power management"
	iwconfig $INTERFACE power period 2 1>$stdout 2>$stderr
	tc_fail_if_bad "$?" "Setting power management on $INTERFACE failed." || return

	iwconfig $INTERFACE power off 1>$stdout 2>$stderr
	tc_pass_or_fail "$?" "Setting power management off on $INTERFACE failed."
}

function test_03k {

	tc_register "Test setting transmit power"
	iwconfig $INTERFACE txpower auto 1>$stdout 2>$stderr
	tc_pass_or_fail "$?" "Setting transmit power on $INTERFACE failed."
}

function test_04 {

	tc_register "Test operation of iwgetid"
	tc_executes "iwgetid $INTERFACE"
	tc_pass_or_fail "$?" "iwgetid not found/not executable"
}

function test_05 {

	tc_register "Test operation of iwlist"
	iwlist $INTERFACE channel 1>$stdout 2>$stderr
	tc_pass_or_fail "$?" "iwlist did not execute correctly"
}

function test_06 {

	tc_register "Test operation of iwpriv"
	iwpriv $INTERFACE 1>$stdout 2>$stderr
	tc_pass_or_fail "$?" "iwpriv not found/not executable"
}

function test_07 {

	tc_register "Test operation of iwspy"
	iwspy $INTERFACE 1>$stdout 2>$stderr
	tc_pass_or_fail "$?" "iwspy did not execute correctly"

}
function test_08 {

	tc_register "Test I/O operation of wireless network"
	ping -c 1 $ADDRESS 1>$stdout 2>$stderr
	tc_pass_or_fail "$?"
}


####################################################################################
# MAIN
####################################################################################

# Function:	main
#

#
# Returns:	- zero on success
#		- non-zero on failure
#
TST_TOTAL=15
tc_setup
tc_root_or_break || exit
tc_exec_or_break $REQUIRED || exit
test_01
tc_info "Operating on wireless interface $INTERFACE"
test_02
test_03d
test_03a
test_03b
test_03c
#test_03e
test_03g
test_03h
test_03i
test_03j
# test_03k
test_04
test_05
test_06
test_07
test_08
ifconfig $INTERFACE down &>/dev/null
