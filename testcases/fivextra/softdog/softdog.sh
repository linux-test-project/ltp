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
# File :	softdog.sh
#
# Description:	Test the software watchdog driver for the Linux system
#
# 		Softdog will monitor your system and try to reboot it if
#		it becomes unable to run. Note that you MUST have installed
#		the kernel software watchdog in order for this software to 
#		work. 
#
# Author:	Yu-Pao Lee, yplee@us.ibm.com
#
# History:	Mar 31 2003 - Created. Yu-Pao Lee, yplee@us.ibm.com
#		17 Dec 2003 - (rcp) updated to tc_utils.source
#		11 May 2004 (rcp) exit as BROK if not run with the
#			"don't blame me" option.

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# local utility functions
################################################################################

function check_running()
{
	# check to see if softdog is already running
	! lsmod | grep -q softdog &>/dev/null
	tc_break_if_bad $? "Softdog module is already loaded." \
			   "This testcase cannot be run twice without a reboot inbetween."
}

function usage()
{
	tc_warn "Running this testcase will load the softdog kernel"
	tc_warn "module and the softdog daemon. You SHOULD NOT kill the"
	tc_warn "daemon because if you do, the module will REBOOT your"
	tc_warn "system. You CANNOT unload the module."
	tc_warn ""
	tc_warn "	YOU HAVE BEEN WARNED!"
	tc_warn ""
	tc_warn "Because this testcase can potentially cause the system"
	tc_warn "to reboot, it will not run unless you invoke it with"
	tc_warn "the command-line argument \"don't blame me\"."
	return 0
}

function check_blame()
{
	if [ "$1" != "don't blame me" ] ; then 
		usage
		tc_break_if_bad 1 "SEE WARNINGS ABOVE"
		return
	fi
	return 0
}

################################################################################
# the testcase functions
################################################################################

function test01()	
{
	tc_register "start softdog"
	tc_exec_or_break uname lsmod grep || return

	local mod_dir="/lib/modules/`uname -r`/kernel/drivers/char"

	# ensure the software watchdog is present
	tc_exist_or_break $mod_dir/softdog.o
	tc_fail_if_bad $? "softdog module not available" || return

	# ensure the device file is present
	tc_exist_or_break /dev/watchdog
	tc_fail_if_bad $? "softdog devices not available" || return

	# start the softdog daemon
	softdog &>$stdout	# softdog stupidly puts normal output in stderr
	tc_fail_if_bad $? "Unexpected results starting softdog daemon" || return

	# verify the module was loaded by the daemon
	lsmod >$stdout 2>$stderr
	grep "softdog" <$stdout >/dev/null
	tc_fail_if_bad $? "lsmod failed to list softdog." || return

	# wait 2 minutes, hoping we do not reboot.
	tc_warn "The softdog kernel module is loaded. It will REBOOT"
	tc_warn "your system if you kill the softdog daemon!"
	tc_warn ""
	tc_warn "	YOU HAVE BEEN WARNED!"
	tc_warn ""
	tc_warn "Waiting 2 miuntes to ensure the softdog daemon prevents reboot"
	tc_warn ""
	tc_warn "	IF THE SYSTEM REBOOTS THEN THIS TEST FAILED!"
	tc_warn ""
	sleep 120

	# OK, did not reboot. Never fails if we get this far
	tc_pass_or_fail 0 ""
}

################################################################################
# main
################################################################################

TST_TOTAL=1

tc_setup			# standard setup

tc_root_or_break || exit
check_blame "$1" || exit
check_running || exit

test01 

