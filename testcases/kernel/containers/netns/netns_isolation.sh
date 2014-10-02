#!/bin/sh
#==============================================================================
# Copyright (c) 2014 Red Hat, Inc.
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
#==============================================================================
# File: netns_isolation.sh
#
# Tests communication with ifconfig (uses ioctl), ip (uses netlink)
# and ping over a device which is not visible from the current network
# namespace (this communication should not be possible).
#

TCID=netns_isolation
TST_TOTAL=3
. test.sh
IP=192.168.0.2


cleanup()
{
	# removes veth0 device (which also removes paired veth1 device)
	ip netns exec myns0 ip link delete veth0
	# removes the network namespace myns
	ip netns del myns0
	ip netns del myns1
}


# SETUP
tst_require_root
tst_check_cmds ip
TST_CLEANUP=cleanup


# creates a new network namespace "myns0" (man 8 ip-netns)
ip netns add myns0 || \
	tst_brkm TBROK "unable to create a new network namespace (myns0)"

# creates a new network namespace "myns1"
ip netns add myns1 || \
	tst_brkm TBROK "unable to create a new network namespace (myns1)"

# creates a pair of virtual ethernet devices
ip netns exec myns0 ip link add veth0 type veth peer name veth1 || \
	tst_brkm TBROK "unable to create veth pair devices"

# adds device veth1 to myns1 namespace
ip netns exec myns0 ip link set veth1 netns myns1 || \
	tst_brkm TBROK "unable to add device veth1 to the network namespace myns1"


# TEST CASE #1
# setup an ip address on the veth1 device which is not visible
# from the "myns0" network namespace using ip (netlink)
ip netns exec myns0 ip address add $IP dev veth1 2>/dev/null
ret=$?
if [ $ret -ne 0 ]; then
	tst_resm TPASS "controlling a device from a separate NETNS over netlink not possible"
else
	tst_resm TFAIL "controlling a device from a separate NETNS over netlink possible"
fi


# TEST CASE #2
# ping over the veth1 device which is not visible from the "myns0"
# network namespace
ip netns exec myns0 ping -q -c 2 -I veth1 $IP 2>/dev/null
ret=$?
if [ $ret -ne 0 ]; then
	tst_resm TPASS "communication over a device from a separate NETNS not possible"
else
	tst_resm TFAIL "communication over a device from a separate NETNS possible"
fi


# TEST CASE #3
# setup an ip address on the veth1 device which is not visible
# from the "myns0" network namespace using ifconfig (ioctl)
tst_check_cmds ifconfig
ip netns exec myns0 ifconfig veth1 $IP 2>/dev/null
ret=$?
if [ $ret -ne 0 ]; then
	tst_resm TPASS "ioctl on a device from a separate NETNS not possible"
else
	tst_resm TFAIL "ioctl on a device from a separate NETNS possible"
fi


tst_exit
