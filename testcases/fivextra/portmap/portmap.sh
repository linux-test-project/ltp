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
# File :	portmap.sh
#
# Description:	Test portmap
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Feb 27 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		Oct 21 2003 - Updated to use the lastest features of
#				utilitry.source so that this can be used as
#				a template for writing testcases.
#		16 Dec 2003 - (robert) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

portmapinit="/etc/init.d/portmap"
must_stop_portmap="no"

################################################################################
# utility functions specific to this script
################################################################################

#
# tc_local_setup
#
#	This function is called automatically by the "tc_setup" function.
#
function tc_local_setup()
{
	tc_root_or_break || return	# this tc must be run as root
}

#
# tc_local_cleanup
#
#	This function is called automatically when your testcase exits
#	for any reason.
#
function tc_local_cleanup()
{
	[ $must_stop_portmap = "yes" ] && $portmapinit stop
}


################################################################################
# the testcase functions
################################################################################

#
# test01	check that portmap is installed
#
function test01()
{
	tc_register	"installation check"
	tc_executes $portmapcmd $portmapinit
	tc_pass_or_fail $? "portmap not installed"
}

#
# test02	start portmap if not already running
#
function test02()
{
	tc_register	"start portmap if not already running"

	if $portmapinit status >$stdout 2>$stderr ; then
		tc_info "portmap is already running"
		tc_pass_or_fail $? "this message will never show"
		return
	fi
	must_stop_portmap="yes"

	$portmapinit start >$stdout 2>$stderr
	tc_pass_or_fail $? "could not start portmap"
}

#
# test03	is portmap working
#
function test03()
{
	tc_register	"is portmap working"

	local rpcinfocmd="rpcinfo -p"
	$rpcinfocmd >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected response from $rpcinfocmd" || return

	grep -q ".*tcp.*portmapper" $stdout 2>$stderr && \
	grep -q ".*udp.*portmapper" $stdout 2>>$stderr
	tc_pass_or_fail $? "expected to see udp and tcp portmappers in stdout" \
			"command issued was $rpcinfocmd"
}

################################################################################
# main
################################################################################

TST_TOTAL=3

tc_setup			# standard setup

test01 &&
test02 &&
test03
