#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) 2019-2022 Petr Vorel <pvorel@suse.cz>
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="sysctl tc"
TST_NEEDS_DRIVERS="sch_netem"

def_alg="cubic"
prev_qlen=
prev_queue=
prev_alg=

set_cong_alg()
{
	local alg=$1
	tst_res TINFO "setting $alg"

	tst_set_sysctl net.ipv4.tcp_congestion_control $alg safe
}

tcp_cc_cleanup()
{
	local rmt_dev="dev $(tst_iface rhost)"

	[ "$prev_cong_ctl" ] && \
		tst_set_sysctl net.ipv4.tcp_congestion_control $prev_alg

	[ "$prev_qlen" ] && \
		tst_rhost_run -c "ip link set txqueuelen $prev_qlen $rmt_dev"

	[ "$prev_queue" ] && \
		tst_rhost_run -c "tc qdisc replace $rmt_dev root $prev_queue"
}

tcp_cc_check_support()
{
	local proc_cc="/proc/sys/net/ipv4/tcp_available_congestion_control"
	local alg="$1"

	modprobe tcp_$alg > /dev/null 2>&1
	grep -q $alg $proc_cc || tst_brk TCONF "Local host doesn't support $alg"

	if [ -z "$TST_USE_NETNS" ]; then
		tst_rhost_run -c "modprobe tcp_$alg" > /dev/null 2>&1
		tst_rhost_run -c "grep -q $alg $proc_cc" || \
			tst_brk TCONF "Remote host doesn't support $alg"
	fi
}

tcp_cc_setup()
{
	prev_alg="$(sysctl -n net.ipv4.tcp_congestion_control)"
}

tcp_cc_set_qdisc()
{
	local qdisc="$1"
	local qlen="${2:-1000}"
	local rmt_dev="$(tst_iface rhost)"

	tst_res TINFO "set qdisc on $(tst_iface rhost) to $qdisc len $qlen"

	[ -z "$prev_qlen" ] && \
		prev_qlen=$(tst_rhost_run -s -c \
			    "cat /sys/class/net/$rmt_dev/tx_queue_len")
	tst_rhost_run -s -c "ip link set txqueuelen $qlen dev $rmt_dev"

	[ -z "$prev_queue" ] && \
		prev_queue=$(tst_rhost_run -s -c \
			     "tc qdisc show dev $rmt_dev | head -1" | \
			     cut -f2 -d' ')
	[ "$qdisc" = "$prev_queue" ] && return 0

	tst_rhost_run -c "tc qdisc replace dev $rmt_dev root $qdisc" >/dev/null
	if [ $? -ne 0 ]; then
		tst_res TCONF "$qdisc qdisc not supported"
		return 1
	fi

	return 0
}

tcp_cc_test01()
{
	local alg=$1
	local threshold=${2:-10}

	tst_res TINFO "compare '$def_alg' and '$alg' congestion alg. results"

	set_cong_alg "$def_alg"

	tst_netload -H $(tst_ipaddr rhost) -A $TST_NET_MAX_PKT
	local res0="$(cat tst_netload.res)"

	set_cong_alg "$alg"

	tst_netload -H $(tst_ipaddr rhost) -A $TST_NET_MAX_PKT
	local res1="$(cat tst_netload.res)"

	tst_netload_compare $res0 $res1 $threshold
}

. tst_net.sh
