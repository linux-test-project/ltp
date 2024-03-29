#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2022 Petr Vorel <pvorel@suse.cz>
# Copyright (c) Linux Test Project, 2014-2023
# Copyright (c) 2015 Red Hat, Inc.

TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="ip ping"
TST_NEEDS_DRIVERS="veth"

TST_OPTS="eI"
TST_PARSE_ARGS="netns_parse_args"
TST_USAGE="netns_usage"
TST_SETUP="${TST_SETUP:-netns_setup}"
TST_CLEANUP="${TST_CLEANUP:-netns_cleanup}"

TST_NET_SKIP_VARIABLE_INIT=1

# from tst_net_vars.c
IPV4_NET16_UNUSED="10.23"
IPV6_NET32_UNUSED="fd00:23"

# Set to "net" for tst_ns_create/tst_ns_exec as their options requires
# to specify a namespace type. Empty for ip command.
NS_TYPE=

# 'ping' or 'ping6'
tping=

# Network namespaces handles for manipulating and executing commands inside
# namespaces. For 'tst_ns_exec' handles are PIDs of daemonized processes running
# in namespaces.
NS_HANDLE0=
NS_HANDLE1=

# Adds "inet6 add" to the 'ifconfig' arguments which is required for the ipv6
# version. Always use with 'ifconfig', even if ipv4 version of a test case is
# used, in which case IFCONF_IN6_ARG will be empty string. Usage:
# ifconfig <device> $IFCONF_IN6_ARG IP/NETMASK
IFCONF_IN6_ARG=

# Program which will be used to enter and run other commands inside a network namespace.
# (tst_ns_exec|ip)
NS_EXEC="ip"

# Communication type between kernel and user space for basic setup: enabling and
# assigning IP addresses to the virtual ethernet devices. (Uses 'ip' command for
# netlink and 'ifconfig' for ioctl.)
# (netlink|ioctl)
COMM_TYPE="netlink"

do_cleanup=

netns_parse_args()
{
	case $1 in
	e) NS_EXEC="tst_ns_exec" ;;
	I) COMM_TYPE="ioctl"; tst_require_cmds ifconfig ;;
	esac
}

netns_usage()
{
	echo "usage: $0 [ -e ] [ -I ]"
	echo "OPTIONS"
	echo "-e      Use tst_ns_exec instead of ip"
	echo "-I      Test ioctl (with ifconfig) instead of netlink (with ip)"
}

netns_setup()
{
	if [ "$NS_EXEC" = "ip" ]; then
		netns_ip_setup
	else
		NS_TYPE="net"
		netns_ns_exec_setup
	fi

	IP0=$(tst_ipaddr_un -c 1)
	IP1=$(tst_ipaddr_un -c 2)

	if [ "$TST_IPV6" ]; then
		IFCONF_IN6_ARG="inet6 add"
		NETMASK=64
		tping="ping -6"
	else
		NETMASK=24
		tping=ping
	fi

	netns_set_ip

	tst_res TINFO "testing netns over $COMM_TYPE with $NS_EXEC $PROG"
	do_cleanup=1
}

netns_cleanup()
{
	[ "$do_cleanup" ] || return

	if [ "$NS_EXEC" = "ip" ]; then
		netns_ip_cleanup
	else
		netns_ns_exec_cleanup
	fi
}

# Sets up NS_EXEC to use 'tst_ns_exec', creates two network namespaces and stores
# their handles into NS_HANDLE0 and NS_HANDLE1 variables (in this case handles
# are PIDs of daemonized processes running in these namespaces). Virtual
# ethernet device is then created for each namespace.
netns_ns_exec_setup()
{
	local ret

	NS_EXEC="tst_ns_exec"

	NS_HANDLE0=$(tst_ns_create $NS_TYPE)
	if [ $? -eq 1 ]; then
		tst_res TINFO "$NS_HANDLE0"
		tst_brk TBROK "unable to create a new network namespace"
	fi

	NS_HANDLE1=$(tst_ns_create $NS_TYPE)
	if [ $? -eq 1 ]; then
		tst_res TINFO "$NS_HANDLE1"
		tst_brk TBROK "unable to create a new network namespace"
	fi

	$NS_EXEC $NS_HANDLE0 $NS_TYPE ip link add veth0 type veth peer name veth1 || \
		tst_brk TBROK "unable to create veth pair devices"

	$NS_EXEC $NS_HANDLE0 $NS_TYPE tst_ns_ifmove veth1 $NS_HANDLE1
	ret=$?
	[ $ret -eq 0 ] && return
	[ $ret -eq 32 ] && tst_brk TCONF "IFLA_NET_NS_PID not supported"

	tst_brk TBROK "unable to add device veth1 to the separate network namespace"
}

# Sets up NS_EXEC to use 'ip netns exec', creates two network namespaces
# and stores their handles into NS_HANDLE0 and NS_HANDLE1 variables. Virtual
# ethernet device is then created for each namespace.
netns_ip_setup()
{
	ip netns > /dev/null || \
		tst_brk TCONF "ip without netns support (required iproute2 >= ss111010 - v3.0.0)"

	NS_EXEC="ip netns exec"

	NS_HANDLE0=tst_net_ns0
	NS_HANDLE1=tst_net_ns1

	ip netns del $NS_HANDLE0 2>/dev/null
	ip netns del $NS_HANDLE1 2>/dev/null

	ROD ip netns add $NS_HANDLE0
	ROD ip netns add $NS_HANDLE1

	ROD $NS_EXEC $NS_HANDLE0 ip link add veth0 type veth peer name veth1
	ROD $NS_EXEC $NS_HANDLE0 ip link set veth1 netns $NS_HANDLE1
}

# Enables virtual ethernet devices and assigns IP addresses for both
# of them (IPv4/IPv6 variant is decided by netns_setup() function).
netns_set_ip()
{
	local cmd="ip"

	# This applies only for ipv6 variant:
	# Do not accept Router Advertisements (accept_ra) and do not use
	# Duplicate Address Detection (accept_dad) which uses Neighbor
	# Discovery Protocol - the problem is that until DAD can confirm that
	# there is no other host with the same address, the address is
	# considered to be "tentative" (attempts to bind() to the address fail
	# with EADDRNOTAVAIL) which may cause problems for tests using ipv6.
	if [ "$TST_IPV6" ]; then
		echo 0 | $NS_EXEC $NS_HANDLE0 $NS_TYPE \
			tee /proc/sys/net/ipv6/conf/veth0/accept_dad \
			/proc/sys/net/ipv6/conf/veth0/accept_ra >/dev/null
		echo 0 | $NS_EXEC $NS_HANDLE1 $NS_TYPE \
			tee /proc/sys/net/ipv6/conf/veth1/accept_dad \
			/proc/sys/net/ipv6/conf/veth1/accept_ra >/dev/null
	fi

	[ "$COMM_TYPE" = "ioctl" ] && cmd="ifconfig"

	if [ "$COMM_TYPE" = "netlink" ]; then
		ROD $NS_EXEC $NS_HANDLE0 $NS_TYPE ip address add $IP0/$NETMASK dev veth0
		ROD $NS_EXEC $NS_HANDLE1 $NS_TYPE ip address add $IP1/$NETMASK dev veth1
		ROD $NS_EXEC $NS_HANDLE0 $NS_TYPE ip link set veth0 up
		ROD $NS_EXEC $NS_HANDLE1 $NS_TYPE ip link set veth1 up
	else
		ROD $NS_EXEC $NS_HANDLE0 $NS_TYPE ifconfig veth0 $IFCONF_IN6_ARG $IP0/$NETMASK
		ROD $NS_EXEC $NS_HANDLE1 $NS_TYPE ifconfig veth1 $IFCONF_IN6_ARG $IP1/$NETMASK
		ROD $NS_EXEC $NS_HANDLE0 $NS_TYPE ifconfig veth0 up
		ROD $NS_EXEC $NS_HANDLE1 $NS_TYPE ifconfig veth1 up
	fi
}

netns_ns_exec_cleanup()
{
	[ "$NS_EXEC" ] || return

	# removes veth0 device (which also removes the paired veth1 device)
	$NS_EXEC $NS_HANDLE0 $NS_TYPE ip link delete veth0

	kill -9 $NS_HANDLE0 2>/dev/null
	kill -9 $NS_HANDLE1 2>/dev/null
}


netns_ip_cleanup()
{
	[ "$NS_EXEC" ] || return

	# removes veth0 device (which also removes the paired veth1 device)
	$NS_EXEC $NS_HANDLE0 ip link delete veth0

	ip netns del $NS_HANDLE0 2>/dev/null
	ip netns del $NS_HANDLE1 2>/dev/null
}

. tst_net.sh
