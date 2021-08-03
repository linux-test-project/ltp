#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019-2021 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2014-2017 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2006
# Author: Mitsuru Chinen <mitch@jp.ibm.com>

TST_TESTFUNC="do_test"
. tst_net.sh

do_test()
{
	# not supported on IPv6
	TST_IPV6=
	TST_IPVER=4

	tst_res TINFO "Sending ICMPv4 with wrong frag. info for $NS_DURATION sec"
	tst_icmp -t $NS_DURATION -s "0 100 500 1000 $NS_ICMPV4_SENDER_DATA_MAXSIZE" -f
	tst_ping
}

tst_run
