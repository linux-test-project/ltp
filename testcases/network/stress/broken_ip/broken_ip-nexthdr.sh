#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019-2021 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2014-2017 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2006
# Author: Mitsuru Chinen <mitch@jp.ibm.com>

TST_TESTFUNC="do_test"

do_test()
{
	tst_res TINFO "Sending ICMPv6 with wrong next header for $NS_DURATION sec"
	tst_icmp -t $NS_DURATION -s "0 100 500 1000 $NS_ICMPV6_SENDER_DATA_MAXSIZE" -n
	tst_ping
}

. tst_net.sh
# not supported on IPv4
TST_IPV6=6
TST_IPVER=6

tst_run
