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
# File :	serial-server.sh
#
# Description:	Read serial port and respond to a few commands.
#
# Author:	RC Paulsen and CSDL
#
# History:	Feb 18 2004 - Created. 
#		Feb 25 2004 (rcp)	remove dependency on lsof 
#		Feb 26 2004 (rcp)	cmd line args similar to serial-client.

################################################################################
# utility finctions
################################################################################

#
# Runs when testcase exits.
#
function cleanup()
{
	[ "$settings" ] && stty -F $tty $settings
	exec 4<&-
	exec 4>&-
}
trap "cleanup" 0

#
# Tell how to use this script.
#
function usage()
{
	local tab=$'\t'
	cat <<-EOF
		usage:
		$tab$0 [-b baudate] [-d device]
		$tab$0	-b : Set baud rate. Defaults to 115200.
		$tab$0	-d : Set serial port device. Defaults to /dev/ttyS1.
		Must have read/write access to the serial port.
	EOF
	exit 1
}

#
# Parse command line arguments.
#
function parse_args()
{
	tty="/dev/ttyS1"	# default
	Baud="115200"		# default
	while getopts b:d: opt ; do
		case "$opt" in
			b)	Baud=$OPTARG
				;;
			d)	tty=$OPTARG
				;;
			*)	usage	# exits
				;;
		esac
	done
}

#
# Initialize the serial port.
#
function setup()
{
	# get device
	[ -r "$tty" ] && [ -w "$tty" ] || usage

	# save original tty settings & set ours
	settings=$(stty -F $tty -g) || exit
	stty -F $tty $Baud sane -brkint -imaxbel crtscts -onlcr -icrnl -echo ||
		usage ||
		exit

	# ensure port is not in use
	# lsof | grep $tty &&
	for pid in $(ls /proc | egrep "[0-9]+") ; do
		ls -l /proc/$pid/fd 2>/dev/null | \
		grep -q "$tty" && found="$found $pid"
	done
	[ -z "$found" ] || {
		echo "refusing to run since $tty is already in use by pid(s) $found"
		exit 1
	}

	# open tty for read/write as fd 4
	exec 4<>$tty
}

################################################################################
# main
################################################################################

parse_args "$@"		# get command line arguments
setup			# exits on failure

#
# Process commands "forever".
#
#	NOTE: the ping and echo commands are in no way related to the
#	system commands ping and echo. They are just named that way to
#	indicate their basic behavior.
#
while : ; do
	echo -n "$tty $Baud waiting for command ... "
	set $(cat <$tty)				# read command line
	cmd=$1; shift; args="$@"
	1>&2 echo -n "PROCESSING \"$cmd $args\" ... "
	case $cmd in
		quit)				# close the server
			echo -n "" >$tty	# send eof
			echo Bye!
			exit 0			# exit from server
			;;
		ping)				# send canned response
			echo "ping response from s-server" >$tty
			echo -n "" >$tty	# send eof
			;;
		echo)				# send back the command line args
			echo "$args" >$tty
			echo -n "" >$tty	# send eof
			;;
		get)				# copy file from server to client
			[ "$args" ] && [ -r "$args" ] &&
			cat $args >$tty
			echo -n "" >$tty	# send eof
			;;
		put)				# copy file from client to server
			[ "$args" ] && put_file=$args || put_file=s-put.txt
			rm -f $put_file
			echo "GO" >$tty		# tell client we are ready to rcv
			echo -n "" >$tty	# send eof
			cat <$tty > $put_file	# receive the file
			;;
		*)				# ignore unrecognized commands
			echo -n "" >$tty	# send eof
			;;
	esac
	echo "done"
done
