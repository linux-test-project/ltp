#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Köry Maincent <kory.maincent@bootlin.com> 2020
# Copyright (c) 2015 Red Hat, Inc.
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
	EXPECT_FAIL $NS_EXEC $NS_HANDLE0 $NS_TYPE ip address add $IP1/$NETMASK dev veth1

	tst_require_cmds ifconfig
	EXPECT_FAIL $NS_EXEC $NS_HANDLE0 $NS_TYPE ifconfig veth1 $IFCONF_IN6_ARG $IP1/$NETMASK
}

. netns_helper.sh

PROG=$1
IP_VER=$2
COM_TYPE=$3

tst_run
