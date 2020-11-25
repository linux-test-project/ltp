#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2006 International Business Machines  Corp.
# Copyright (c) 2020 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Mitsuru Chinen <mitch@jp.ibm.com>
#
# Verify that the kernel is not crashed when joining a multicast group
# on a single socket, then receiving a large number of Multicast Address
# Specific Query

TST_NEEDS_ROOT=1
TST_NEEDS_TMPDIR=1
. mcast-lib.sh

do_setup()
{
	mcast_setup $MCASTNUM_NORMAL
}

do_test()
{
	tst_res TINFO "joining an IPv${TST_IPVER} multicast group on a single socket, then receiving a large number of Multicast Address Specific Query in $NS_DURATION seconds"

	local prefix="$MCAST_IPV4_ADDR_PREFIX"
	[ "$TST_IPV6" ] && prefix="$MCAST_IPV6_ADDR_PREFIX"

	# Run a multicast join tool
	local tmpfile=$$
	EXPECT_PASS $MCAST_LCMD -n 1 -p $prefix \> $tmpfile
	tst_res TINFO "joined $(grep groups $tmpfile)"

	# Send IGMP Multicast Address Specific Query from the remote host
	local params="-m $MCAST_IPV4_ADDR"
	[ "$TST_IPV6" ] && params="-S $(tst_ipaddr) -m -D $MCAST_IPV6_ADDR"
	EXPECT_RHOST_PASS $MCAST_RCMD -t $NS_DURATION -r 0 $params
}

tst_run
