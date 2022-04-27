#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2006 International Business Machines  Corp.
# Copyright (c) 2020 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Mitsuru Chinen <mitch@jp.ibm.com>
#
# Verify that the kernel is not crashed when joining a multicast group with
# a single socket, then receiving a large number of UDP packets at the socket

TST_NEEDS_ROOT=1
TST_NEEDS_TMPDIR=1
TST_SETUP="mcast_setup_normal_udp"
TST_CLEANUP="mcast_cleanup"
TST_TESTFUNC="do_test"

do_test()
{
	tst_res TINFO "joining an IPv${TST_IPVER} multicast group with a single socket, then receiving a large number of UDP packets at the socket in $NS_DURATION seconds"

	local addr="$MCAST_IPV4_ADDR"
	[ "$TST_IPV6" ] && addr="$MCAST_IPV6_ADDR"

	local port=$(tst_get_unused_port ipv${TST_IPVER} dgram)
	[ $? -ne 0 ] && tst_brk TBROK "no free udp port available"

	# Run a receiver
	ROD $MCAST_LCMD -f $TST_IPVER -I $(tst_iface lhost) -m $addr -p $port -b

	# Run a sender
	tst_rhost_run -s -c "$MCAST_RCMD -D $addr -f $TST_IPVER -p $port -s 32767 -m -I $(tst_iface rhost) -t $NS_DURATION"

	tst_res TPASS "test finished successfully"
}

. mcast-lib.sh
tst_run
