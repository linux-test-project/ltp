#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2006 International Business Machines  Corp.
# Copyright (c) 2020 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Copyright (c) 2021 Petr Vorel <pvorel@suse.cz>
# Author: Mitsuru Chinen <mitch@jp.ibm.com>
#
# Verify that the kernel is not crashed when joining a multicast group
# on a single socket, then receiving a large number of Multicast Address
# Specific Query

TST_NEEDS_ROOT=1
TST_NEEDS_TMPDIR=1
TST_SETUP="mcast_setup_normal"
TST_CLEANUP="mcast_cleanup"
TST_TESTFUNC="do_test"

do_test()
{
	tst_res TINFO "joining an IPv${TST_IPVER} multicast group on a single socket, then receiving a large number of Multicast Address Specific Query in $NS_DURATION seconds"

	local extra="-m $MCAST_IPV4_ADDR"
	[ "$TST_IPV6" ] && extra="-D $MCAST_IPV6_ADDR"

	# Send IGMP Multicast Address Specific Query from the remote host
	do_multicast_test_join_single_socket "$extra"
}

. mcast-lib.sh
tst_run
