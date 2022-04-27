#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Köry Maincent <kory.maincent@bootlin.com> 2020
# Copyright (c) 2015 Red Hat, Inc.
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

TST_POS_ARGS=3
TST_SETUP=do_setup
TST_TESTFUNC=do_test

do_setup()
{
	netns_setup $PROG $IP_VER $COM_TYPE "192.168.0.2" "192.168.0.3" "fd00::2" "fd00::3"
	tst_res TINFO "NS interaction: $PROG | devices setup: $COM_TYPE"
}

do_test()
{
	EXPECT_PASS $NS_EXEC $NS_HANDLE0 $NS_TYPE $tping -q -c2 -I veth0 $IP1 1>/dev/null

	EXPECT_PASS $NS_EXEC $NS_HANDLE1 $NS_TYPE $tping -q -c2 -I veth1 $IP0 1>/dev/null

	case "$IP_VER" in
	ipv4) ip_lo="127.0.0.1" ;;
	ipv6) ip_lo="::1" ;;
	esac
	case "$COM_TYPE" in
	netlink)
		$NS_EXEC $NS_HANDLE0 $NS_TYPE ip link set dev lo up || \
			tst_brk TBROK "enabling lo device failed"
		;;
	ioctl)
		$NS_EXEC $NS_HANDLE0 $NS_TYPE ifconfig lo up || \
			tst_brk TBROK "enabling lo device failed"
		;;
	esac
	EXPECT_PASS $NS_EXEC $NS_HANDLE0 $NS_TYPE $tping -q -c2 -I lo $ip_lo 1>/dev/null
}

. netns_helper.sh

PROG=$1
IP_VER=$2
COM_TYPE=$3

tst_run
