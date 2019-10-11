#!/bin/sh
#==============================================================================
# Copyright (c) 2015 Red Hat, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of version 2 the GNU General Public License as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Written by Matus Marhefka <mmarhefk@redhat.com>
#
#==============================================================================
#
# SYNOPSIS:
# netns_breakns.sh <NS_EXEC_PROGRAM> <IP_VERSION> <COMM_TYPE>
#
# OPTIONS:
#	* NS_EXEC_PROGRAM (ns_exec|ip)
#		Program which will be used to enter and run other commands
#		inside a network namespace.
#	* IP_VERSION (ipv4|ipv6)
#		Version of IP. (ipv4|ipv6)
#	* COMM_TYPE (netlink|ioctl)
#		Communication type between kernel and user space
#		for basic setup: enabling and assigning IP addresses
#		to the virtual ethernet devices. (Uses 'ip' command for netlink
#		and 'ifconfig' for ioctl.)
#
# Tests communication with ip (uses netlink) and ifconfig (uses ioctl)
# over a device which is not visible from the current network namespace.
#
# There are two test cases which are trying to set an ip address on the veth1
# device which is not inside the network namespace referred to by NS_HANDLE0:
# 1. using netlink (ip command).
# 2. using ioctl (ifconfig command).
#==============================================================================

TCID="netns_breakns_$1_$2_$3"
TST_TOTAL=2
. netns_helper.sh

# SETUP
netns_setup $1 $2 $3 "192.168.0.2" "192.168.0.3" "fd00::2" "fd00::3"
tst_resm TINFO "NS interaction: $1 | devices setup: $3"


# TEST CASE #1
$NS_EXEC $NS_HANDLE0 $NS_TYPE ip address add $IP1/$NETMASK dev veth1 2>/dev/null
if [ $? -ne 0 ]; then
	tst_resm TPASS "controlling device over netlink"
else
	tst_resm TFAIL "controlling device over netlink"
fi


# TEST CASE #2
tst_require_cmds ifconfig
$NS_EXEC $NS_HANDLE0 $NS_TYPE ifconfig veth1 $IFCONF_IN6_ARG $IP1/$NETMASK 2>/dev/null
if [ $? -ne 0 ]; then
	tst_resm TPASS "controlling device over ioctl"
else
	tst_resm TFAIL "controlling device over ioctl"
fi


tst_exit
