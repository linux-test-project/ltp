#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2014-2017 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) 2016-2018 Petr Vorel <pvorel@suse.cz>
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

[ -n "$TST_LIB_NET_LOADED" ] && return 0
TST_LIB_NET_LOADED=1

TST_OPTS="6$TST_OPTS"
TST_PARSE_ARGS_CALLER="$TST_PARSE_ARGS"
TST_PARSE_ARGS="tst_net_parse_args"
TST_USAGE_CALLER="$TST_USAGE"
TST_USAGE="tst_net_usage"
TST_SETUP_CALLER="$TST_SETUP"
TST_SETUP="tst_net_setup"

# Blank for an IPV4 test; 6 for an IPV6 test.
TST_IPV6=${TST_IPV6:-}
TST_IPVER=${TST_IPV6:-4}

tst_net_parse_args()
{
	case $1 in
	6) TST_IPV6=6 TST_IPVER=6;;
	*) $TST_PARSE_ARGS_CALLER "$1" "$2";;
	esac
}

tst_net_read_opts()
{
	local OPTIND
	while getopts ":$TST_OPTS" opt; do
		$TST_PARSE_ARGS "$opt" "$OPTARG"
	done
}

tst_net_usage()
{
	if [ -n "$TST_USAGE_CALLER" ]; then
		$TST_USAGE_CALLER
	else
		echo "Usage: $0 [-6]"
		echo "OPTIONS"
	fi
	echo "-6      IPv6 tests"
}

tst_net_remote_tmpdir()
{
	[ "$TST_NEEDS_TMPDIR" = 1 ] || return 0
	[ -n "$TST_USE_LEGACY_API" ] && tst_tmpdir
	tst_rhost_run -c "mkdir -p $TST_TMPDIR"
	tst_rhost_run -c "chmod 777 $TST_TMPDIR"
	export TST_TMPDIR_RHOST=1
}

tst_net_setup()
{
	tst_net_remote_tmpdir
	[ -n "$TST_SETUP_CALLER" ] && $TST_SETUP_CALLER
}

[ -n "$TST_USE_LEGACY_API" ] && . test.sh || . tst_test.sh

if [ "$TST_PARSE_ARGS_CALLER" = "$TST_PARSE_ARGS" ]; then
	tst_res TWARN "TST_PARSE_ARGS_CALLER same as TST_PARSE_ARGS, unset it ($TST_PARSE_ARGS)"
	unset TST_PARSE_ARGS_CALLER
fi
if [ "$TST_SETUP_CALLER" = "$TST_SETUP" ]; then
	tst_res TWARN "TST_SETUP_CALLER same as TST_SETUP, unset it ($TST_SETUP)"
	unset TST_SETUP_CALLER
fi
if [ "$TST_USAGE_CALLER" = "$TST_USAGE" ]; then
	tst_res TWARN "TST_USAGE_CALLER same as TST_USAGE, unset it ($TST_USAGE)"
	unset TST_USAGE_CALLER
fi

if [ -n "$TST_USE_LEGACY_API" ]; then
	tst_net_read_opts "$@"
fi

# old vs. new API compatibility layer
tst_res_()
{
	[ -z "$TST_USE_LEGACY_API" ] && tst_res $@ || tst_resm $@
}
tst_brk_()
{
	[ -z "$TST_USE_LEGACY_API" ] && tst_brk $@ || tst_brkm $@
}
tst_require_root_()
{
	[ -z "$TST_USE_LEGACY_API" ] && TST_NEEDS_ROOT=1 || tst_require_root
}

init_ltp_netspace()
{
	tst_check_cmds ip
	tst_require_root_

	local pid=

	if [ ! -f /var/run/netns/ltp_ns -a -z "$LTP_NETNS" ]; then
		ROD ip li add name ltp_ns_veth1 type veth peer name ltp_ns_veth2
		pid="$(ROD ns_create net,mnt)"
		mkdir -p /var/run/netns
		ROD ln -s /proc/$pid/ns/net /var/run/netns/ltp_ns
		ROD ns_exec $pid net,mnt mount --make-rprivate /sys
		ROD ns_exec $pid net,mnt mount -t sysfs none /sys
		ROD ns_ifmove ltp_ns_veth1 $pid
		ROD ns_exec $pid net,mnt ip li set lo up
	elif [ -n "$LTP_NETNS" ]; then
		tst_res_ TINFO "using not default LTP netns: '$LTP_NETNS'"
	fi

	LHOST_IFACES="${LHOST_IFACES:-ltp_ns_veth2}"
	RHOST_IFACES="${RHOST_IFACES:-ltp_ns_veth1}"

	export TST_INIT_NETNS="no"

	pid="$(echo $(readlink /var/run/netns/ltp_ns) | cut -f3 -d'/')"
	export LTP_NETNS="${LTP_NETNS:-ns_exec $pid net,mnt}"

	tst_restore_ipaddr
	tst_restore_ipaddr rhost
}

# Run command on remote host.
# Options:
# -b run in background
# -B run in background and save output to $TST_TMPDIR/bg.cmd
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
	local bg=

	OPTIND=0

	while getopts :bBsc:u: opt; do
		case "$opt" in
		b|B) [ "$TST_USE_NETNS" ] && pre_cmd= || pre_cmd="nohup"
		   [ "$opt" = b ] && bg="/dev/null" || bg="$TST_TMPDIR/bg.cmd"
		   post_cmd=" > $bg 2>&1 &"
		   out="1> /dev/null"
		;;
		s) safe=1 ;;
		c) cmd="$OPTARG" ;;
		u) user="$OPTARG" ;;
		*) tst_brk_ TBROK "tst_rhost_run: unknown option: $OPTARG" ;;
		esac
	done

	OPTIND=0

	if [ -z "$cmd" ]; then
		[ "$safe" -eq 1 ] && \
			tst_brk_ TBROK "tst_rhost_run: command not defined"
		tst_res_ TWARN "tst_rhost_run: command not defined"
		return 1
	fi

	local output=
	local ret=0
	if [ -n "${TST_USE_SSH:-}" ]; then
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
			tst_brk_ TBROK "'$cmd' failed on '$RHOST': '$output'"
	fi

	[ -z "$out" -a -n "$output" ] && echo "$output"

	return $ret
}

EXPECT_RHOST_PASS()
{
	tst_rhost_run -c "$*" > /dev/null
	if [ $? -eq 0 ]; then
		tst_res_ TPASS "$* passed as expected"
	else
		tst_res_ TFAIL "$* failed unexpectedly"
	fi
}

EXPECT_RHOST_FAIL()
{
	tst_rhost_run -c "$* 2> /dev/null"
	if [ $? -ne 0 ]; then
		tst_res_ TPASS "$* failed as expected"
	else
		tst_res_ TFAIL "$* passed unexpectedly"
	fi
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
	tst_check_cmds awk

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
	tst_check_cmds awk

	local type="${1:-lhost}"
	local link_num="${2:-0}"
	link_num="$(( $link_num + 1 ))"
	echo "$(tst_get_ifaces $type)" | awk '{ print $'"$link_num"' }'
}

# Get IP address
# tst_ipaddr [TYPE]
# TYPE: { lhost | rhost }; Default value is 'lhost'.
tst_ipaddr()
{
	local type="${1:-lhost}"
	if [ "$TST_IPV6" ]; then
		[ "$type" = "lhost" ] && echo "$IPV6_LHOST" || echo "$IPV6_RHOST"
	else
		[ "$type" = "lhost" ] && echo "$IPV4_LHOST" || echo "$IPV4_RHOST"
	fi
}

# Get IP address of unused network, specified either by type and counter
# or by net and host.
# tst_ipaddr_un [-cCOUNTER] [TYPE]
# tst_ipaddr_un NET_ID [HOST_ID]
# TYPE: { lhost | rhost }; Default value is 'lhost'.
# COUNTER: Integer value for counting HOST_ID and NET_ID. Default is 1.
# NET_ID: Integer or hex value of net. For IPv4 is 3rd octet, for IPv6
# is 3rd hextet.
# HOST_ID: Integer or hex value of host. For IPv4 is 4th octet, for
# IPv6 is the last hextet. Default value is 0.
tst_ipaddr_un()
{
	local counter host_id net_id max_host_id max_net_id tmp type
	local OPTIND

	while getopts "c:" opt; do
		case $opt in
			c) counter="$OPTARG";;
		esac
	done
	shift $(($OPTIND - 1))

	[ "$TST_IPV6" ] && max_net_id=65535 || max_net_id=255

	if [ $# -eq 0 -o "$1" = "lhost" -o "$1" = "rhost" ]; then
		[ -z "$counter" ] && counter=1
		[ $counter -lt 1 ] && counter=1
		type="${1:-lhost}"
		max_host_id=$((max_net_id - 1))
		tmp=$((counter * 2))
		[ "$type" = "rhost" ] && tmp=$((tmp - 1))

		host_id=$((tmp % max_host_id))
		net_id=$((tmp / max_host_id))

		if [ $host_id -eq 0 ]; then
			host_id=$max_host_id
			net_id=$((net_id - 1))
		fi
	else
		net_id="$1"
		host_id="${2:-0}"
		if [ "$TST_IPV6" ]; then
			net_id=$(printf %d $net_id)
			host_id=$(printf %d $host_id)
		fi
		[ $net_id -lt 0 ] && net_id=0
		[ $host_id -lt 0 ] && host_id=1
	fi

	net_id=$((net_id % max_net_id))
	host_id=$((host_id % max_net_id))

	if [ -z "$TST_IPV6" ]; then
		echo "${IPV4_NET16_UNUSED}.${net_id}.${host_id}"
		return
	fi

	[ $host_id -gt 0 ] && host_id="$(printf %x $host_id)" || host_id=
	[ $net_id -gt 0 ] && net_id="$(printf %x $net_id)" || net_id=
	[ "$net_id" ] && net_id=":$net_id"
	echo "${IPV6_NET32_UNUSED}${net_id}::${host_id}"
}

# tst_init_iface [TYPE] [LINK]
# TYPE: { lhost | rhost }; Default value is 'lhost'.
# LINK: link number starting from 0. Default value is '0'.
tst_init_iface()
{
	local type="${1:-lhost}"
	local link_num="${2:-0}"
	local iface="$(tst_iface $type $link_num)"
	tst_res_ TINFO "initialize '$type' '$iface' interface"

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
	local mask dad

	if [ "$TST_IPV6" ]; then
		dad="nodad"
		[ "$type" = "lhost" ] && mask=$IPV6_LPREFIX || mask=$IPV6_RPREFIX
	else
		[ "$type" = "lhost" ] && mask=$IPV4_LPREFIX || mask=$IPV4_RPREFIX
	fi

	local iface=$(tst_iface $type $link_num)

	if [ $type = "lhost" ]; then
		tst_res_ TINFO "set local addr $(tst_ipaddr)/$mask"
		ip addr add $(tst_ipaddr)/$mask dev $iface $dad
		return $?
	fi

	tst_res_ TINFO "set remote addr $(tst_ipaddr rhost)/$mask"
	tst_rhost_run -c "ip addr add $(tst_ipaddr rhost)/$mask dev $iface $dad"
}

# tst_restore_ipaddr [TYPE] [LINK]
# Restore default ip addresses defined in network.sh
# TYPE: { lhost | rhost }; Default value is 'lhost'.
# LINK: link number starting from 0. Default value is '0'.
tst_restore_ipaddr()
{
	tst_check_cmds ip
	tst_require_root_

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
			tst_res_ TINFO "wait for IPv6 DAD completion $((i / 10))/5 sec"

		tst_sleep 100ms
	done
}

tst_dump_rhost_cmd()
{
	tst_rhost_run -c "cat $TST_TMPDIR/netstress.log"
}

# Run network load test, see 'netstress -h' for option description
tst_netload()
{
	local rfile="tst_netload.res"
	local expect_res="pass"
	local ret=0
	local type="tcp"
	local hostopt=
	local setup_srchost=0
	# common options for client and server
	local cs_opts=

	local c_num="$TST_NETLOAD_CLN_NUMBER"
	local c_requests="$TST_NETLOAD_CLN_REQUESTS"
	local c_opts=

	# number of server replies after which TCP connection is closed
	local s_replies="${TST_NETLOAD_MAX_SRV_REPLIES:-500000}"
	local s_opts=

	if [ ! "$TST_NEEDS_TMPDIR" = 1 ]; then
		tst_brk_ TBROK "Using tst_netload requires setting TST_NEEDS_TMPDIR=1"
	fi

	OPTIND=0
	while getopts :a:H:d:n:N:r:R:S:b:t:T:fFe:m:A: opt; do
		case "$opt" in
		a) c_num="$OPTARG" ;;
		H) c_opts="${c_opts}-H $OPTARG "
		   hostopt="$OPTARG" ;;
		d) rfile="$OPTARG" ;;
		n) c_opts="${c_opts}-n $OPTARG " ;;
		N) c_opts="${c_opts}-N $OPTARG " ;;
		r) c_requests="$OPTARG" ;;
		A) c_opts="${c_opts}-A $OPTARG " ;;
		R) s_replies="$OPTARG" ;;
		S) c_opts="${c_opts}-S $OPTARG "
		   setup_srchost=1 ;;
		b) cs_opts="${cs_opts}-b $OPTARG " ;;
		t) cs_opts="${cs_opts}-t $OPTARG " ;;
		T) cs_opts="${cs_opts}-T $OPTARG "
		   type="$OPTARG" ;;
		m) cs_opts="${cs_opts}-m $OPTARG " ;;
		f) cs_opts="${cs_opts}-f " ;;
		F) cs_opts="${cs_opts}-F " ;;
		e) expect_res="$OPTARG" ;;
		*) tst_brk_ TBROK "tst_netload: unknown option: $OPTARG" ;;
		esac
	done
	OPTIND=0

	[ "$setup_srchost" = 1 ] && s_opts="${s_opts}-S $hostopt "

	local expect_ret=0
	[ "$expect_res" != "pass" ] && expect_ret=1

	tst_rhost_run -c "pkill -9 netstress\$"
	s_opts="${cs_opts}${s_opts}-R $s_replies -B $TST_TMPDIR"
	tst_res_ TINFO "run server 'netstress $s_opts'"
	tst_rhost_run -c "netstress $s_opts" > tst_netload.log 2>&1
	if [ $? -ne 0 ]; then
		cat tst_netload.log
		local ttype="TFAIL"
		grep -e 'CONF:' tst_netload.log && ttype="TCONF"
		tst_brk_ $ttype "server failed"
	fi

	local port=$(tst_rhost_run -s -c "cat $TST_TMPDIR/netstress_port")
	c_opts="${cs_opts}${c_opts}-a $c_num -r $c_requests -d $rfile -g $port"

	tst_res_ TINFO "run client 'netstress -l $c_opts'"
	netstress -l $c_opts > tst_netload.log 2>&1 || ret=1
	tst_rhost_run -c "pkill -9 netstress\$"

	if [ "$expect_ret" -ne "$ret" ]; then
		tst_dump_rhost_cmd
		cat tst_netload.log
		tst_brk_ TFAIL "expected '$expect_res' but ret: '$ret'"
	fi

	if [ "$ret" -eq 0 ]; then
		if [ ! -f $rfile ]; then
			tst_dump_rhost_cmd
			cat tst_netload.log
			tst_brk_ TFAIL "can't read $rfile"
		fi
		tst_res_ TPASS "netstress passed, time spent '$(cat $rfile)' ms"
	else
		tst_res_ TPASS "netstress failed as expected"
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
	local msg="tst_ping IPv${TST_IPV6:-4} iface $src_iface, msg_size"
	local cmd="ping$TST_IPV6"
	local ret=0

	tst_check_cmds $cmd

	# ping cmd use 56 as default message size
	for size in ${msg_sizes:-"56"}; do
		$cmd -I $src_iface -c $PING_MAX $dst_addr \
			-s $size -i 0 > /dev/null 2>&1
		ret=$?
		if [ $ret -eq 0 ]; then
			tst_res_ TPASS "$msg $size: pass"
		else
			tst_res_ TFAIL "$msg $size: fail"
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
			tst_res_ TPASS "'ns-icmpv${ver}_sender -s $size $opts' pass"
		else
			tst_res_ TFAIL "'ns-icmpv${ver}_sender -s $size $opts' fail"
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
		ROD sysctl -q -w $name=$value
	else
		sysctl -q -w $name=$value
	fi

	tst_rhost_run $safe -c "sysctl -q -w $add_opt $name=$value"
}

tst_cleanup_rhost()
{
	tst_rhost_run -c "rm -rf $TST_TMPDIR"
}

# Management Link
[ -z "$RHOST" ] && TST_USE_NETNS="yes"
export RHOST="$RHOST"
export PASSWD="${PASSWD:-}"
# Don't use it in new tests, use tst_rhost_run() from tst_net.sh instead.
export LTP_RSH="${LTP_RSH:-rsh -n}"

# Test Links
# IPV{4,6}_{L,R}HOST can be set with or without prefix (e.g. IP or IP/prefix),
# but if you use IP/prefix form, /prefix will be removed by tst_net_vars.
IPV4_LHOST="${IPV4_LHOST:-10.0.0.2/24}"
IPV4_RHOST="${IPV4_RHOST:-10.0.0.1/24}"
IPV6_LHOST="${IPV6_LHOST:-fd00:1:1:1::2/64}"
IPV6_RHOST="${IPV6_RHOST:-fd00:1:1:1::1/64}"

# tst_net_ip_prefix
# Strip prefix from IP address and save both If no prefix found sets
# default prefix.
#
# tst_net_iface_prefix reads prefix and interface from rtnetlink.
# If nothing found sets default prefix value.
#
# tst_net_vars exports environment variables related to test links and
# networks that aren't reachable through the test links.
#
# For full list of exported environment variables see:
# tst_net_ip_prefix -h
# tst_net_iface_prefix -h
# tst_net_vars -h
if [ -z "$TST_PARSE_VARIABLES" ]; then
	eval $(tst_net_ip_prefix $IPV4_LHOST || echo "exit $?")
	eval $(tst_net_ip_prefix -r $IPV4_RHOST || echo "exit $?")
	eval $(tst_net_ip_prefix $IPV6_LHOST || echo "exit $?")
	eval $(tst_net_ip_prefix -r $IPV6_RHOST || echo "exit $?")
fi

[ -n "$TST_USE_NETNS" -a "$TST_INIT_NETNS" != "no" ] && init_ltp_netspace

if [ -z "$TST_PARSE_VARIABLES" ]; then
	eval $(tst_net_iface_prefix $IPV4_LHOST || echo "exit $?")
	eval $(tst_rhost_run -c 'tst_net_iface_prefix -r '$IPV4_RHOST \
		|| echo "exit $?")
	eval $(tst_net_iface_prefix $IPV6_LHOST || echo "exit $?")
	eval $(tst_rhost_run -c 'tst_net_iface_prefix -r '$IPV6_RHOST \
		|| echo "exit $?")

	eval $(tst_net_vars $IPV4_LHOST/$IPV4_LPREFIX \
		$IPV4_RHOST/$IPV4_RPREFIX || echo "exit $?")
	eval $(tst_net_vars $IPV6_LHOST/$IPV6_LPREFIX \
		$IPV6_RHOST/$IPV6_RPREFIX || echo "exit $?")

	tst_res_ TINFO "Network config (local -- remote):"
	tst_res_ TINFO "$LHOST_IFACES -- $RHOST_IFACES"
	tst_res_ TINFO "$IPV4_LHOST/$IPV4_LPREFIX -- $IPV4_RHOST/$IPV4_RPREFIX"
	tst_res_ TINFO "$IPV6_LHOST/$IPV6_LPREFIX -- $IPV6_RHOST/$IPV6_RPREFIX"
	export TST_PARSE_VARIABLES="yes"
fi

export TST_NETLOAD_CLN_REQUESTS="${TST_NETLOAD_CLN_REQUESTS:-10000}"
export TST_NETLOAD_CLN_NUMBER="${TST_NETLOAD_CLN_NUMBER:-2}"
export HTTP_DOWNLOAD_DIR="${HTTP_DOWNLOAD_DIR:-/var/www/html}"
export FTP_DOWNLOAD_DIR="${FTP_DOWNLOAD_DIR:-/var/ftp}"
export FTP_UPLOAD_DIR="${FTP_UPLOAD_DIR:-/var/ftp/pub}"
export FTP_UPLOAD_URLDIR="${FTP_UPLOAD_URLDIR:-pub}"

# network/stress tests require additional parameters
export NS_DURATION="${NS_DURATION:-10}"
export NS_TIMES="${NS_TIMES:-10}"
export CONNECTION_TOTAL="${CONNECTION_TOTAL:-10}"
export IP_TOTAL="${IP_TOTAL:-100}"
export IP_TOTAL_FOR_TCPIP="${IP_TOTAL_FOR_TCPIP:-100}"
export ROUTE_TOTAL="${ROUTE_TOTAL:-100}"
export MTU_CHANGE_TIMES="${MTU_CHANGE_TIMES:-100}"
export IF_UPDOWN_TIMES="${IF_UPDOWN_TIMES:-100}"
export DOWNLOAD_BIGFILESIZE="${DOWNLOAD_BIGFILESIZE:-2147483647}"
export DOWNLOAD_REGFILESIZE="${DOWNLOAD_REGFILESIZE:-1048576}"
export UPLOAD_BIGFILESIZE="${UPLOAD_BIGFILESIZE:-2147483647}"
export UPLOAD_REGFILESIZE="${UPLOAD_REGFILESIZE:-1024}"
export MCASTNUM_NORMAL="${MCASTNUM_NORMAL:-20}"
export MCASTNUM_HEAVY="${MCASTNUM_HEAVY:-4000}"

# Warning: make sure to set valid interface names and IP addresses below.
# Set names for test interfaces, e.g. "eth0 eth1"
# This is fallback for LHOST_IFACES in case tst_net_vars finds nothing or we
# want to use more ifaces.
export LHOST_IFACES="${LHOST_IFACES:-eth0}"
export RHOST_IFACES="${RHOST_IFACES:-eth0}"

# Set corresponding HW addresses, e.g. "00:00:00:00:00:01 00:00:00:00:00:02"
export LHOST_HWADDRS="${LHOST_HWADDRS:-$(tst_get_hwaddrs lhost)}"
export RHOST_HWADDRS="${RHOST_HWADDRS:-$(tst_get_hwaddrs rhost)}"

# More information about network parameters can be found
# in the following document: testcases/network/stress/README

if [ -n "$TST_USE_LEGACY_API" ]; then
	tst_net_remote_tmpdir
fi
