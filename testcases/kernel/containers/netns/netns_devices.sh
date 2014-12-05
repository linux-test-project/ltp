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
# File: netns_devices.sh
#
# Tests that a separate network namespace can configure and communicate
# over the devices it sees. Tests are done using ip command which uses
# netlink(7) sockets to control devices. There are three test cases:
# 1,2. communication over paired veth (virtual ethernet) devices
#      from two different network namespaces (each namespace has
#      one device)
#   3. communication over the lo (localhost) device in a separate
#      network namespace
#

TCID=netns_devices
TST_TOTAL=3
. test.sh
. netns_helper.sh
IP0=192.168.0.1
IP1=192.168.0.2


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
tst_check_iproute 111010
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


# setup for veth0 device (which is inside myns0 namespace)
ip netns exec myns0 ip address add $IP0/24 dev veth0 || \
	tst_brkm TBROK "adding address to veth0 failed"
ip netns exec myns0 ip link set dev veth0 up || \
	tst_brkm TBROK "enabling veth0 device failed"

# setup for veth1 (which is inside myns1 namespace)
ip netns exec myns1 ip address add $IP1/24 dev veth1 || \
	tst_brkm TBROK "adding address to veth1 failed"
ip netns exec myns1 ip link set dev veth1 up || \
	tst_brkm TBROK "enabling veth1 device failed"


# TEST CASE #1
ip netns exec myns0 ping -q -c 2 -I veth0 $IP1 >/dev/null
ret=$?
if [ $ret -eq 0 ]; then
	tst_resm TPASS "netlink configuration and communication over veth0 device"
else
	tst_resm TFAIL "netlink configuration and communication over veth0 device"
fi


# TEST CASE #2
ip netns exec myns1 ping -q -c 2 -I veth1 $IP0 >/dev/null
ret=$?
if [ $ret -eq 0 ]; then
	tst_resm TPASS "netlink configuration and communication over veth1 device"
else
	tst_resm TFAIL "netlink configuration and communication over veth1 device"
fi


# TEST CASE #3
# enables lo device
ip netns exec myns0 ip link set dev lo up || \
	tst_brkm TBROK "enabling lo device failed"

ip netns exec myns0 ping -q -c 2 -I lo 127.0.0.1 >/dev/null
ret=$?
if [ $ret -eq 0 ]; then
	tst_resm TPASS "netlink configuration and communication over lo device"
else
	tst_resm TFAIL "netlink configuration and communication over lo device"
fi


tst_exit
