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
# netns_comm.sh <NS_EXEC_PROGRAM> <IP_VERSION> <COMM_TYPE>
#
# OPTIONS:
#       * NS_EXEC_PROGRAM (ns_exec|ip)
#               Program which will be used to enter and run other commands
#               inside a network namespace.
#       * IP_VERSION (ipv4|ipv6)
#               Version of IP. (ipv4|ipv6)
#	* COMM_TYPE (netlink|ioctl)
#		Communication type between kernel and user space
#		for basic setup: enabling and assigning IP addresses
#		to the virtual ethernet devices. (Uses 'ip' command for netlink
#		and 'ifconfig' for ioctl.)
#
# Tests that a separate network namespace can configure and communicate
# over the devices it sees. Tests are done using netlink(7) ('ip' command)
# or ioctl(2) ('ifconfig' command) for controlling devices.
#
# There are three test cases:
# 1,2. communication over paired veth (virtual ethernet) devices
#      from two different network namespaces (each namespace has
#      one device)
#   3. communication over the lo (localhost) device in a separate
#      network namespace
#==============================================================================

TCID="netns_comm_$1_$2_$3"
TST_TOTAL=3
. netns_helper.sh

# SETUP
netns_setup $1 $2 $3 "192.168.0.2" "192.168.0.3" "fd00::2" "fd00::3"
tst_resm TINFO "NS interaction: $1 | devices setup: $3"


# TEST CASE #1
$NS_EXEC $NS_HANDLE0 $NS_TYPE $tping -q -c2 -I veth0 $IP1 1>/dev/null
if [ $? -eq 0 ]; then
	tst_resm TPASS "configuration and communication over veth0"
else
	tst_resm TFAIL "configuration and communication over veth0"
fi


# TEST CASE #2
$NS_EXEC $NS_HANDLE1 $NS_TYPE $tping -q -c2 -I veth1 $IP0 1>/dev/null
if [ $? -eq 0 ]; then
	tst_resm TPASS "configuration and communication over veth1"
else
	tst_resm TFAIL "configuration and communication over veth1"
fi


# TEST CASE #3
case "$2" in
ipv4) IP_LO="127.0.0.1" ;;
ipv6) IP_LO="::1" ;;
esac
case "$3" in
netlink)
	$NS_EXEC $NS_HANDLE0 $NS_TYPE ip link set dev lo up || \
		tst_brkm TBROK "enabling lo device failed"
	;;
ioctl)
	$NS_EXEC $NS_HANDLE0 $NS_TYPE ifconfig lo up || \
		tst_brkm TBROK "enabling lo device failed"
	;;
esac
$NS_EXEC $NS_HANDLE0 $NS_TYPE $tping -q -c2 -I lo $IP_LO 1>/dev/null
if [ $? -eq 0 ]; then
	tst_resm TPASS "configuration and communication over localhost"
else
	tst_resm TFAIL "configuration and communication over localhost"
fi


tst_exit
