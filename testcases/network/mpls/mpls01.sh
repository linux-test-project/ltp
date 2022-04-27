#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Oracle and/or its affiliates. All Rights Reserved.

TST_SETUP="setup"
TST_TESTFUNC="test"
TST_CLEANUP="cleanup"
TST_CNT=3
TST_MIN_KVER="4.1"
TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1
TST_NEEDS_DRIVERS="mpls_router"
TST_NEEDS_CMDS="sysctl modprobe"


cleanup()
{
	ip -f mpls route flush dev lo > /dev/null 2>&1
	ip -f mpls route flush dev $(tst_iface) > /dev/null 2>&1
}

setup()
{
	ROD modprobe mpls_router
}

test1()
{
	ROD sysctl -q net.mpls.platform_labels=0xfffff
	ROD ip -f mpls route add 0xffffe dev lo
	ROD ip -f mpls route show \> /dev/null
	ROD ip -f mpls route del 0xffffe dev lo
	tst_res TPASS "added label 0xffffe to lo dev"
}

test2()
{
	ROD sysctl -q net.mpls.platform_labels=0xffffe
	ip -f mpls route add 0xffffe dev lo > /dev/null 2>&1
	if [ $? -eq 0 ]; then
		tst_res TFAIL "can add label that is >= platform_labels"
		return
	fi

	tst_res TPASS "can't add label >= platform_lables"
}

test3()
{
	local start=16
	local end=$((start + NS_TIMES))

	ROD sysctl -q net.mpls.platform_labels=$((end + 1))
	tst_res TINFO "creating mpls routes with labels from $start..$end"
	for l in $(seq $start $end); do
		ROD ip -f mpls route add $l dev $(tst_iface)
	done

	tst_res TINFO "listing created routes"
	ROD ip -f mpls route show \> /dev/null

	tst_res TINFO "removing the routes"
	for l in $(seq $start $end); do
		ROD ip -f mpls route del $l dev $(tst_iface)
	done
	tst_res TPASS "created and removed mpls routes"
}

. tst_net.sh
tst_run
