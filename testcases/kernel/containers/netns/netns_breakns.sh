#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2022 Petr Vorel <pvorel@suse.cz>
# Copyright (c) Köry Maincent <kory.maincent@bootlin.com> 2020
# Copyright (c) 2015 Red Hat, Inc.
#
# Tests communication with ip (uses netlink) and ifconfig (uses ioctl)
# over a device which is not visible from the current network namespace.
#
# There are two test cases which are trying to set an ip address on the veth1
# device which is not inside the network namespace referred to by NS_HANDLE0:
# 1. using netlink (ip command).
# 2. using ioctl (ifconfig command).

TST_TESTFUNC="do_test"

do_test()
{
	EXPECT_FAIL $NS_EXEC $NS_HANDLE0 $NS_TYPE ip address add $IP1/$NETMASK dev veth1

	tst_require_cmds ifconfig
	EXPECT_FAIL $NS_EXEC $NS_HANDLE0 $NS_TYPE ifconfig veth1 $IFCONF_IN6_ARG $IP1/$NETMASK
}

. netns_helper.sh
tst_run
