#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2014-2018 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

TST_SETUP="setup"
TST_TESTFUNC="test"
TST_CNT=2
TST_CLEANUP="cleanup"
TST_MIN_KVER="3.7"
TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="tc"
TST_NEEDS_DRIVERS="sch_netem"
TST_OPTS="R:"
TST_USAGE=tcp_fastopen_usage
TST_PARSE_ARGS=tcp_fastopen_parse_args

srv_replies=3

tcp_fastopen_usage()
{
	echo "-R x    Number of requests, after which connection is closed"
}

tcp_fastopen_parse_args()
{
	case "$1" in
	R) srv_replies=$2 ;;
	esac
}


cleanup()
{
	tc qdisc del dev $(tst_iface) root netem delay 100 >/dev/null
}

setup()
{
	if tst_kvcmp -lt "3.16" && [ "$TST_IPV6" ]; then
		tst_brk TCONF "test must be run with kernel 3.16 or newer"
	fi

	ROD tc qdisc add dev $(tst_iface) root netem delay 100
}

test1()
{
	tst_res TINFO "using old TCP API and set tcp_fastopen to '0'"
	tst_netload -H $(tst_ipaddr rhost) -t 0 -R $srv_replies
	time_tfo_off=$(cat tst_netload.res)

	tst_res TINFO "using new TCP API and set tcp_fastopen to '3'"
	tst_netload -H $(tst_ipaddr rhost) -f -t 3 -R $srv_replies
	time_tfo_on=$(cat tst_netload.res)

	tst_netload_compare $time_tfo_off $time_tfo_on 3
}

test2()
{
	tst_kvcmp -lt "4.11" && \
		tst_brk TCONF "next test must be run with kernel 4.11 or newer"

	tst_res TINFO "using connect() and TCP_FASTOPEN_CONNECT socket option"
	tst_netload -H $(tst_ipaddr rhost) -F -t 3 -R $srv_replies
	time_tfo_on=$(cat tst_netload.res)

	tst_netload_compare $time_tfo_off $time_tfo_on 3
}

. tst_net.sh
tst_run
