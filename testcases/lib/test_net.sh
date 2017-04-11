#!/bin/sh
# Copyright (c) 2014-2016 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) 2016-2017 Petr Vorel <pvorel@suse.cz>
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
# along with this program; if not, write the Free Software Foundation,
# Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#

[ -z "$TST_LIB_LOADED" ] && . test.sh

init_ltp_netspace()
{
	if [ ! -f /var/run/netns/ltp_ns ]; then
		ROD ip net add ltp_ns
		ROD ip li add name ltp_ns_veth1 type veth peer name ltp_ns_veth2
		ROD ip li set dev ltp_ns_veth1 netns ltp_ns
		ROD ip netns exec ltp_ns ip li set lo up
	fi

	LHOST_IFACES="${LHOST_IFACES:-ltp_ns_veth2}"
	RHOST_IFACES="${RHOST_IFACES:-ltp_ns_veth1}"

	export TST_INIT_NETNS="no"
	export LTP_NETNS="${LTP_NETNS:-ip netns exec ltp_ns}"

	tst_restore_ipaddr
	tst_restore_ipaddr rhost

	tst_wait_ipv6_dad
}

# Run command on remote host.
# Options:
# -b run in background
# -s safe option, if something goes wrong, will exit with TBROK
# -c specify command to run

tst_rhost_run()
{
	local pre_cmd=
	local post_cmd=' || echo RTERR'
	local out=
	local user="root"
	local cmd=
	local safe=0

	OPTIND=0

	while getopts :bsc:u: opt; do
		case "$opt" in
		b) [ "$TST_USE_NETNS" ] && pre_cmd="" || pre_cmd="nohup"
		   post_cmd=" > /dev/null 2>&1 &"
		   out="1> /dev/null"
		;;
		s) safe=1 ;;
		c) cmd="$OPTARG" ;;
		u) user="$OPTARG" ;;
		*) tst_brkm TBROK "tst_rhost_run: unknown option: $OPTARG" ;;
		esac
	done

	OPTIND=0

	if [ -z "$cmd" ]; then
		[ "$safe" -eq 1 ] && \
			tst_brkm TBROK "tst_rhost_run: command not defined"
		tst_resm TWARN "tst_rhost_run: command not defined"
		return 1
	fi

	local output=
	local ret=0
	if [ -n "$TST_USE_SSH" ]; then
		output=`ssh -n -q $user@$RHOST "sh -c \
			'$pre_cmd $cmd $post_cmd'" $out 2>&1 || echo 'RTERR'`
	elif [ -n "$TST_USE_NETNS" ]; then
		output=`$LTP_NETNS sh -c \
			"$pre_cmd $cmd $post_cmd" $out 2>&1 || echo 'RTERR'`
	else
		output=`rsh -n -l $user $RHOST "sh -c \
			'$pre_cmd $cmd $post_cmd'" $out 2>&1 || echo 'RTERR'`
	fi
	echo "$output" | grep -q 'RTERR$' && ret=1
	if [ $ret -eq 1 ]; then
		output=$(echo "$output" | sed 's/RTERR//')
		[ "$safe" -eq 1 ] && \
			tst_brkm TBROK "'$cmd' failed on '$RHOST': '$output'"
	fi

	[ -z "$out" -a -n "$output" ] && echo "$output"

	return $ret
}

# Get test interface names for local/remote host.
# tst_get_ifaces [TYPE]
# TYPE: { lhost | rhost }; Default value is 'lhost'.
tst_get_ifaces()
{
	local type="${1:-lhost}"
	if [ "$type" = "lhost" ]; then
		echo "$LHOST_IFACES"
	else
		echo "$RHOST_IFACES"
	fi
}

# Get HW addresses from defined test interface names.
# tst_get_hwaddrs [TYPE]
# TYPE: { lhost | rhost }; Default value is 'lhost'.
tst_get_hwaddrs()
{
	local type="${1:-lhost}"
	local addr=
	local list=

	for eth in $(tst_get_ifaces $type); do

		local addr_path="/sys/class/net/${eth}/address"

		case $type in
		lhost) addr=$(cat $addr_path) ;;
		rhost) addr=$(tst_rhost_run -s -c "cat $addr_path")
		esac

		[ -z "$list" ] && list="$addr" || list="$list $addr"
	done
	echo "$list"
}

# Get test HW address.
# tst_hwaddr [TYPE] [LINK]
# TYPE: { lhost | rhost }; Default value is 'lhost'.
# LINK: link number starting from 0. Default value is '0'.
tst_hwaddr()
{
	local type="${1:-lhost}"
	local link_num="${2:-0}"
	local hwaddrs=
	link_num=$(( $link_num + 1 ))
	[ "$type" = "lhost" ] && hwaddrs=$LHOST_HWADDRS || hwaddrs=$RHOST_HWADDRS
	echo "$hwaddrs" | awk '{ print $'"$link_num"' }'
}

# Get test interface name.
# tst_iface [TYPE] [LINK]
# TYPE: { lhost | rhost }; Default value is 'lhost'.
# LINK: link number starting from 0. Default value is '0'.
tst_iface()
{
	local type="${1:-lhost}"
	local link_num="${2:-0}"
	link_num="$(( $link_num + 1 ))"
	echo "$(tst_get_ifaces $type)" | awk '{ print $'"$link_num"' }'
}

# Blank for an IPV4 test; 6 for an IPV6 test.
TST_IPV6=

tst_read_opts()
{
	OPTIND=0
	while getopts ":6" opt; do
		case "$opt" in
		6)
			TST_IPV6=6;;
		esac
	done
	OPTIND=0
}

tst_read_opts $*

# Get IP address
# tst_ipaddr [TYPE]
# TYPE: { lhost | rhost }; Default value is 'lhost'.
tst_ipaddr()
{
	local type="${1:-lhost}"
	local ipv="${TST_IPV6:-4}"
	local tst_host=

	if [ "$type" = "lhost" ]; then
		eval "tst_host=\$LHOST_IPV${ipv}_HOST"
	else
		eval "tst_host=\$RHOST_IPV${ipv}_HOST"
	fi

	if [ "$TST_IPV6" ]; then
		echo "${IPV6_NETWORK}:${tst_host}"
	else
		echo "${IPV4_NETWORK}.${tst_host}"
	fi
}

# tst_init_iface [TYPE] [LINK]
# TYPE: { lhost | rhost }; Default value is 'lhost'.
# LINK: link number starting from 0. Default value is '0'.
tst_init_iface()
{
	local type="${1:-lhost}"
	local link_num="${2:-0}"
	local iface="$(tst_iface $type $link_num)"
	tst_resm TINFO "initialize '$type' '$iface' interface"

	if [ "$type" = "lhost" ]; then
		ip xfrm policy flush || return $?
		ip xfrm state flush || return $?
		ip link set $iface down || return $?
		ip route flush dev $iface || return $?
		ip addr flush dev $iface || return $?
		ip link set $iface up
		return $?
	fi

	tst_rhost_run -c "ip xfrm policy flush" || return $?
	tst_rhost_run -c "ip xfrm state flush" || return $?
	tst_rhost_run -c "ip link set $iface down" || return $?
	tst_rhost_run -c "ip route flush dev $iface" || return $?
	tst_rhost_run -c "ip addr flush dev $iface" || return $?
	tst_rhost_run -c "ip link set $iface up"
}

# tst_add_ipaddr [TYPE] [LINK]
# TYPE: { lhost | rhost }; Default value is 'lhost'.
# LINK: link number starting from 0. Default value is '0'.
tst_add_ipaddr()
{
	local type="${1:-lhost}"
	local link_num="${2:-0}"

	local mask=24
	[ "$TST_IPV6" ] && mask=64

	local iface=$(tst_iface $type $link_num)

	if [ $type = "lhost" ]; then
		tst_resm TINFO "set local addr $(tst_ipaddr)/$mask"
		ip addr add $(tst_ipaddr)/$mask dev $iface
		return $?
	fi

	tst_resm TINFO "set remote addr $(tst_ipaddr rhost)/$mask"
	tst_rhost_run -c "ip addr add $(tst_ipaddr rhost)/$mask dev $iface"
}

# tst_restore_ipaddr [TYPE] [LINK]
# Restore default ip addresses defined in network.sh
# TYPE: { lhost | rhost }; Default value is 'lhost'.
# LINK: link number starting from 0. Default value is '0'.
tst_restore_ipaddr()
{
	local type="${1:-lhost}"
	local link_num="${2:-0}"

	tst_init_iface $type $link_num || return $?

	local ret=0
	local backup_tst_ipv6=$TST_IPV6
	TST_IPV6= tst_add_ipaddr $type $link_num || ret=$?
	TST_IPV6=6 tst_add_ipaddr $type $link_num || ret=$?
	TST_IPV6=$backup_tst_ipv6

	return $ret
}

# tst_wait_ipv6_dad [LHOST_IFACE] [RHOST_IFACE]
# wait for IPv6 DAD completion
tst_wait_ipv6_dad()
{
	local ret=
	local i=
	local iface_loc=${1:-$(tst_iface)}
	local iface_rmt=${2:-$(tst_iface rhost)}

	for i in $(seq 1 50); do
		ip a sh $iface_loc | grep -q tentative
		ret=$?

		tst_rhost_run -c "ip a sh $iface_rmt | grep -q tentative"

		[ $ret -ne 0 -a $? -ne 0 ] && return

		[ $(($i % 10)) -eq 0 ] && \
			tst_resm TINFO "wait for IPv6 DAD completion $((i / 10))/5 sec"

		tst_sleep 100ms
	done
}

# Run network load test, see 'netstress -h' for option description
tst_netload()
{
	local rfile="tst_netload.res"
	local expect_res="pass"
	local ret=0

	# common options for client and server
	local cs_opts=

	local c_num="${TST_NETLOAD_CLN_NUMBER:-2}"
	local c_requests="${TST_NETLOAD_CLN_REQUESTS:-500000}"
	local c_opts=

	# number of server replies after which TCP connection is closed
	local s_replies="${TST_NETLOAD_MAX_SRV_REPLIES:-500000}"
	local s_opts=

	OPTIND=0
	while getopts :a:H:d:n:N:r:R:b:t:Ufe: opt; do
		case "$opt" in
		a) c_num="$OPTARG" ;;
		H) c_opts="${c_opts}-H $OPTARG " ;;
		d) rfile="$OPTARG" ;;
		n) c_opts="${c_opts}-n $OPTARG " ;;
		N) c_opts="${c_opts}-N $OPTARG " ;;
		r) c_requests="$OPTARG" ;;
		R) s_replies="$OPTARG" ;;
		b) cs_opts="${cs_opts}-b $OPTARG " ;;
		t) cs_opts="${cs_opts}-t $OPTARG " ;;
		U) cs_opts="${cs_opts}-U " ;;
		f) cs_opts="${cs_opts}-f " ;;

		e) expect_res="$OPTARG" ;;
		*) tst_brkm TBROK "tst_netload: unknown option: $OPTARG" ;;
		esac
	done
	OPTIND=0

	local expect_ret=0
	[ "$expect_res" != "pass" ] && expect_ret=1

	local port="$(tst_rhost_run -c 'tst_get_unused_port ipv6 stream')"
	[ $? -ne 0 ] && tst_brkm TBROK "failed to get unused port"

	tst_rhost_run -c "pkill -9 netstress\$"

	c_opts="${cs_opts}${c_opts}-a $c_num -r $c_requests -d $rfile -g $port"
	s_opts="${cs_opts}${s_opts}-R $s_replies -g $port"

	tst_resm TINFO "run server 'netstress $s_opts'"
	tst_rhost_run -s -b -c "netstress $s_opts"

	tst_resm TINFO "check that server port in 'LISTEN' state"
	local sec_waited=
	for sec_waited in $(seq 1 600); do
		tst_rhost_run -c "ss -lutn | grep -q $port" && break
		if [ $sec_waited -eq 600 ]; then
			tst_rhost_run -c "ss -utnp | grep $port"
			tst_brkm TFAIL "server not in LISTEN state"
		fi
		tst_sleep 100ms
	done

	tst_resm TINFO "run client 'netstress -l $c_opts'"
	netstress -l $c_opts > tst_netload.log 2>&1 || ret=1
	tst_rhost_run -c "pkill -9 netstress\$"

	if [ "$expect_ret" -ne "$ret" ]; then
		cat tst_netload.log
		tst_brkm TFAIL "expected '$expect_res' but ret: '$ret'"
	fi

	if [ "$ret" -eq 0 ]; then
		if [ ! -f $rfile ]; then
			cat tst_netload.log
			tst_brkm TFAIL "can't read $rfile"
		fi
		tst_resm TPASS "netstress passed, time spent '$(cat $rfile)' ms"
	else
		tst_resm TPASS "netstress failed as expected"
	fi

	return $ret
}

# tst_ping [IFACE] [DST ADDR] [MESSAGE SIZE ARRAY]
# Check icmp connectivity
# IFACE: source interface name
# DST ADDR: destination IPv4 or IPv6 address
# MESSAGE SIZE ARRAY: message size array
tst_ping()
{
	# The max number of ICMP echo request
	PING_MAX="${PING_MAX:-500}"

	local src_iface="${1:-$(tst_iface)}"
	local dst_addr="${2:-$(tst_ipaddr rhost)}"; shift $(( $# >= 2 ? 2 : 0 ))
	local msg_sizes="$*"
	local ret=0

	# ping cmd use 56 as default message size
	for size in ${msg_sizes:-"56"}; do
		ping$TST_IPV6 -I $src_iface -c $PING_MAX $dst_addr \
			-s $size -i 0 > /dev/null 2>&1
		ret=$?
		if [ $ret -eq 0 ]; then
			tst_resm TINFO "tst_ping IPv${TST_IPV6:-4} msg_size $size pass"
		else
			tst_resm TINFO "tst_ping IPv${TST_IPV6:-4} msg_size $size fail"
			break
		fi
	done
	return $ret
}

# tst_icmp -t TIMEOUT -s MESSAGE_SIZE_ARRAY OPTS
# TIMEOUT: total time for the test in seconds
# OPTS: additional options for ns-icmpv4|6-sender tool
tst_icmp()
{
	local timeout=1
	local msg_sizes=56
	local opts=
	local num=
	local ret=0
	local ver="${TST_IPV6:-4}"

	OPTIND=0
	while getopts :t:s: opt; do
		case "$opt" in
		t) timeout="$OPTARG" ;;
		s) msg_sizes="$OPTARG" ;;
		*) opts="-$OPTARG $opts" ;;
		esac
	done
	OPTIND=0

	local num=$(echo "$msg_sizes" | wc -w)
	timeout="$(($timeout / $num))"
	[ "$timeout" -eq 0 ] && timeout=1

	opts="${opts}-I $(tst_iface) -S $(tst_ipaddr) -D $(tst_ipaddr rhost) "
	opts="${opts}-M $(tst_hwaddr rhost) -t $timeout"

	for size in $msg_sizes; do
		ns-icmpv${ver}_sender -s $size $opts
		ret=$?
		if [ $ret -eq 0 ]; then
			tst_resm TPASS "'ns-icmpv${ver}_sender -s $size $opts' pass"
		else
			tst_resm TFAIL "'ns-icmpv${ver}_sender -s $size $opts' fail"
			break
		fi
	done
	return $ret
}

# tst_set_sysctl NAME VALUE [safe]
# It can handle netns case when sysctl not namespaceified.
tst_set_sysctl()
{
	local name="$1"
	local value="$2"
	local safe=
	[ "$3" = "safe" ] && safe="-s"

	local add_opt=
	[ "$TST_USE_NETNS" = "yes" ] && add_opt="-e"

	if [ "$safe" ]; then
		ROD sysctl -qw $name=$value
	else
		sysctl -qw $name=$value
	fi

	tst_rhost_run $safe -c "sysctl -qw $add_opt $name=$value"
}

# Management Link
[ -z "$RHOST" ] && TST_USE_NETNS="yes"
export RHOST="$RHOST"
export PASSWD="${PASSWD:-}"
# Don't use it in new tests, use tst_rhost_run() from test_net.sh instead.
export LTP_RSH="${LTP_RSH:-rsh -n}"

# Test Links
# Set first three octets of the network address, default is '10.0.0'
export IPV4_NETWORK="${IPV4_NETWORK:-10.0.0}"
# Set local host last octet, default is '2'
export LHOST_IPV4_HOST="${LHOST_IPV4_HOST:-2}"
# Set remote host last octet, default is '1'
export RHOST_IPV4_HOST="${RHOST_IPV4_HOST:-1}"
# Set the reverse of IPV4_NETWORK
export IPV4_NET_REV="${IPV4_NET_REV:-0.0.10}"
# Set first three octets of the network address, default is 'fd00:1:1:1'
export IPV6_NETWORK="${IPV6_NETWORK:-fd00:1:1:1}"
# Set local host last octet, default is '2'
export LHOST_IPV6_HOST="${LHOST_IPV6_HOST:-:2}"
# Set remote host last octet, default is '1'
export RHOST_IPV6_HOST="${RHOST_IPV6_HOST:-:1}"

# Networks that aren't reachable through the test links
export IPV4_NET16_UNUSED="${IPV4_NET16_UNUSED:-10.23}"
export IPV6_NET32_UNUSED="${IPV6_NET32_UNUSED:-fd00:23}"

export HTTP_DOWNLOAD_DIR="${HTTP_DOWNLOAD_DIR:-/var/www/html}"
export FTP_DOWNLOAD_DIR="${FTP_DOWNLOAD_DIR:-/var/ftp}"
export FTP_UPLOAD_DIR="${FTP_UPLOAD_DIR:-/var/ftp/pub}"
export FTP_UPLOAD_URLDIR="${FTP_UPLOAD_URLDIR:-pub}"

# network/stress tests require additional parameters
export NS_DURATION="${NS_DURATION:-3600}"
export NS_TIMES="${NS_TIMES:-10000}"
export CONNECTION_TOTAL="${CONNECTION_TOTAL:-4000}"
export IP_TOTAL="${IP_TOTAL:-10000}"
export IP_TOTAL_FOR_TCPIP="${IP_TOTAL_FOR_TCPIP:-100}"
export ROUTE_TOTAL="${ROUTE_TOTAL:-10000}"
export MTU_CHANGE_TIMES="${MTU_CHANGE_TIMES:-1000}"
export IF_UPDOWN_TIMES="${IF_UPDOWN_TIMES:-10000}"
export DOWNLOAD_BIGFILESIZE="${DOWNLOAD_BIGFILESIZE:-2147483647}"
export DOWNLOAD_REGFILESIZE="${DOWNLOAD_REGFILESIZE:-1048576}"
export UPLOAD_BIGFILESIZE="${UPLOAD_BIGFILESIZE:-2147483647}"
export UPLOAD_REGFILESIZE="${UPLOAD_REGFILESIZE:-1024}"
export MCASTNUM_NORMAL="${MCASTNUM_NORMAL:-20}"
export MCASTNUM_HEAVY="${MCASTNUM_HEAVY:-40000}"

[ -n "$TST_USE_NETNS" -a "$TST_INIT_NETNS" != "no" ] && init_ltp_netspace

# Warning: make sure to set valid interface names and IP addresses below.
# Set names for test interfaces, e.g. "eth0 eth1"
export LHOST_IFACES="${LHOST_IFACES:-eth0}"
export RHOST_IFACES="${RHOST_IFACES:-eth0}"
# Set corresponding HW addresses, e.g. "00:00:00:00:00:01 00:00:00:00:00:02"
export LHOST_HWADDRS="${LHOST_HWADDRS:-$(tst_get_hwaddrs lhost)}"
export RHOST_HWADDRS="${RHOST_HWADDRS:-$(tst_get_hwaddrs rhost)}"

# More information about network parameters can be found
# in the following document: testcases/network/stress/README
