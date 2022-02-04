#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2022 Petr Vorel <pvorel@suse.cz>
# Copyright (c) Köry Maincent <kory.maincent@bootlin.com> 2020
# Copyright (c) 2015 Red Hat, Inc.
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

TST_TESTFUNC="do_test"

do_test()
{
	local ip_lo="127.0.0.1"
	[ "$TST_IPV6" ] && ip_lo="::1"

	EXPECT_PASS $NS_EXEC $NS_HANDLE0 $NS_TYPE $tping -q -c2 -I veth0 $IP1 1>/dev/null
	EXPECT_PASS $NS_EXEC $NS_HANDLE1 $NS_TYPE $tping -q -c2 -I veth1 $IP0 1>/dev/null

	if [ "$COMM_TYPE" = "netlink" ]; then
		ROD $NS_EXEC $NS_HANDLE0 $NS_TYPE ip link set dev lo up
	else
		ROD $NS_EXEC $NS_HANDLE0 $NS_TYPE ifconfig lo up
	fi

	EXPECT_PASS $NS_EXEC $NS_HANDLE0 $NS_TYPE $tping -q -c2 -I lo $ip_lo 1>/dev/null
}

. netns_helper.sh
tst_run
