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
# File :	atalkd_tests.sh
#
# Description: This program tests basic functionality of atalkd deamon
#
# Author:	Manoj Iyer  manjo@mail.utexas.edu
#
# History:	July 31 2003 - created - Manoj Iyer
#		- Thanks to Robb Romans for figuring out my broken
#		  cleanup function mystery.
#		Sept 24 2003 - Modified - Manoj Iyer
#		   - suse uses /var/log/messages instead of /var/log/syslog. 
#		   - There is no easy way telling where the messages will go
#		   - so this kludge.
#		Oct 24 2003 - Modified - Manoj Iyer
#		   - discover a working interface... eth0 need not be configured
#		   always.
#		08 Jan 2004 - (RR) updated to tc_utils.source
#		21 Jan 2004 - (rcp) general cleanup
#				remove dependency on awk


################################################################################
# source the standard utility functions
################################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

atalkd_was_running=0

################################################################################
# utility functions specific to this script
################################################################################

#
# tc_local_setup
#
function tc_local_setup()
{
	tc_root_or_break || exit
	tc_exec_or_break  grep cat ps || exit

	# check if atalkd is already running, kill it if its alive.
	ps -C atalkd | grep atalkd &>/dev/null && { 
		tc_info "atalkd is already running... stopping..."
		atalkd_was_running=1 
		killall -9 atalkd 
	}
	return 0
}

#
# tc_local_cleanup
# 
function tc_local_cleanup()
{
	# kill any instance that was started by the test and start default.
	tc_info "\"Killed\" message is OK ..."
	killall -9 /usr/sbin/atalkd &>/dev/null
	[ $atalkd_was_running -eq 1 ] && { /usr/sbin/atalkd &>/dev/null; }
}

################################################################################
# the test functions
################################################################################

#
# test01	Installation check
#
function test01()
{
	tc_register	"installation check"
	tc_executes atalkd
	tc_pass_or_fail $? "atalkd not installed properly"
}

#
# test02	Test the atalkd deamon
#
function test02()
{
	tc_register    "start atalkd daemon"

	# create a simple config file for atalkd
	set $(/sbin/route | grep default)
	local interface=$8				# e.g. eth0
	cat > $TCTMP/tst_atalkd.config <<-EOF
	$interface -phase 2 -net 66-67 -addr 66.6 -zone "NO PARKING"
	EOF

	# capture syslog before starting atalkd
	cp /var/log/messages $TCTMP/messages
   
	# start atalkd with phase 2
	/usr/sbin/atalkd -f $TCTMP/tst_atalkd.config &
	tc_fail_if_bad $? "failed to start atalkd" || return
	sleep 5

	# get additions to /var/log/messages and look for atalkd (re)start message
	diff $TCTMP/messages /var/log/messages | grep "^>" > $TCTMP/new_messages
	grep -q "atalkd.*start" $TCTMP/new_messages 2>$stderr >$stdout
	tc_pass_or_fail $? "failed to find atalkd (re)start messages in syslog" \
			"syslog had:"$'\n'"$(cat /$TCTMP/new_messages)"

}


################################################################################
# main
################################################################################

TST_TOTAL=2

tc_setup		# exits on failure

test01 &&
test02
