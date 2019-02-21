#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Oracle and/or its affiliates.

TST_CNT=2
TST_TESTFUNC="test"
TST_MIN_KVER="4.2"
TST_NEEDS_TMPDIR=1
TST_TEST_DATA="tcp udp udp_lite dccp"

. tst_net.sh

test1()
{
	local type="$2"

	tst_res TINFO "test IP_BIND_ADDRESS_NO_PORT with $type socket"
	# when using '-S' parameter, netstress sets IP_BIND_ADDRESS_NO_PORT
	tst_netload -T $type -S $(tst_ipaddr) -H $(tst_ipaddr rhost)
}

test2()
{
	local type="$2"
	local rsize="65000"

	[ "$type" = "dccp" ] && rsize=1300

	tst_res TINFO "test bind() with $type socket and random size payload"
	tst_netload -T $type -S $(tst_ipaddr) -H $(tst_ipaddr rhost) -A $rsize
}


tst_run
