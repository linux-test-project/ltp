#!/bin/sh
# Copyright (c) International Business Machines  Corp., 2006
# Copyright (c) 2015-2017 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it would be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Author: Petr Vorel <pvorel@suse.cz>
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Library for all network/stress/ tests.
# NOTE: More information about network variables can be found
# in test_net.sh and testcases/network/stress/README.

export TCID="${TCID:-$(basename $0)}"

. test_net.sh

ipver=${TST_IPV6:-4}

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
	tst_require_root
	tst_check_cmds ip pgrep pkill
	trap "tst_brkm TBROK 'test interrupted'" INT
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

	tst_resm TINFO "ping through $src_iface iface to ${dst_addr}$cnt_msg"

	tst_ping $src_iface $dst_addr
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
	local port=$(tst_get_unused_port ipv${ipver} stream)

	netstress -R 3 -g $port > /dev/null 2>&1 &
	tst_rhost_run -b -c "netstress -l -H $ip -g $port"
}
