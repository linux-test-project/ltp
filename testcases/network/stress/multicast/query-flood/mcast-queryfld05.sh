#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2006 International Business Machines  Corp.
# Copyright (c) 2020 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Mitsuru Chinen <mitch@jp.ibm.com>
#
# Verify that the kernel is not crashed when joining multiple
# multicast groups on separate sockets, then receiving a large number of
# Multicast Address Specific Queries

TST_NEEDS_ROOT=1
TST_NEEDS_TMPDIR=1
TST_SETUP="mcast_setup_normal"
TST_CLEANUP="mcast_cleanup"
TST_TESTFUNC="do_test"

do_test()
{
	tst_res TINFO "joining $MCASTNUM_NORMAL IPv${TST_IPVER} multicast groups on separate sockets, then receiving a large number of Multicast Address Specific Queries in $NS_DURATION seconds"

	local prefix="$MCAST_IPV4_ADDR_PREFIX"
	[ "$TST_IPV6" ] && prefix="$MCAST_IPV6_ADDR_PREFIX"

	# Run a multicast join tool
	local tmpfile=$$
	EXPECT_PASS $MCAST_LCMD -n $MCASTNUM_NORMAL -p $prefix \> $tmpfile
	tst_res TINFO "joined $(grep groups $tmpfile)"

	# Send IGMP Multicast Address Specific Query from the remote host
	local n=0
	while [ $n -lt $MCASTNUM_NORMAL ]; do
		# Define the multicast address
		if [ "$TST_IPV6" ]; then
			local n_hex=$(printf "%x" $n)
			local addr=${MCAST_IPV6_ADDR_PREFIX}:${n_hex}
		else
			local x=$((n / 254))
			local y=$((n % 254 + 1))
			local addr=${MCAST_IPV4_ADDR_PREFIX}.${x}.${y}
		fi

		local params="-m $addr"
		[ "$TST_IPV6" ] && params="-S $(tst_ipaddr) -m -D $addr"
		tst_rhost_run -c "$MCAST_RCMD -t $NS_DURATION -r 0 -b $params"

		n=$((n+1))
	done

	sleep $NS_DURATION

	tst_res TPASS "test finished successfully"
}

. mcast-lib.sh
tst_run
