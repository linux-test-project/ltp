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
# File :	hotplug.sh
#
# Description:	Test the hotplug notification capability. This takes advantage
#		of the ability to simulate hotplug programmatically.
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Mar 08 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		15 Dec 2003 (rcp) updated to tc_utils.source
#		27 May 2004 (rcp) bug 6341

# source the utility functions
me=`which $0`
LTPBIN=${me%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

hpdir="/etc/hotplug"
hpprocdir="/proc/sys/kernel/hotplug"
initscr="/etc/init.d/hotplug"
daemonstarted=""

################################################################################
# local utility functions
################################################################################

#
# my setup
#
function tc_local_setup()
{
	tc_root_or_break || return

	[ -d /proc/bus/usb ] || {
		tc_exist_or_break /etc/sysconfig/hotplug || return
		tc_exec_or_break sed || return
		mv /etc/sysconfig/hotplug $TCTMP/hotplug-config
		sed 's/HOTPLUG_START_USB=yes/HOTPLUG_START_USB=no/' \
			$TCTMP/hotplug-config > /etc/sysconfig/hotplug
	}
	return 0
}

#
# my cleanup
#
function tc_local_cleanup()
{
	[ -f $TCTMP/hotplug-config ] && mv $TCTMP/hotplug-config /etc/sysconfig/hotplug
	rm -rf $hpdir/hp$$.agent &>/dev/null
	[ "$daemonstarted" = "yes" ] && $initscr stop
}

################################################################################
# the testcase functions
################################################################################

#
# test01	installation check
#
function test01()
{
	tc_register "installation check"
	tc_exists $hpprocdir $hpdir $initscr
	tc_fail_if_bad $? "not properly installed" || return

	local command=`cat $hpprocdir`
	tc_executes $command
	tc_pass_or_fail $? "not properly installed"
}

#
# test02	simulate a hotplug event
#
function test02()
{
	tc_register "test simulated hotplug event"
	tc_exec_or_break cat chmod || return

	# add hotplug agent for testing
	local msg="FIV HOTPLUG $TCID$$"
	cat > $hpdir/hp$$.agent <<-EOF
		#!$SHELL
		echo "$msg" > $TCTMP/$$result
	EOF
	chmod +x $hpdir/hp$$.agent

	# be sure hotplug daemon is running
	if ! $initscr status >/dev/null ; then
		daemonstarted=yes
		$initscr start >$stdout 2>$stderr
		tc_fail_if_bad $? "hotplug daemon did not start" || return
	fi

	# simulate the hotplug event
	tc_info "simulate hotplug event hp$$"
	/sbin/hotplug hp$$ 2>$stderr >$stdout
	tc_fail_if_bad $? "bad response from hotplug command" || return

	# see that result file was created
	[ -e $TCTMP/$$result ]
	tc_fail_if_bad $? "hotplug agent didn't run" || return

	# see that result file has correct contents
	cat $TCTMP/$$result | grep "$msg" &>/dev/null
	tc_pass_or_fail $? "hotplug service didn't call hotplug agent"
}

################################################################################
# main
################################################################################

TST_TOTAL=2

tc_setup

test01 &&
test02
