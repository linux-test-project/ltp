#!/bin/bash
################################################################################
#                                                                              #
#  (C) Copyright IBM Corp. 2003							#
#                                                                              #
#  This program is free software;  you can redistribute it and/or modify       #
#  it under the terms of the GNU General Public License as published by        #
#  the Free Software Foundation; either version 2 of the License, or           #
#  (at your option) any later version.                                         #
#                                                                              #
#  This program is distributed in the hope that it will be useful, but         #
#  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY  #
#  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    #
#  for more details.                                                           #
#                                                                              #
#  You should have received a copy of the GNU General Public License           #
#  along with this program;  if not, write to the Free Software                #
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA     #
#                                                                              #
################################################################################
#
# File :	arpwatch_tests.sh
#
# Description: This program tests basic functionality of arpwatch command.
#
# Author:	Manoj Iyer  manjo@mail.utexas.edu
#
# History:	June 03 2003 - created - Manoj Iyer
#		June 17 2003 (RC Paulsen)
#			- drop alias when done
#			- kill arpwatch when done
#			- ensure running as root
#			- standardized use of utility functions
#		Oct 24 2003 - Manoj Iyer
#			added delay so that arpwatch can write to /var/log
#		Dec 03 2003 - Andrew Pham
#			default interface=eth0 if unable to determine
#		Jan 06 2003 - Robb Romans
#			Convert to new utility methods.
#		Jan 26 2004 (rcp) remove test for arp activity. Can't
#			predict if there will be any.
#		Feb 11 2004 (rcp) remove dependency on awk for interface
#			determination.
#		Feb 17 2004 (rcp) give it 5 seconds instead of 1.
#

###############################################################################
# source the utility functions
###############################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source
#		06 Jan 2004 - (robb) updated to tc_utils.source

interface=
aliasip="10.1.1.12"
notaliasip="10.2.2.12"

###############################################################################
# utility functions
###############################################################################

#
# tc_local_setup	Check dependencies; get alias interface.
#
function tc_local_setup()
{
	# check dependencies
	tc_root_or_break || return
	tc_exec_or_break arp cat head tail ifconfig grep syslogd ping || return

	# get the default interface
	line=$(route -n | grep "^0.0.0.0")
	[ "$line" ]
	tc_break_if_bad $? "Can't find route. Network not configured properly" || return
	set $line
	interface="$8"
	[ "$interface" ]
	tc_break_if_bad $? "Can't find network interface" || return

	# make alias
	ifconfig $interface:1 $aliasip up >$stdout 2>$stderr
	tc_fail_if_bad $? "Failed to alias interface on $interface" || exit
	tc_info "working with interface $interface:1 as $aliasip"
}

#
#	Restore networking, kill arpwatch.
#
function tc_local_cleanup()
{
	# drop alias
	ifconfig $interface:1 $aliasip down &>/dev/null

	# stop arpwatch
	killall arpwatch &>/dev/null
}

###############################################################################
#	test functions
###############################################################################

#
#	test01	ensure arpwatch installed
#
function test01()
{
	tc_register "is arpwatch installed?"
	tc_executes arpwatch
	tc_pass_or_fail $? "arpwatch NOT installed"
}

#
# test02	See that arpwatch starts w/o error
#
function test02()
{
	tc_register "run arpwatch"

	# capture syslog before starting arpwatch
	cp /var/log/messages $TCTMP/syslog_one
	
	# start arpwatch
	cat /dev/null > $TCTMP/arp.dat	# required for arpwatch
	arpwatch -i $interface:1 -f $TCTMP/arp.dat >$stdout 2>$stderr
	tc_fail_if_bad $? "failed to start arp watch on interface $interface:1" \
		|| return

	# be sure syslog has time to update
	sleep 5

	# capture syslog after starting arpwatch
	diff $TCTMP/syslog_one /var/log/messages > $TCTMP/started

	# ensure arpwatch started
	grep -iq "arpwatch: listening on $interface:1" $TCTMP/started >$stdout 2>$stderr
	tc_pass_or_fail $? "arpwatch failed to report \"listening\"" \
			"/var/log/messages has"$'\n'"$(tail /var/log/messages)" \
			|| return

	#
	# NOTE: By returning here we skip test for activity which is impossible
	# to predict. At least I can't seem to force it on any system.
	#
	# Note also that the above "tc_pass_or_fail" used to be
	# "tc_fail_if_bad".
	#
	return

	local system=`arp | head -2 | tail -1`
	system=${system%%.*}.$(hostname -d)
	[ "$system" ] 
	tc_break_if_bad $? "No other systems on lan to watch!" || return
	tc_info "waiting for activity from $system"

	#
	# TODO: None of this seems to ensure arp activity from $system
	#
	arp -d $system
	ping -c 1 $system -I $intrface &>/dev/null

	sleep 30

	# capture syslog after activity
	diff $TCTMP/syslog_one /var/log/messages > $TCTMP/activity

	grep -iq "arpwatch: bogon" $TCTMP/activity >$stdout 2>$stderr || \
	grep -iq "arpwatch: new station" $TCTMP/activity >>$stdout 2>>$stderr
	tc_pass_or_fail $? "arpwatch failed to report activity using syslog" \
		"/var/log/messages has"$'\n'"$(cat $TCTMP/activity)"
}

###############################################################################
#	main
###############################################################################

TST_TOTAL=2
tc_setup
test01 &&
test02
