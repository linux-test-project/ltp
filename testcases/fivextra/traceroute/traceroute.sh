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
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.							      ##
##									      ##
## You should have received a copy of the GNU General Public License	      ##
## along with this program;  if not, write to the Free Software		      ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##									      ##
################################################################################
#
# File :	traceroute.sh
#
# Description:	Test Basic functionality of traceroute command.
#		Test #1: execute traceroute on hostname, expected number of
#		hops is 1.
#
# Author:	Manoj Iyer, manjo@mail.utexas.edu
#
# History:	Mar 03 2003 - Created - Manoj Iyer.
#		Mar 19 2003 - Modified - Robert Paulsen - added code to set 
#			path, so that test can be run as non-root.
#		Mar 24 2003 - Modified - Manoj Iyer - fixed testcase to 
#			count the number of hops correctly.
#		Sep 29 2003 - Modified - Manoj Iyer - using tc_utils.source
#		Oct 06 2003 - Modified - Manoj Iyer - removed tc_local_setup, 
#			seems to be broken so replaced with tst_setup
#		08 Jan 2004 - (rcp) updated to tc_utils.source
#			general cleanup and simplification (no required
#			utility commands like head, tail, cat, awk, etc.)

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# testcase functions
################################################################################

#
# test01
#
function test01()
{
	tc_register	"installation check"
	tc_executes traceroute
	tc_pass_or_fail $? "traceroute not installed properly"
}

#
# test02	Test that traceroute hostname will trace route of an IP 
#				packet to that host.
#
function test02()
{
	tc_register	"traceroute functionality"
	thishost=$(hostname --long)

	# traceroute to self, max hops 25, 50 byte packets.
	traceroute -m 25 $thishost 50 >$stdout 2>$stderr
	tc_fail_if_bad $? "failed to traceroute to $thishost" || return

	# Only one hop is required to get to hostname. 
	while read x y z; do
		nhops=$x; foundhost=$y
	done < $stdout
	[ "$nhops" -eq 1 ] && [ "$thishost" = "$foundhost" ]
	tc_pass_or_fail $? \
		"expected to take only 1 hop count, but got \"$nhops\" instead"
}

################################################################################
# main
################################################################################

TST_TOTAL=2

tc_setup

test01 &&
test02

