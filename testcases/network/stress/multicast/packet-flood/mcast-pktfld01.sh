#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2006 International Business Machines  Corp.
# Copyright (c) 2020 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Mitsuru Chinen <mitch@jp.ibm.com>
#
# Verify that the kernel is not crashed when joining a multicast group with
# a single socket, then receiving a large number of UDP packets at the socket

TST_NEEDS_ROOT=1
. mcast-lib.sh

do_setup()
{
	mcast_setup $MCASTNUM_NORMAL
	MCAST_LCMD=ns-mcast_receiver
	MCAST_RCMD=ns-udpsender
}

do_test()
{
	tst_res TINFO "joining an IPv${TST_IPVER} multicast group with a single socket, then receiving a large number of UDP packets at the socket in $NS_DURATION seconds"

	local addr="$MCAST_IPV4_ADDR"
	[ "$TST_IPV6" ] && addr="$MCAST_IPV6_ADDR"

	local port=$(tst_get_unused_port ipv${TST_IPVER} dgram)
	[ $? -ne 0 ] && tst_brk TBROK "no free udp port available"

	# Run a receiver
	ROD ns-mcast_receiver -f $TST_IPVER -I $(tst_iface lhost) -m $addr -p $port -b

	# Run a sender
	tst_rhost_run -s -c "ns-udpsender -D $addr -f $TST_IPVER -p $port -s 32767 -m -I $(tst_iface rhost) -t $NS_DURATION"

	tst_res TPASS "test finished successfully"
}

tst_run
