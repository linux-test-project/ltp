#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2017-2018 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2015-2017 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines Corp., 2006
# Author: Petr Vorel <pvorel@suse.cz>
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Library for all network/stress/ tests.
# NOTE: More information about network variables can be found
# in tst_net.sh and testcases/network/stress/README.

# Netmask of for the tested network
IPV4_NETMASK="255.255.255.0"
IPV4_NETMASK_NUM=24

# Multicast address and it's prefix
MCAST_IPV4_ADDR_PREFIX="224.10"
MCAST_IPV4_ADDR="${MCAST_IPV4_ADDR_PREFIX}.10.1"
MCAST_IPV6_ADDR_PREFIX="ff0e::1111"
MCAST_IPV6_ADDR="${MCAST_IPV6_ADDR_PREFIX}:1"

# Setup for tests using netstress.
netstress_setup()
{
	TST_NEEDS_ROOT=1
	tst_require_cmds pgrep pkill
}

# Cleanup for tests using netstress.
netstress_cleanup()
{
	# Stop the background TCP traffic
	pkill -13 -x netstress
	tst_rhost_run -c "pkill -13 -x netstress"
}

# restore_ipaddr [TYPE] [LINK] [LOCAL_IFACE] [REMOTE_IFACE]
# TYPE: { lhost | rhost }; Default value is 'lhost'.
# LINK: link number starting from 0. Default value is '0'.
# LOCAL_IFACE: local iface name.
# REMOTE_IFACE: local iface name.
restore_ipaddr()
{
	local type="${1:-lhost}"
	local link_num="${2:-0}"
	local iface_loc=${3:-$(tst_iface lhost $link_num)}
	local iface_rmt=${4:-$(tst_iface rhost $link_num)}

	tst_restore_ipaddr $type $link_num || return $?
	[ $type = "lhost" ] && tst_wait_ipv6_dad $iface_loc $iface_rmt
}

# Check connectivity with tst_ping.
# check_connectivity SRC_IFACE DST_ADDR [CNT]
# SRC_IFACE: source interface name.
# DST_ADDR: destination IPv4 or IPv6 address.
# CNT: loop step.
check_connectivity()
{
	local src_iface="${1}"
	local dst_addr="${2}"
	local cnt="${3:-}"
	local cnt_msg

	[ -n "$cnt" ] && cnt_msg=" (step $cnt)"

	tst_res TINFO "ping through $src_iface iface to ${dst_addr}$cnt_msg"

	tst_ping -I $src_iface -H $dst_addr
}

# check_connectivity_interval CNT [RESTORE] [SRC_IFACE] [DST_ADDR]
# CNT: loop step.
# RESTORE: whether restore ip addr.
# SRC_IFACE: source interface name.
# DST_ADDR: destination IPv4 or IPv6 address.
check_connectivity_interval()
{
	local cnt="$1"
	local restore="${2:-false}"
	local src_iface="${3:-$(tst_iface)}"
	local dst_addr="${4:-$(tst_ipaddr rhost)}"

	[ $CHECK_INTERVAL -eq 0 ] && return

	[ $(($cnt % $CHECK_INTERVAL)) -ne 0 ] && return

	[ "$restore" != "false" ] && restore_ipaddr

	check_connectivity $src_iface $dst_addr $cnt
}

# Run netstress process on both lhost and rhost.
# make_background_tcp_traffic [IP]
# IP: server IP; Default value is $(tst_ipaddr).
make_background_tcp_traffic()
{
	pgrep -x netstress > /dev/null && return

	local ip="${1:-$(tst_ipaddr)}"
	local port=$(tst_get_unused_port ipv${TST_IPVER} stream)

	netstress -R 3 -g $port > /dev/null 2>&1 &
	tst_rhost_run -b -c "netstress -l -H $ip -g $port"
}

test_if_ip()
{
	case $1 in
	1) test_body 'if_cmd';;
	2) test_body 'ip_cmd';;
	esac
}

test_rt_ip()
{
	case $1 in
	1) test_body 'rt_cmd';;
	2) test_body 'ip_cmd';;
	esac
}

. tst_net.sh
