#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003	                                              ##
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
# File :	snort.sh
#
# Description:	Test snort package
#
# Author:	Robb Romans <robb@austin.ibm.com>
#
# History:	Sep 01 2003 - created - RR
#               Sep 08 2003 - modified per peer review
#                           - removed awk requirement
#		Dec 02 2003 (rcp) BUG 5438: add sleep 3 to expect script.
#		08 Jan 2004 - (RR) updated to tc_utils.source,
#				Add logic to find default interface (bug 5282),
# 				Add logic to test connectivity to nmap server.
#
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# Variable definitions
################################################################################

REQUIRED="awk cat chmod echo expect grep host hostname mv ping route sed which"

NMAP_SERVER="godzilla" # (has to be on local subnet)
# Username and passwd on $NMAP_SERVER:
REM_USER="fivtest"
REM_PASS="fivtest"

INTERFACE="" # set in tc_local_setup
CONFIG="/etc/sysconfig/snort"
STARTUP="/etc/init.d/snort"

################################################################################
# testcase functions
################################################################################

function tc_local_setup {

	# Make sure $NMAP_SERVER is up
	ping -c 1 -w 10 $NMAP_SERVER &>/dev/null
	tc_break_if_bad "$?" "FATAL: Unable to find $NMAP_SERVER." || exit

	# find default network interface
	local TMP=""
	TMP=$(route | grep default)
	set $TMP
	INTERFACE=$8
	[ "$INTERFACE" ] || INTERFACE=eth0

	# Modify /etc/sysconfig/snort
	[ -f $CONFIG ]
	tc_break_if_bad $? "FATAL: No snort config file in $CONFIG!" || exit
	local ORIG_IFACE=""
	if ! grep "SNORT_INTERFACE\=$INTERFACE" $CONFIG &>/dev/null ; then
		tc_info "Modifying $CONFIG to use $INTERFACE"
		ORIG_IFACE=$(grep SNORT_INTERFACE $CONFIG | awk -F= '{print $2}')
		mv $CONFIG $CONFIG.orig
		sed -e "s,SNORT_INTERFACE=$ORIG_IFACE,SNORT_INTERFACE=$INTERFACE," \
			$CONFIG.orig > $CONFIG
		tc_break_if_bad $? "Unable to set network interface in $CONFIG" || exit
	fi
}

function tc_local_cleanup {

	tc_info "Stopping snort server."
	$STARTUP stop &>/dev/null

	# Restore /etc/sysconfig/snort
	[ -f $CONFIG.orig ] && mv $CONFIG.orig $CONFIG
}

function test_inst {

	tc_register "Is snort installed?"
	tc_executes "snort -V"
	tc_pass_or_fail $? "snort is not properly installed."
}

function start_server {
       
	tc_register "Starting the snort server."
       
        # Clean out existing log 
	[ -f /var/log/snort/alert ] && rm /var/log/snort/alert &>/dev/null

        # Test for initialization script
	[ -f $STARTUP ]
	tc_fail_if_bad $? "No /etc/init.d/snort file! Unable to start snort." || return

	$STARTUP restart 1>$stdout 2>$stderr
	tc_pass_or_fail $? "Initialization file failed to start snort."
}

function scan_test {

	tc_register "Initiate a scan - check response"
	local MY_IP=""
	local TMP_IP=""

        # Get our ip address
	MY_IP="$(hostname -i)"
	[ -n "$MY_IP" ]
	tc_fail_if_bad $? "Unable to determine our IP address." || return

        # Create an expect script to run remote command over ssh
        # Handles host key checking
	local expcmd=`which expect`
	cat > $TCTMP/exp$TCID <<-EOF
		#!$expcmd -f
		set timeout 10
		proc abort {} { echo "Timeout on expect script in scan_test()" ; exit 1 }
		spawn ssh -l $REM_USER $NMAP_SERVER nmap $MY_IP &>/dev/null
		expect {
			"(yes/no)?" { send "yes\r" }
		}
		expect {
			timeout abort
			"assword:" { sleep 3;  send "$REM_PASS\r" }
		}
		expect eof
	EOF
	chmod +x $TCTMP/exp$TCID 2>$stderr

	# Start scan.
	tc_info "Starting nmap scan on $NMAP_SERVER"
	$TCTMP/exp$TCID >$stdout 2>$stderr
	tc_fail_if_bad $? "Expect script failed: Could not start remote nmap scan." || return

        # Snort will have created an alert log for the incoming scan
        # if it is functioning correctly.
	tc_info "Sleeping 6 seconds to let snort catch up."
	sleep 6

	tc_info "Checking local log for scan response to $MY_IP"
	grep -q $MY_IP /var/log/snort/alert >$stdout 2>$stderr
	tc_pass_or_fail $? "Scan initiated but snort did not respond." \
			"expected to see $MY_IP in /var/log/snort/alert" \
			"/var/log/snort/alert:"$'\n'"$(cat /var/log/snort/alert)"
}

################################################################################
# MAIN
################################################################################

# Function:	main
#
# Exit:		- zero on success
#		- non-zero on failure
#
TST_TOTAL=3
tc_setup
tc_exec_or_break $REQUIRED || exit
tc_root_or_break || exit
test_inst &&
start_server &&
scan_test


