#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Oracle and/or its affiliates.

TST_TESTFUNC="test"
TST_MIN_KVER="4.2"
TST_NEEDS_TMPDIR=1
TST_TEST_DATA="tcp udp udp_lite dccp"

. tst_net.sh

test()
{
	local type="$2"

	tst_res TINFO "test IP_BIND_ADDRESS_NO_PORT with $type socket"
	# when using '-S' parameter, netstress sets IP_BIND_ADDRESS_NO_PORT
	tst_netload -T $type -S $(tst_ipaddr) -H $(tst_ipaddr rhost)
}

tst_run
