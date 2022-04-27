#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Linux Test Project, 2018-2022
# Copyright (c) 2018 Oracle and/or its affiliates. All Rights Reserved.

TST_MIN_KVER="4.3"
TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1
TST_NEEDS_DRIVERS="mpls_router mpls_iptunnel mpls_gso"
TST_NEEDS_CMDS="sysctl modprobe"
TST_TEST_DATA="icmp tcp udp"
TST_NETLOAD_BINDTODEVICE=

mpls_cleanup()
{
	local flush_dev="ip -f mpls route flush dev"

	$flush_dev lo > /dev/null 2>&1
	tst_rhost_run -c "$flush_dev lo" > /dev/null

	[ -n "$rpf_loc" ] && sysctl -q net.ipv4.conf.all.rp_filter=$rpf_loc
	[ -n "$rpf_rmt" ] && tst_rhost_run -s -c "sysctl -q net.ipv4.conf.all.rp_filter=$rpf_rmt"
}

mpls_virt_cleanup()
{
	ip route del $ip_virt_remote/32 dev ltp_v0 > /dev/null 2>&1
	ip route del $ip6_virt_remote/128 dev ltp_v0 > /dev/null 2>&1
	tst_rhost_run -c "ip route del $ip_virt_local/32 dev ltp_v0" > /dev/null
	tst_rhost_run -c "ip route del $ip6_virt_local/128 dev ltp_v0" > /dev/null

	virt_cleanup
	mpls_cleanup
}

mpls_setup()
{
	local label="$1"

	tst_net_run -s "modprobe -a $TST_NEEDS_DRIVERS"

	ROD sysctl -q net.mpls.conf.$(tst_iface).input=1
	tst_set_sysctl net.mpls.conf.lo.input 1 safe
	tst_set_sysctl net.mpls.platform_labels $label safe
	rpf_loc="$(sysctl -n net.ipv4.conf.all.rp_filter)"

	tst_rhost_run -s -c "sysctl -q net.mpls.conf.$(tst_iface rhost).input=1"
	rpf_rmt="$(tst_rhost_run -c 'sysctl -n net.ipv4.conf.all.rp_filter')"

	tst_set_sysctl net.ipv4.conf.all.rp_filter 2 safe
}

mpls_setup_tnl()
{
	local ip_loc="$1"
	local ip_rmt="$2"
	local label="$3"
	local mask

	echo "$ip_loc" | grep -q ':' && mask=128 || mask=32

	ROD ip route add $ip_rmt/$mask encap mpls $label dev ltp_v0
	ROD ip -f mpls route add $((label + 1)) dev lo

	tst_rhost_run -s -c "ip route add $ip_loc/$mask encap mpls $((label + 1)) dev ltp_v0"
	tst_rhost_run -s -c "ip -f mpls route add $label dev lo"
}
mpls_virt_setup()
{
	mpls_setup 62

	virt_lib_setup

	tst_res TINFO "test $virt_type with MPLS"
	virt_setup "local $(tst_ipaddr) remote $(tst_ipaddr rhost) dev $(tst_iface)" \
		   "local $(tst_ipaddr rhost) remote $(tst_ipaddr) dev $(tst_iface rhost)"

	mpls_setup_tnl $ip_virt_local $ip_virt_remote 60
	mpls_setup_tnl $ip6_virt_local $ip6_virt_remote 50

	tst_set_sysctl net.mpls.conf.ltp_v0.input 1 safe
}

mpls_virt_test()
{
	local type=$2
	local max_size=$TST_NET_MAX_PKT

	if [ "$type" = "icmp" ]; then
		tst_ping -I $ip_virt_local -H $ip_virt_remote -s "10 100 1000 2000 $max_size"
		tst_ping -I $ip6_virt_local -H $ip6_virt_remote -s "10 100 1000 2000 $max_size"
	else
		tst_netload -S $ip_virt_local -H $ip_virt_remote -T $type -n 10 -N 10
		tst_netload -S $ip6_virt_local -H $ip6_virt_remote -T $type -n 10 -N 10
		tst_netload -S $ip_virt_local -H $ip_virt_remote -T $type -A $max_size
		tst_netload -S $ip6_virt_local -H $ip6_virt_remote -T $type -A $max_size
	fi
}

. tst_net.sh
