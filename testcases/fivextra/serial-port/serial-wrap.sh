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
# File :	serial-wrap.sh
#
# Description:	Test serial port by running serial-server.sh and
#		serial-client.sh. This assumes a wrapped serial cable.
#
# Author:	RC Paulsen
#
# History:	Mar 04 2004 - Created. 
#

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

# Optional command line arguments
client_tty="/dev/ttyS0"
server_tty="/dev/ttyS1"
baud="115200"
count=5			# how often to run each test in serial-client.sh
size=1			# size of file to xfer (k-bytes) in serial-client.sh

################################################################################
# helper functions
################################################################################

#
# usage		how to use this test
#
function usage()
{
	local tab=$'\t'
	cat <<-EOF
		usage:
		${tab}$0 [-b baudrate] [-c repcount] [-s filesize] [-x tty] [-y tty] [-help]
		${tab}    -b : baud rate, default is 115200
		${tab}    -c :  how many time you want to repeat for every test
		${tab}    -s : the file size to be translate(in KB), default is 1
		${tab}         It can be 1, 10 or 100.
		${tab}    -x : client-side tty device, default is /dev/ttyS0
		${tab}    -y : server-side tty device, default is /dev/ttyS1
		${tab}    -help : print this message
		This test requires a wrapped serial cable. It starts serial-server.sh
		and then the actual testcase, serial-client.sh.
		exapmle:
		${tab}$0 -b 115200 -s 10 -c 20 -x /dev/ttyS1 -y /dsev/ttyS0
		note:
		${tab}You must have read/write permissions to the tty device.
	EOF
	exit 1
}

#
# parse_args	Parse the command line arguments
#
function parse_args()
{
	while getopts b:s:c:x:y:h opt ; do
		case "$opt" in
			x) client_tty="$OPTARG" ;;
			y) server_tty="$OPTARG" ;;
			b) baud="$OPTARG" ;;
			s) size="$OPTARG" ;;
			c) count="$OPTARG" ;;
			*) usage ;; # exits
		esac
	done
}

################################################################################
# main
################################################################################

# get command line arguments
parse_args "$@"

# start the server in background
./serial-server.sh -b $baud -d $server_tty &>/dev/null &
sleep 2		# give it time to start

# now run the test
./serial-client.sh -q -b $baud -d $client_tty -s $size -c $count
