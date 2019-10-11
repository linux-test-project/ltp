#!/bin/sh
#==============================================================================
# Copyright (c) Linux Test Project, 2014
# Copyright (c) 2015 Red Hat, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#==============================================================================

. test.sh
TST_CLEANUP=netns_ns_exec_cleanup

# Set to 1 only for test cases using ifconfig (ioctl).
USE_IFCONFIG=0


##
# Variables which can be used in test cases (set by netns_setup() function):
###############################################################################
# Use in test cases to execute commands inside a namespace. Set to 'ns_exec' or
# 'ip netns exec' command according to NS_EXEC_PROGRAM argument specified in
# netns_setup() function call.
NS_EXEC=""

# Set to "net" for ns_create/ns_exec as their options requires
# to specify a namespace type. Empty for ip command.
NS_TYPE=""

# IP addresses of veth0 (IP0) and veth1 (IP1) devices (ipv4/ipv6 variant
# is determined according to the IP_VERSION argument specified in netns_setup()
# function call.
IP0=""
IP1=""
NETMASK=""

# 'ping' or 'ping6' according to the IP_VERSION argument specified
# in netns_setup() function call.
tping=""

# Network namespaces handles for manipulating and executing commands inside
# namespaces. For 'ns_exec' handles are PIDs of daemonized processes running
# in namespaces.
NS_HANDLE0=""
NS_HANDLE1=""

# Adds "inet6 add" to the 'ifconfig' arguments which is required for the ipv6
# version. Always use with 'ifconfig', even if ipv4 version of a test case is
# used, in which case IFCONF_IN6_ARG will be empty string. Usage:
# ifconfig <device> $IFCONF_IN6_ARG IP/NETMASK
IFCONF_IN6_ARG=""
###############################################################################


tst_check_iproute()
{
	local cur_ipver="$(ip -V)"
	local spe_ipver="$1"

	cur_ipver=${cur_ipver##*s}

	if [ -z $cur_ipver ] || [ -z $spe_ipver ]; then
		tst_brkm TBROK "don't obtain valid iproute version"
	fi

	if [ $cur_ipver -lt $spe_ipver ]; then
		tst_brkm TCONF \
			"The commands in iproute tools do not support required objects"
	fi
}

##
# Sets up global variables which can be used in test cases (documented above),
# creates two network namespaces and a pair of virtual ethernet devices, each
# device in one namespace. Each device is then enabled and assigned an IP
# address according to the function parameters. IFCONF_IN6_ARG variable is set
# only if ipv6 variant of test case is used (determined by IP_VERSION argument).
#
# SYNOPSIS:
# netns_setup <NS_EXEC_PROGRAM> <IP_VERSION> <COMM_TYPE> <IP4_VETH0>
#             <IP4_VETH1> <IP6_VETH0> <IP6_VETH1>
#
# OPTIONS:
#	* NS_EXEC_PROGRAM (ns_exec|ip)
#		Program which will be used to enter and run other commands
#		inside a network namespace.
#	* IP_VERSION (ipv4|ipv6)
#		Version of IP. (ipv4|ipv6)
#	* COMM_TYPE (netlink|ioctl)
#		Communication type between kernel and user space
#		for enabling and assigning IP addresses to the virtual
#		ethernet devices. Uses 'ip' command for netlink and 'ifconfig'
#		for ioctl. (If set to ioctl, function also checks the existance
#		of the 'ifconfig' command.)
#	* IP4_VETH0, IP4_VETH1
#		IPv4 addresses for veth0 and veth1 devices.
#	* IP6_VETH0, IP6_VETH1
#		IPv6 addresses for veth0 and veth1 devices.
#
# On success function returns, on error tst_brkm is called and TC is terminated.
netns_setup()
{
	tst_require_root
	tst_require_cmds ip modprobe

	modprobe veth > /dev/null 2>&1

	case "$1" in
	ns_exec)
		setns_check
		if [ $? -eq 32 ]; then
			tst_brkm TCONF "setns not supported"
		fi
		NS_TYPE="net"
		netns_ns_exec_setup
		TST_CLEANUP=netns_ns_exec_cleanup
		;;
	ip)
		netns_ip_setup
		TST_CLEANUP=netns_ip_cleanup
		;;
	*)
		tst_brkm TBROK \
		"first argument must be a program used to enter a network namespace (ns_exec|ip)"
		;;
	esac

	case "$3" in
	netlink)
		;;
	ioctl)
		USE_IFCONFIG=1
		tst_require_cmds ifconfig
		;;
	*)
		tst_brkm TBROK \
		"third argument must be a comm. type between kernel and user space (netlink|ioctl)"
		;;
	esac

	if [ -z "$4" ]; then
		tst_brkm TBROK "fourth argument must be the IPv4 address for veth0"
	fi
	if [ -z "$5" ]; then
		tst_brkm TBROK "fifth argument must be the IPv4 address for veth1"
	fi
	if [ -z "$6" ]; then
		tst_brkm TBROK "sixth argument must be the IPv6 address for veth0"
	fi
	if [ -z "$7" ]; then
		tst_brkm TBROK "seventh argument must be the IPv6 address for veth1"
	fi

	case "$2" in
	ipv4)
		IP0=$4; IP1=$5
		tping="ping"; NETMASK=24
		;;
	ipv6)
		IFCONF_IN6_ARG="inet6 add"
		IP0=$6; IP1=$7;
		if which ping6 >/dev/null 2>&1; then
		    tping="ping6"
		else
		    tping="ping -6"
		fi
		NETMASK=64
		;;
	*)
		tst_brkm TBROK "second argument must be an ip version (ipv4|ipv6)"
		;;
	esac

	netns_set_ip
}

##
# Sets up NS_EXEC to use 'ns_exec', creates two network namespaces and stores
# their handles into NS_HANDLE0 and NS_HANDLE1 variables (in this case handles
# are PIDs of daemonized processes running in these namespaces). Virtual
# ethernet device is then created for each namespace.
netns_ns_exec_setup()
{
	NS_EXEC="ns_exec"

	NS_HANDLE0=$(ns_create $NS_TYPE)
	if [ $? -eq 1 ]; then
		tst_resm TINFO "$NS_HANDLE0"
		tst_brkm TBROK "unable to create a new network namespace"
	fi

	NS_HANDLE1=$(ns_create $NS_TYPE)
	if [ $? -eq 1 ]; then
		tst_resm TINFO "$NS_HANDLE1"
		tst_brkm TBROK "unable to create a new network namespace"
	fi

	$NS_EXEC $NS_HANDLE0 $NS_TYPE ip link add veth0 type veth peer name veth1 || \
		tst_brkm TBROK "unable to create veth pair devices"

	$NS_EXEC $NS_HANDLE0 $NS_TYPE ns_ifmove veth1 $NS_HANDLE1
	ret=$?
	if [ $ret -eq 0 ]; then
		return;
	fi

	if [ $ret -eq 32 ]; then
		tst_brkm TCONF "IFLA_NET_NS_PID not supported"
	fi

	tst_brkm TBROK "unable to add device veth1 to the separate network namespace"
}

##
# Sets up NS_EXEC to use 'ip netns exec', creates two network namespaces
# and stores their handles into NS_HANDLE0 and NS_HANDLE1 variables. Virtual
# ethernet device is then created for each namespace.
netns_ip_setup()
{
	tst_check_iproute 111010
	NS_EXEC="ip netns exec"

	NS_HANDLE0=tst_net_ns0
	NS_HANDLE1=tst_net_ns1

	ip netns del $NS_HANDLE0 2>/dev/null
	ip netns del $NS_HANDLE1 2>/dev/null

	ip netns add $NS_HANDLE0 || \
		tst_brkm TBROK "unable to create a new network namespace"
	ip netns add $NS_HANDLE1 || \
		tst_brkm TBROK "unable to create a new network namespace"

	$NS_EXEC $NS_HANDLE0 ip link add veth0 type veth peer name veth1 || \
		tst_brkm TBROK "unable to create veth pair devices"

	$NS_EXEC $NS_HANDLE0 ip link set veth1 netns $NS_HANDLE1 || \
		tst_brkm TBROK "unable to add device veth1 to the separate network namespace"
}

##
# Enables virtual ethernet devices and assigns IP addresses for both
# of them (IPv4/IPv6 variant is decided by netns_setup() function).
netns_set_ip()
{
	if [ -z "$NS_EXEC" ]; then
		tst_brkm TBROK "netns_setup() function must be called first"
	fi

	# This applies only for ipv6 variant:
	# Do not accept Router Advertisements (accept_ra) and do not use
	# Duplicate Address Detection (accept_dad) which uses Neighbor
	# Discovery Protocol - the problem is that until DAD can confirm that
	# there is no other host with the same address, the address is
	# considered to be "tentative" (attempts to bind() to the address fail
	# with EADDRNOTAVAIL) which may cause problems for tests using ipv6.
	echo 0 | $NS_EXEC $NS_HANDLE0 $NS_TYPE \
		tee /proc/sys/net/ipv6/conf/veth0/accept_dad \
		/proc/sys/net/ipv6/conf/veth0/accept_ra >/dev/null
	echo 0 | $NS_EXEC $NS_HANDLE1 $NS_TYPE \
		tee /proc/sys/net/ipv6/conf/veth1/accept_dad \
		/proc/sys/net/ipv6/conf/veth1/accept_ra >/dev/null

	case $USE_IFCONFIG in
	1)
		$NS_EXEC $NS_HANDLE0 $NS_TYPE ifconfig veth0 $IFCONF_IN6_ARG $IP0/$NETMASK ||
			tst_brkm TBROK "adding address to veth0 failed"
		$NS_EXEC $NS_HANDLE1 $NS_TYPE ifconfig veth1 $IFCONF_IN6_ARG $IP1/$NETMASK ||
			tst_brkm TBROK "adding address to veth1 failed"
		$NS_EXEC $NS_HANDLE0 $NS_TYPE ifconfig veth0 up ||
			tst_brkm TBROK "enabling veth0 device failed"
		$NS_EXEC $NS_HANDLE1 $NS_TYPE ifconfig veth1 up ||
			tst_brkm TBROK "enabling veth1 device failed"
		;;
	*)
		$NS_EXEC $NS_HANDLE0 $NS_TYPE ip address add $IP0/$NETMASK dev veth0 ||
			tst_brkm TBROK "adding address to veth0 failed"
		$NS_EXEC $NS_HANDLE1 $NS_TYPE ip address add $IP1/$NETMASK dev veth1 ||
			tst_brkm TBROK "adding address to veth1 failed"
		$NS_EXEC $NS_HANDLE0 $NS_TYPE ip link set veth0 up ||
			tst_brkm TBROK "enabling veth0 device failed"
		$NS_EXEC $NS_HANDLE1 $NS_TYPE ip link set veth1 up ||
			tst_brkm TBROK "enabling veth1 device failed"
		;;
	esac
}

netns_ns_exec_cleanup()
{
	if [ -z "$NS_EXEC" ]; then
		return
	fi

	# removes veth0 device (which also removes the paired veth1 device)
	$NS_EXEC $NS_HANDLE0 $NS_TYPE ip link delete veth0

	kill -9 $NS_HANDLE0 2>/dev/null
	kill -9 $NS_HANDLE1 2>/dev/null
}


netns_ip_cleanup()
{
	if [ -z "$NS_EXEC" ]; then
		return
	fi

	# removes veth0 device (which also removes the paired veth1 device)
	$NS_EXEC $NS_HANDLE0 ip link delete veth0

	ip netns del $NS_HANDLE0 2>/dev/null
	ip netns del $NS_HANDLE1 2>/dev/null
}
