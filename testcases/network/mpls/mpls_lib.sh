#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Oracle and/or its affiliates. All Rights Reserved.

TST_MIN_KVER="4.3"
TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1
TST_NEEDS_DRIVERS="mpls_router mpls_iptunnel mpls_gso"
TST_NEEDS_CMDS="sysctl modprobe"
TST_TEST_DATA="icmp tcp udp"

. tst_net.sh

mpls_cleanup()
{
	local flush_dev="ip -f mpls route flush dev"

	$flush_dev lo > /dev/null 2>&1
	tst_rhost_run -c "$flush_dev lo" > /dev/null

	[ -n "$rpf_loc" ] && sysctl -q net.ipv4.conf.all.rp_filter=$rpf_loc
	[ -n "$rpf_rmt" ] && tst_rhost_run -s -c "sysctl -q net.ipv4.conf.all.rp_filter=$rpf_rmt"
}

mpls_setup()
{
	local label="$1"

	ROD modprobe -a $TST_NEEDS_DRIVERS
	ROD sysctl -q net.mpls.conf.$(tst_iface).input=1
	tst_set_sysctl net.mpls.conf.lo.input 1 safe
	tst_set_sysctl net.mpls.platform_labels $label safe
	rpf_loc="$(sysctl -n net.ipv4.conf.all.rp_filter)"

	tst_rhost_run -s -c "modprobe -a $TST_NEEDS_DRIVERS"
	tst_rhost_run -s -c "sysctl -q net.mpls.conf.$(tst_iface rhost).input=1"
	rpf_rmt="$(tst_rhost_run -c 'sysctl -n net.ipv4.conf.all.rp_filter')"

	tst_set_sysctl net.ipv4.conf.all.rp_filter 2 safe
}
