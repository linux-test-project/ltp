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
# File :	serial-client.sh
#
# Description:	Test serial port
#
# Author:	RC Paulsen and CSDL
#
# History:	Feb 18 2004 - Created. 
#		Feb 25 2004 (rcp)	1. remove dependency on lsof 
#					2. probe for server w/o hanging

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

args="$@"

# Optional command line arguments
SPDev="/dev/ttyS0"
SPBaud="115200"
LOOPNU=5

SPData="s-datafile-1.txt"		# file to send to serial server
SPSet=""				# to save original tty settings
SPCfg="sane -brkint -imaxbel crtscts -onlcr -icrnl -echo" # our tty settings

################################################################################
# helper functions
################################################################################

#
# tc_local_cleanup	runs automatically when testcase exits
#
function tc_local_cleanup()
{
	[ "$quit_server" = "yes" ] && [ "$server_ok" = "yes" ] && send_command quit
	[ "$SPSet" ] && stty -F $SPDev $SPSet
	exec 4>&-
	exec 4<&-
}

#
# tc_usage		how to use this test
#
function tc_usage()
{
	local tab=$'\t'
	cat <<-EOF > $stdout
		usage:
		${tab}$0 [-b baudrate] [-c repcount] [-d device] [-s filesize] [--help]
		${tab}    -b : set baud rate, default is 115200
		${tab}    -c : how many time you want to repeat each command
		${tab}    -d : set device, default is /dev/ttyS0
		${tab}    -s : the file size to be translate(in KB), default is 1
		${tab}         It can be 1, 10 or 100.
		${tab}    -help : print this message
		exapmle:
		${tab}$0 -b 115200 -d /dev/ttyS1 -s 10 -c 20
		note:
		${tab}You must have read/write permissions to the tty device.
	EOF
	tc_break_if_bad 1
	exit
}

#
# read in command line arguments and convert them to local variables
#
function parse_args()
{
	while getopts c:b:d:s:hq opt ; do
		case "$opt" in
			c) LOOPNU=$OPTARG ;;
			b) SPBaud=$OPTARG ;;
			d) SPDev=$OPTARG ;;
			s)
				case "$OPTARG" in
					1|10|100)
						SPData="s-datafile-$OPTARG.txt" ;;
					*)
						tc_info \
						"Invalid file size $OPTARG"
						tc_usage  # exits
				esac
				;;
			q) quit_server="yes" ;;
			h) tc_usage ;;		#exits
			*)
				tc_info "Invalid option -$opt"
				tc_usage	# exits
				;;
		esac
	done
}

#
# probe for server
#
function probe_server()
{
	# ping server and read results
	echo ping > $SPDev
	echo -n  > $SPDev
	cat < $SPDev > $TCTMP/probe &	# run "cat" asynchronously in case it hangs
	killit=$!			# so we can kill cat if it hangs

	# give server 10 seconds to respond
	ctr=10
	while [ $ctr != 0 ] ; do
		tc_info "Waiting $ctr seconds for response on serial port $SPDev at $SPBaud baud"
		sleep 1
		[ -s $TCTMP/probe ] && return	# all is OK!
		let ctr-=1
	done

	# server did not respond: close tty, kill cat and exit
	tc_break_if_bad 1 "Server does not seem to be running" \
			"killing cat command $killit"
	exec 4>&-
	exec 4<&-
	kill $killit
	return 1
}

#
# tc_local_setup	runs automatically at testcase start (from tc_setup)
#
function tc_local_setup()
{
	parse_args $args	# exits on failure

	# used to get and compare responses from serial serve
	SPExp="$TCTMP/s-expected.txt"
	SPResp="$TCTMP/s-response.txt"

	# must have read/write priv to tty
	[ -r "$SPDev" ] && [ -w "$SPDev" ]
	tc_break_if_bad $? "$SPDev can not read or write" || return

	# ensure port is not in use
	# lsof | grep $SPDev >$stdout 2>$stderr 
	for pid in $(ls /proc | egrep "[0-9]+") ; do
		ls -l /proc/$pid/fd 2>/dev/null | \
		grep -q "$SPDev" && found="$found $pid"
	done
	[ -z "$found" ]
	tc_break_if_bad $? "Port $SPDev already in use by pid(s) $found" || return

	# save original tty settings
	stty -F $SPDev -g >$stdout 2>$stderr
	tc_break_if_bad $? "stty could not save settings for $SPDev" || return
	SPSet=$(cat $stdout)

	# configure tty for our use
	stty -F $SPDev $SPBaud $SPCfg >$stdout 2>$stderr
	tc_break_if_bad $? "stty could not configure $SPDev" || return

	# open tty for read/write as fd 4
	exec 4<>$SPDev
	tc_break_if_bad $? "Could not open $SPDev for read/write" || return

	# ensure server is running
	probe_server && server_ok="yes"
}

#
# send_command		zero is returned on success, non-zero on failure
#
function send_command()
{
	local command="$@"

	# send command to the device
	echo "$command" >$SPDev		# write the command
	echo -n "" >$SPDev		# send eof

	# get response
	cat <$SPDev > $SPResp		# read response

	# verify response
	diff $SPExp $SPResp >$stdout 2>$stderr
}

################################################################################
# testcase functions
################################################################################

function server_dummy()
{
	tc_register	"send dummy command to server $LOOPNU times"

	local i=0
	while [ $i -lt $LOOPNU ] ; do
		let i+=1
		cat /dev/null > $SPExp
		send_command "dummy command"
		tc_fail_if_bad $? "Failed on pass $i" || return
	done

	tc_pass_or_fail 0	# always pass if we get this far
}

function server_ping()
{
	tc_register	"get canned response from server $LOOPNU times"

	local i=0
	while [ $i -lt $LOOPNU ] ; do
		let i+=1
		echo "ping response from s-server" > $SPExp
		send_command "ping"
		tc_fail_if_bad $? "Failed on pass $i" || return
	done

	tc_pass_or_fail 0	# always pass if we get this far
}

function server_echo()
{
	tc_register	"get echo response from server $LOOPNU times"

	local i=0
	while [ $i -lt $LOOPNU ] ; do
		let i+=1
		local args="here are some args"
		echo "$args" > $SPExp
		send_command "echo $args"
		tc_fail_if_bad $? "Failed on pass $i" || return
	done

	tc_pass_or_fail 0	# always pass if we get this far
}

function server_get()
{
	tc_register	"get data file from server $LOOPNU times"

	local i=0
	while [ $i -lt $LOOPNU ] ; do
		let i+=1
		cp $SPData $SPExp
		send_command "get $SPData"
		tc_fail_if_bad $? "Failed on pass $i" || return
	done

	tc_pass_or_fail 0	# always pass if we get this far
}

function server_put()
{
	tc_register	"send and then get data file to/from server $LOOPNU times"

	local i=0
	while [ $i -lt $LOOPNU ] ; do
		let i+=1

		# Tell server that a file is coming and wait for "GO".
		# s-put is the name the server will use to save the file
		# so we can ask for it back again by name.
		echo "GO" > $SPExp
		send_command "put s-put" || return 

		# send the file after "GO" response is received
		cat $SPData >$SPDev
		echo -n "" >$SPDev

		# now get it back and compare it
		cp $SPData $SPExp
		send_command "get s-put"
		tc_fail_if_bad $? "Failed on pass $i" || return
	done

	tc_pass_or_fail 0	# always pass if we get this far
}

################################################################################
# main
################################################################################

let TST_TOTAL=5

# setup now
tc_setup

server_dummy &&
server_ping &&
server_echo &&
server_get &&
server_put
