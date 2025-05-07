#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2014-2017 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) 2016-2024 Petr Vorel <pvorel@suse.cz>
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
# Blank for IPv4, '-6' for IPv6 test.
TST_IPV6_FLAG=${TST_IPV6_FLAG:-}

tst_net_parse_args()
{
	case $1 in
	6) TST_IPV6=6 TST_IPVER=6 TST_IPV6_FLAG="-6";;
	*) [ "$TST_PARSE_ARGS_CALLER" ] && $TST_PARSE_ARGS_CALLER "$1" "$2";;
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
	[ "$TST_IPV6" ] && tst_net_require_ipv6

	tst_net_remote_tmpdir
	[ -n "$TST_SETUP_CALLER" ] && $TST_SETUP_CALLER

	if [ -z "$NS_ICMP_SENDER_DATA_MAXSIZE" ]; then
		if [ "$TST_IPV6" ]; then
			NS_ICMP_SENDER_DATA_MAXSIZE="$NS_ICMPV6_SENDER_DATA_MAXSIZE"
		else
			NS_ICMP_SENDER_DATA_MAXSIZE="$NS_ICMPV4_SENDER_DATA_MAXSIZE"
		fi
	fi
}

# old vs. new API compatibility layer
tst_res_()
{
	[ -z "$TST_USE_LEGACY_API" ] && tst_res $@ || tst_resm $@
}

tst_brk_()
{
	[ -z "$TST_USE_LEGACY_API" ] && tst_brk $@ || tst_brkm $@
}

# Detect IPv6 disabled via 1) CONFIG_IPV6=n or 2) ipv6.disable=1 kernel cmdline
# parameter or 3) sysctl net.ipv6.conf.all.disable_ipv6=1 (disables IPv6 on all
# interfaces (including both already created and later created).
# $TST_NET_IPV6_ENABLED: 1 on IPv6 enabled, 0 on IPv6 disabled.
tst_net_detect_ipv6()
{
	local type="${1:-lhost}"
	local cmd='[ -f /proc/net/if_inet6 ]'
	local disabled iface ret

	if [ "$type" = "lhost" ]; then
		$cmd
	else
		tst_rhost_run -c "$cmd"
	fi

	if [ $? -ne 0 ]; then
		TST_NET_IPV6_ENABLED=0
		tst_res_ TINFO "IPv6 disabled on $type via kernel command line or not compiled in"
		return
	fi

	cmd='cat /proc/sys/net/ipv6/conf/all/disable_ipv6'
	if [ "$type" = "lhost" ]; then
		disabled=$($cmd)
	else
		disabled=$(tst_rhost_run -c "$cmd")
	fi
	if [ $disabled = 1 ]; then
		tst_res_ TINFO "IPv6 disabled on $type net.ipv6.conf.all.disable_ipv6=1"
		TST_NET_IPV6_ENABLED=0
		return
	fi

	TST_NET_IPV6_ENABLED=1
}

# Detect IPv6 disabled on interface via sysctl
# net.ipv6.conf.$iface.disable_ipv6=1.
# $TST_NET_IPV6_ENABLED: 1 on IPv6 enabled, 0 on IPv6 disabled.
# return: 0 on IPv6 enabled, 1 on IPv6 disabled.
tst_net_detect_ipv6_iface()
{
	[ "$TST_NET_IPV6_ENABLED" = 1 ] || return 1

	local iface="$1"
	local type="${2:-lhost}"
	local check="cat /proc/sys/net/ipv6/conf/$iface/disable_ipv6"
	local disabled

	if [ "$type" = "lhost" ]; then
		disabled=$($check)
	else
		disabled=$(tst_rhost_run -c "$check")
	fi
	if [ $disabled = 1 ]; then
		tst_res_ TINFO "IPv6 disabled on $type on $iface"
		TST_NET_IPV6_ENABLED=0
		return 1
	fi

	return 0
}

# Detect IPv6 disabled on used interfaces.
tst_net_check_ifaces_ipv6()
{
	local iface

	for iface in $(tst_get_ifaces); do
		tst_net_detect_ipv6_iface $iface || return
	done

	for iface in $(tst_get_ifaces rhost); do
		tst_net_detect_ipv6_iface $iface rhost || return
	done
}

tst_net_require_ipv6()
{
	[ "$TST_NET_IPV6_ENABLED" = 1 ] || tst_brk_ TCONF "IPv6 disabled"
}

init_ltp_netspace()
{
	local pid

	if [ ! -f /var/run/netns/ltp_ns -a -z "$LTP_NETNS" ]; then
		tst_require_cmds ip tst_ns_create tst_ns_exec tst_ns_ifmove
		tst_require_root

		if [ -z "$TST_USE_LEGACY_API" ]; then
			tst_require_drivers veth
		fi
		ROD ip link add name ltp_ns_veth1 type veth peer name ltp_ns_veth2
		pid="$(ROD tst_ns_create net,mnt)"
		mkdir -p /var/run/netns
		ROD ln -s /proc/$pid/ns/net /var/run/netns/ltp_ns
		ROD tst_ns_exec $pid net,mnt mount --make-rprivate /sys
		ROD tst_ns_exec $pid net,mnt mount -t sysfs none /sys
		ROD tst_ns_ifmove ltp_ns_veth1 $pid
		ROD tst_ns_exec $pid net,mnt ip link set lo up
	elif [ -n "$LTP_NETNS" ]; then
		tst_res_ TINFO "using not default LTP netns: '$LTP_NETNS'"
	fi

	LHOST_IFACES="${LHOST_IFACES:-ltp_ns_veth2}"
	RHOST_IFACES="${RHOST_IFACES:-ltp_ns_veth1}"

	pid="$(echo $(readlink /var/run/netns/ltp_ns) | cut -f3 -d'/')"
	export LTP_NETNS="${LTP_NETNS:-tst_ns_exec $pid net,mnt}"

	tst_restore_ipaddr
	tst_restore_ipaddr rhost
}

# return 0: use ssh, 1: use netns
tst_net_use_netns()
{
	[ -n "$TST_USE_NETNS" ]
}

# Run command on remote host.
# tst_rhost_run -c CMD [-b] [-s] [-u USER]
# Options:
# -b run in background
# -c CMD specify command to run (this must be binary, not shell builtin/function)
# -s safe option, if something goes wrong, will exit with TBROK
# -u USER for ssh (default root)
# RETURN: 0 on success, 1 on failure
# TST_NET_RHOST_RUN_DEBUG=1 enables debugging
tst_rhost_run()
{
	local post_cmd=' || echo RTERR'
	local user="root"
	local ret=0
	local cmd out output pre_cmd rcmd sh_cmd safe use

	local OPTIND
	while getopts :bc:su: opt; do
		case "$opt" in
		b) [ "${TST_USE_NETNS:-}" ] && pre_cmd= || pre_cmd="nohup"
		   post_cmd=" > /dev/null 2>&1 &"
		   out="1> /dev/null"
		;;
		c) cmd="$OPTARG" ;;
		s) safe=1 ;;
		u) user="$OPTARG" ;;
		*) tst_brk_ TBROK "tst_rhost_run: unknown option: $OPTARG" ;;
		esac
	done

	if [ -z "$cmd" ]; then
		[ "$safe" ] && \
			tst_brk_ TBROK "tst_rhost_run: command not defined"
		tst_res_ TWARN "tst_rhost_run: command not defined"
		return 1
	fi

	sh_cmd="$pre_cmd $cmd $post_cmd"

	if [ -n "${TST_USE_NETNS:-}" ]; then
		use="NETNS"
		rcmd="$LTP_NETNS sh -c"
	else
		tst_require_cmds ssh
		use="SSH"
		rcmd="ssh -nq $user@$RHOST"
	fi

	if [ "$TST_NET_RHOST_RUN_DEBUG" = 1 ]; then
		tst_res_ TINFO "tst_rhost_run: cmd: $cmd"
		tst_res_ TINFO "$use: $rcmd \"$sh_cmd\" $out 2>&1"
	fi

	output=$($rcmd "$sh_cmd" $out 2>&1 || echo 'RTERR')

	echo "$output" | grep -q 'RTERR$' && ret=1
	if [ $ret -eq 1 ]; then
		output=$(echo "$output" | sed 's/RTERR//')
		[ "$safe" ] && \
			tst_brk_ TBROK "'$cmd' failed on $use: '$output'"
	fi

	[ -z "$out" -a -n "$output" ] && echo "$output"

	return $ret
}

# Run command on both lhost and rhost.
# tst_net_run [-s] [-l LPARAM] [-r RPARAM] [ -q ] CMD [ARG [ARG2]]
# Options:
# -l LPARAM: parameter passed to CMD in lhost
# -r RPARAM: parameter passed to CMD in rhost
# -q: quiet mode (suppress failure warnings)
# -i: ignore errors on rhost
# CMD: command to run (this must be binary, not shell builtin/function due
# tst_rhost_run() limitation)
# RETURN: 0 on success, 1 on missing CMD or exit code on lhost or rhost
tst_net_run()
{
	local cmd
	local lparams
	local rparams
	local lsafe
	local rsafe
	local lret
	local rret
	local quiet

	local OPTIND
	while getopts l:qr:si opt; do
		case "$opt" in
		l) lparams="$OPTARG" ;;
		q) quiet=1 ;;
		r) rparams="$OPTARG" ;;
		s) lsafe="ROD"; rsafe="-s" ;;
		i) rsafe="" ;;
		*) tst_brk_ TBROK "tst_net_run: unknown option: $OPTARG" ;;
		esac
	done
	shift $((OPTIND - 1))
	cmd="$1"
	shift

	if [ -z "$cmd" ]; then
		[ -n "$lsafe" ] && \
			tst_brk_ TBROK "tst_net_run: command not defined"
		tst_res_ TWARN "tst_net_run: command not defined"
		return 1
	fi

	$lsafe $cmd $lparams $@
	lret=$?
	tst_rhost_run $rsafe -c "$cmd $rparams $@"
	rret=$?

	if [ -z "$quiet" ]; then
		[ $lret -ne 0 ] && tst_res_ TWARN "tst_net_run: lhost command failed: $lret"
		[ $rret -ne 0 ] && tst_res_ TWARN "tst_net_run: rhost command failed: $rret"
	fi

	[ $lret -ne 0 ] && return $lret
	return $rret
}

EXPECT_RHOST_PASS()
{
	local log="$TMPDIR/log.$$"

	tst_rhost_run -c "$*" > $log
	if [ $? -eq 0 ]; then
		tst_res_ TPASS "$* passed as expected"
	else
		tst_res_ TFAIL "$* failed unexpectedly"
		cat $log
	fi

	rm -f $log
}

EXPECT_RHOST_FAIL()
{
	local log="$TMPDIR/log.$$"

	tst_rhost_run -c "$*" > $log
	if [ $? -ne 0 ]; then
		tst_res_ TPASS "$* failed as expected"
	else
		tst_res_ TFAIL "$* passed unexpectedly"
		cat $log
	fi

	rm -f $log
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

# Get count of test interfaces for local/remote host.
tst_get_ifaces_cnt()
{
	tst_require_cmds awk
	local type="${1:-lhost}"
	echo "$(tst_get_ifaces $type)" | awk '{print NF}'
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
	tst_require_cmds awk

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
	tst_require_cmds awk

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

# Get IP address of unused network, specified either counter and type
# or by net and host.
# counter mode:
# tst_ipaddr_un [-h MIN,MAX] [-n MIN,MAX] [-p] [-c COUNTER] [TYPE]
# net & host mode:
# tst_ipaddr_un [-h MIN,MAX] [-n MIN,MAX] [-p] NET_ID [HOST_ID]
#
# TYPE: { lhost | rhost } (default: 'lhost')
# NET_ID: integer or hex value of net (IPv4: 3rd octet <0,255>, IPv6: 3rd
# hextet <0,65535>)
# HOST_ID: integer or hex value of host (IPv4: 4th octet <0,255>, IPv6: the
# last hextet <0, 65535>, default: 0)
#
# OPTIONS
# -c COUNTER: integer value for counting HOST_ID and NET_ID (default: 1)
#
# -h: specify *host* address range (HOST_ID)
# -h MIN,MAX or -h MIN or -h ,MAX
#
# -n: specify *network* address range (NET_ID)
# -n MIN,MAX or -n MIN or -n ,MAX
#
# -p: print also prefix
tst_ipaddr_un()
{
	local default_max=255
	[ "$TST_IPV6" ] && default_max=65535
	local max_net_id=$default_max
	local min_net_id=0

	local counter host_id host_range is_counter max_host_id min_host_id net_id prefix= tmp type

	local OPTIND
	while getopts "c:h:n:p" opt; do
		case $opt in
			c) counter="$OPTARG";;
			h)
				if echo $OPTARG | grep -q ','; then # 'min,max' or 'min,' or ',max'
					min_host_id="$(echo $OPTARG | cut -d, -f1)"
					max_host_id="$(echo $OPTARG | cut -d, -f2)"
				else # min
					min_host_id="$OPTARG"
				fi
				;;
			n)
				if echo $OPTARG | grep -q ','; then # 'min,max' or 'min,' or ',max'
					min_net_id="$(echo $OPTARG | cut -d, -f1)"
					max_net_id="$(echo $OPTARG | cut -d, -f2)"
				else # min
					min_net_id="$OPTARG"
				fi
				;;
			m)
				! tst_is_int "$OPTARG" || [ "$OPTARG" -lt 0 ]|| [ "$OPTARG" -gt $max_net_id ] && \
					tst_brk_ TBROK "tst_ipaddr_un: -m must be integer <0,$max_net_id> ($OPTARG)"
				[ "$OPTARG" -gt $max_net_id ] && \
					tst_brk_ TBROK "tst_ipaddr_un: -m cannot be higher than $max_net_id ($OPTARG)"
				max_host_id="$OPTARG"
				;;
			p) [ "$TST_IPV6" ] && prefix="/64" || prefix="/24";;
		esac
	done
	shift $(($OPTIND - 1))
	[ $# -eq 0 -o "$1" = "lhost" -o "$1" = "rhost" ] && is_counter=1

	if [ -z "$min_host_id" ]; then
		[ "$is_counter" ] && min_host_id=1 || min_host_id=0
	fi
	if [ -z "$max_host_id" ]; then
		[ "$is_counter" ] && max_host_id=$((default_max - 1)) || max_host_id=$default_max
	fi

	! tst_is_int "$min_host_id" || ! tst_is_int "$max_host_id" || \
		[ $min_host_id -lt 0 -o $min_host_id -gt $default_max ] || \
		[ $max_host_id -lt 0 -o $max_host_id -gt $default_max ] && \
		tst_brk_ TBROK "tst_ipaddr_un: HOST_ID must be int in range <0,$default_max> ($min_host_id,$max_host_id)"
	! tst_is_int "$min_net_id" || ! tst_is_int "$max_net_id" || \
		[ $min_net_id -lt 0 -o $min_net_id -gt $default_max ] || \
		[ $max_net_id -lt 0 -o $max_net_id -gt $default_max ] && \
		tst_brk_ TBROK "tst_ipaddr_un: NET_ID must be int in range <0,$default_max> ($min_net_id,$max_net_id)"

	[ $min_host_id -gt $max_host_id ] && \
		tst_brk_ TBROK "tst_ipaddr_un: max HOST_ID ($max_host_id) must be >= min HOST_ID ($min_host_id)"
	[ $min_net_id -gt $max_net_id ] && \
		tst_brk_ TBROK "tst_ipaddr_un: max NET_ID ($max_net_id) must be >= min NET_ID ($min_net_id)"

	# counter
	host_range=$((max_host_id - min_host_id + 1))
	if [ "$is_counter" ]; then
		[ -z "$counter" ] && counter=1
		[ $counter -lt 1 ] && counter=1
		type="${1:-lhost}"
		tmp=$((counter * 2))
		[ "$type" = "rhost" ] && tmp=$((tmp - 1))
		net_id=$(((tmp - 1) / host_range))
		host_id=$((tmp - net_id * host_range + min_host_id - 1))
	else # net_id & host_id
		net_id="$1"
		host_id="${2:-0}"
		if [ "$TST_IPV6" ]; then
			net_id=$(printf %d $net_id)
			host_id=$(printf %d $host_id)
		fi
		host_id=$((host_id % host_range + min_host_id))
	fi

	net_id=$((net_id % (max_net_id - min_net_id + 1) + min_net_id))

	if [ -z "$TST_IPV6" ]; then
		echo "${IPV4_NET16_UNUSED}.${net_id}.${host_id}${prefix}"
		return
	fi

	[ $host_id -gt 0 ] && host_id="$(printf %x $host_id)" || host_id=
	[ $net_id -gt 0 ] && net_id="$(printf %x $net_id)" || net_id=
	[ "$net_id" ] && net_id=":$net_id"
	echo "${IPV6_NET32_UNUSED}${net_id}::${host_id}${prefix}"
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
	tst_net_detect_ipv6_iface $iface $type

	if [ "$type" = "lhost" ]; then
		if ip xfrm state 1>/dev/null 2>&1; then
			ip xfrm policy flush || return $?
			ip xfrm state flush || return $?
		fi
		ip link set $iface down || return $?
		ip route flush dev $iface || return $?
		ip addr flush dev $iface || return $?
		if [ "$TST_NET_IPV6_ENABLED" = 1 ]; then
			sysctl -qw net.ipv6.conf.$iface.accept_dad=0 || return $?
		fi
		ip link set $iface up
		return $?
	fi

	if tst_rhost_run -c "ip xfrm state 1>/dev/null 2>&1"; then
		tst_rhost_run -c "ip xfrm policy flush" || return $?
		tst_rhost_run -c "ip xfrm state flush" || return $?
	fi
	tst_rhost_run -c "ip link set $iface down" || return $?
	tst_rhost_run -c "ip route flush dev $iface" || return $?
	tst_rhost_run -c "ip addr flush dev $iface" || return $?
	if [ "$TST_NET_IPV6_ENABLED" = 1 ]; then
		tst_rhost_run -c "sysctl -qw net.ipv6.conf.$iface.accept_dad=0" || return $?
	fi
	tst_rhost_run -c "ip link set $iface up"
}

# tst_add_ipaddr [TYPE] [LINK] [-a IP] [-d] [-q] [-s]
# Options:
# TYPE: { lhost | rhost }, default value is 'lhost'
# LINK: link number starting from 0, default value is '0'
# -a IP: IP address to be added, default value is
# $(tst_ipaddr)/$IPV{4,6}_{L,R}PREFIX
# -d: delete address instead of adding
# -q: quiet mode (don't print info)
# -s: safe option, if something goes wrong, will exit with TBROK
tst_add_ipaddr()
{
	local action="add"
	local addr dad lsafe mask quiet rsafe

	local OPTIND
	while getopts a:dqs opt; do
		case "$opt" in
		a) addr="$OPTARG" ;;
		d) action="del" ;;
		q) quiet=1 ;;
		s) lsafe="ROD"; rsafe="-s" ;;
		*) tst_brk_ TBROK "tst_add_ipaddr: unknown option: $OPTARG" ;;
		esac
	done
	shift $((OPTIND - 1))

	local type="${1:-lhost}"
	local link_num="${2:-0}"
	local iface=$(tst_iface $type $link_num)

	tst_net_detect_ipv6_iface $iface $type

	if [ "$TST_IPV6" ]; then
		dad="nodad"
		[ "$type" = "lhost" ] && mask=$IPV6_LPREFIX || mask=$IPV6_RPREFIX
	else
		[ "$type" = "lhost" ] && mask=$IPV4_LPREFIX || mask=$IPV4_RPREFIX
	fi
	[ -n "$addr" ] || addr="$(tst_ipaddr $type)"
	echo $addr | grep -q / || addr="$addr/$mask"

	if [ $type = "lhost" ]; then
		[ "$quiet" ] || tst_res_ TINFO "$action local addr $addr"
		$lsafe ip addr $action $addr dev $iface $dad
		return $?
	fi

	[ "$quiet" ] || tst_res_ TINFO "$action remote addr $addr"
	tst_rhost_run $rsafe -c "ip addr $action $addr dev $iface $dad"
}

# tst_del_ipaddr [ tst_add_ipaddr options ]
# Delete IP address
tst_del_ipaddr()
{
	tst_add_ipaddr -d $@
}

# tst_restore_ipaddr [TYPE] [LINK]
# Restore default ip addresses defined in network.sh
# TYPE: { lhost | rhost }; Default value is 'lhost'.
# LINK: link number starting from 0. Default value is '0'.
tst_restore_ipaddr()
{
	tst_require_cmds ip
	tst_require_root

	local type="${1:-lhost}"
	local link_num="${2:-0}"

	tst_init_iface $type $link_num || return $?

	local ret=0
	local backup_tst_ipv6=$TST_IPV6
	TST_IPV6= tst_add_ipaddr $type $link_num || ret=$?
	if [ "$TST_NET_IPV6_ENABLED" = 1 ]; then
		TST_IPV6=6 tst_add_ipaddr $type $link_num || ret=$?
	fi
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
		ip addr sh $iface_loc | grep -q tentative
		ret=$?

		tst_rhost_run -c "ip addr sh $iface_rmt | grep -q tentative"

		[ $ret -ne 0 -a $? -ne 0 ] && return

		[ $(($i % 10)) -eq 0 ] && \
			tst_res_ TINFO "wait for IPv6 DAD completion $((i / 10))/5 sec"

		tst_sleep 100ms
	done
}

tst_netload_brk()
{
	tst_rhost_run -c "cat $TST_TMPDIR/netstress.log"
	cat tst_netload.log
	tst_brk_ $1 $2
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

	local run_cnt="$TST_NETLOAD_RUN_COUNT"
	local c_num="$TST_NETLOAD_CLN_NUMBER"
	local c_requests="$TST_NETLOAD_CLN_REQUESTS"
	local c_opts=

	# number of server replies after which TCP connection is closed
	local s_replies="${TST_NETLOAD_MAX_SRV_REPLIES:-500000}"
	local s_opts=
	local bind_to_device=1

	if [ ! "$TST_NEEDS_TMPDIR" = 1 ]; then
		tst_brk_ TBROK "Using tst_netload requires setting TST_NEEDS_TMPDIR=1"
	fi

	OPTIND=0
	while getopts :a:c:H:n:N:r:R:S:b:t:T:fFe:m:A:D: opt; do
		case "$opt" in
		a) c_num="$OPTARG" ;;
		H) c_opts="${c_opts}-H $OPTARG "
		   hostopt="$OPTARG" ;;
		c) rfile="$OPTARG" ;;
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
		D) [ "$TST_NETLOAD_BINDTODEVICE" = 1 ] && cs_opts="${cs_opts}-d $OPTARG "
		   bind_to_device=0 ;;
		*) tst_brk_ TBROK "tst_netload: unknown option: $OPTARG" ;;
		esac
	done
	OPTIND=0

	[ "$setup_srchost" = 1 ] && s_opts="${s_opts}-S $hostopt "

	if [ "$bind_to_device" = 1 -a "$TST_NETLOAD_BINDTODEVICE" = 1 ]; then
		c_opts="${c_opts}-d $(tst_iface) "
		s_opts="${s_opts}-d $(tst_iface rhost) "
	fi

	local expect_ret=0
	[ "$expect_res" != "pass" ] && expect_ret=3

	local was_failure=0
	if [ "$run_cnt" -lt 2 ]; then
		run_cnt=1
		was_failure=1
	fi

	s_opts="${cs_opts}${s_opts}-R $s_replies -B $TST_TMPDIR"
	c_opts="${cs_opts}${c_opts}-a $c_num -r $((c_requests / run_cnt)) -c $PWD/$rfile"

	tst_res_ TINFO "run server 'netstress $s_opts'"
	tst_res_ TINFO "run client 'netstress -l $c_opts' $run_cnt times"

	tst_rhost_run -c "pkill -9 netstress\$"
	rm -f tst_netload.log

	local results
	local passed=0

	for i in $(seq 1 $run_cnt); do
		tst_rhost_run -c "netstress $s_opts" > tst_netload.log 2>&1
		if [ $? -ne 0 ]; then
			cat tst_netload.log
			local ttype="TFAIL"
			grep -e 'CONF:' tst_netload.log && ttype="TCONF"
			tst_brk_ $ttype "server failed"
		fi

		local port=$(tst_rhost_run -s -c "cat $TST_TMPDIR/netstress_port")
		netstress -l ${c_opts} -g $port > tst_netload.log 2>&1
		ret=$?
		tst_rhost_run -c "pkill -9 netstress\$"

		if [ "$expect_ret" -ne 0 ]; then
			if [ $((ret & expect_ret)) -ne 0 ]; then
				tst_res_ TPASS "netstress failed as expected"
			else
				tst_res_ TFAIL "expected '$expect_res' but ret: '$ret'"
			fi
			return $ret
		fi

		if [ "$ret" -ne 0 ]; then
			[ $((ret & 32)) -ne 0 ] && \
				tst_netload_brk TCONF "not supported configuration"

			[ $((ret & 3)) -ne 0 -a $was_failure -gt 0 ] && \
				tst_netload_brk TFAIL "expected '$expect_res' but ret: '$ret'"

			tst_res_ TWARN "netstress failed, ret: $ret"
			was_failure=1
			continue
		fi

		[ ! -f $rfile ] && \
			tst_netload_brk TFAIL "can't read $rfile"

		results="$results $(cat $rfile)"
		passed=$((passed + 1))
	done

	if [ "$ret" -ne 0 ]; then
		[ $((ret & 4)) -ne 0 ] && \
			tst_res_ TWARN "netstress has warnings"
		tst_netload_brk TFAIL "expected '$expect_res' but ret: '$ret'"
	fi

	local median=$(tst_get_median $results)
	echo "$median" > $rfile

	tst_res_ TPASS "netstress passed, median time $median ms, data:$results"

	return $ret
}

# Compares results for netload runs.
# tst_netload_compare TIME_BASE TIME THRESHOLD_LOW [THRESHOLD_HI]
# TIME_BASE: time taken to run netstress load test - 100%
# TIME: time that is compared to the base one
# THRESHOD_LOW: lower limit for TFAIL
# THRESHOD_HIGH: upper limit for TWARN
#
# Slow performance can be ignored with setting environment variable
# LTP_NET_FEATURES_IGNORE_PERFORMANCE_FAILURE=1
tst_netload_compare()
{
	local base_time=$1
	local new_time=$2
	local threshold_low=$3
	local threshold_hi=$4
	local ttype='TFAIL'
	local msg res

	if [ -z "$base_time" -o -z "$new_time" -o -z "$threshold_low" ]; then
		tst_brk_ TBROK "tst_netload_compare: invalid argument(s)"
	fi

	res=$(((base_time - new_time) * 100 / base_time))
	msg="performance result is ${res}%"

	if [ "$res" -lt "$threshold_low" ]; then
		if [ "$LTP_NET_FEATURES_IGNORE_PERFORMANCE_FAILURE" = 1 ]; then
			ttype='TINFO';
			tst_res_ TINFO "WARNING: slow performance is not treated as error due LTP_NET_FEATURES_IGNORE_PERFORMANCE_FAILURE=1"
		else
			tst_res_ TINFO "Following slow performance can be ignored with LTP_NET_FEATURES_IGNORE_PERFORMANCE_FAILURE=1"
		fi
		tst_res_ $ttype "$msg < threshold ${threshold_low}%"
		return
	fi

	[ "$threshold_hi" ] && [ "$res" -gt "$threshold_hi" ] && \
		tst_res_ TWARN "$msg > threshold ${threshold_hi}%"

	tst_res_ TPASS "$msg, in range [${threshold_low}:${threshold_hi}]%"
}

tst_ping_opt_unsupported()
{
	ping $@ 2>&1 | grep -qE "(invalid|unrecognized) option"
}

# tst_ping -c COUNT -s MESSAGE_SIZES -p PATTERN -I IFACE -H HOST
# Check icmp connectivity
# IFACE: source interface name or IP address
# HOST: destination IPv4 or IPv6 address
# MESSAGE_SIZES: message size array
tst_ping()
{
	# The max number of ICMP echo request
	local ping_count="${PING_MAX:-500}"
	local flood_opt="-f"
	local pattern_opt
	local msg_sizes
	local src_iface="$(tst_iface)"
	local dst_addr="$(tst_ipaddr rhost)"
	local cmd="ping"
	local ret=0
	local opts

	local OPTIND
	while getopts c:s:p:I:H: opt; do
		case "$opt" in
		c) ping_count="$OPTARG";;
		s) msg_sizes="$OPTARG";;
		p) pattern_opt="-p $OPTARG";;
		I) src_iface="$OPTARG"
		   tst_ping_opt_unsupported -I $OPTARG && \
			   tst_brk_ TCONF "unsupported ping version (ping from inetutils?)"
		   ;;
		H) dst_addr="$OPTARG";;
		*) tst_brk_ TBROK "tst_ping: unknown option: $OPTARG";;
		esac
	done

	echo "$dst_addr" | grep -q ':' && cmd="ping6"
	tst_require_cmds $cmd

	if tst_ping_opt_unsupported $flood_opt; then
		flood_opt="-i 0.01"
		[ "$pattern_opt" ] && pattern_opt="-p aa"

		tst_ping_opt_unsupported -i $pattern_opt && \
			tst_brk_ TCONF "unsupported ping version (old busybox?)"
	fi

	# ping cmd use 56 as default message size
	for size in ${msg_sizes:-"56"}; do
		EXPECT_PASS $cmd -I $src_iface -c $ping_count -s $size \
			$flood_opt $pattern_opt $dst_addr \>/dev/null
		ret=$?
		[ "$ret" -ne 0 ] && break
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
		ns-icmpv${TST_IPVER}_sender -s $size $opts
		ret=$?
		if [ $ret -eq 0 ]; then
			tst_res_ TPASS "'ns-icmpv${TST_IPVER}_sender -s $size $opts' pass"
		else
			tst_res_ TFAIL "'ns-icmpv${TST_IPVER}_sender -s $size $opts' fail"
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

	local rparam=
	[ "$TST_USE_NETNS" = "yes" ] && rparam="-i -r '-e'"

	tst_net_run $safe -q $rparam "sysctl" "-q -w $name=$value"
}

tst_cleanup_rhost()
{
	tst_rhost_run -c "rm -rf $TST_TMPDIR"
}

tst_default_max_pkt()
{
	local mtu="$(cat /sys/class/net/$(tst_iface)/mtu)"

	echo "$((mtu + mtu / 10))"
}

# Setup LTP network.
#
# Used tools:
# * tst_net_ip_prefix
# Strip prefix from IP address and save both If no prefix found sets
# default prefix.
# * tst_net_iface_prefix reads prefix and interface from rtnetlink.
# If nothing found sets default prefix value.
# * tst_net_vars exports environment variables related to test links and
# networks that aren't reachable through the test links.
#
# For full list of exported environment variables see:
# tst_net_ip_prefix -h
# tst_net_iface_prefix -h
# tst_net_vars -h
tst_net_setup_network()
{
	tst_require_cmds tst_net_iface_prefix tst_net_ip_prefix tst_net_vars

	eval $(tst_net_ip_prefix $IPV4_LHOST || echo "exit $?")
	eval $(tst_net_ip_prefix -r $IPV4_RHOST || echo "exit $?")

	[ "$TST_NET_IPV6_ENABLED" = 1 ] && tst_net_detect_ipv6 rhost

	if [ "$TST_NET_IPV6_ENABLED" = 1 ]; then
		eval $(tst_net_ip_prefix $IPV6_LHOST || echo "exit $?")
		eval $(tst_net_ip_prefix -r $IPV6_RHOST || echo "exit $?")
	fi

	tst_net_use_netns && init_ltp_netspace

	eval $(tst_net_iface_prefix $IPV4_LHOST || echo "exit $?")
	eval $(tst_rhost_run -c 'tst_net_iface_prefix -r '$IPV4_RHOST \
		|| echo "exit $?")
	eval $(tst_net_vars $IPV4_LHOST/$IPV4_LPREFIX \
		$IPV4_RHOST/$IPV4_RPREFIX || echo "exit $?")

	if [ "$TST_NET_IPV6_ENABLED" = 1 ]; then
		tst_net_check_ifaces_ipv6
		eval $(tst_net_iface_prefix $IPV6_LHOST || echo "exit $?")
		eval $(tst_rhost_run -c 'tst_net_iface_prefix -r '$IPV6_RHOST \
			|| echo "exit $?")
		eval $(tst_net_vars $IPV6_LHOST/$IPV6_LPREFIX \
			$IPV6_RHOST/$IPV6_RPREFIX || echo "exit $?")
	fi

	tst_res_ TINFO "Network config (local -- remote):"
	tst_res_ TINFO "$LHOST_IFACES -- $RHOST_IFACES"
	tst_res_ TINFO "$IPV4_LHOST/$IPV4_LPREFIX -- $IPV4_RHOST/$IPV4_RPREFIX"
	tst_res_ TINFO "$IPV6_LHOST/$IPV6_LPREFIX -- $IPV6_RHOST/$IPV6_RPREFIX"

	if [ -n "$TST_USE_LEGACY_API" ]; then
		[ "$TST_IPV6" ] && tst_net_require_ipv6
		tst_net_remote_tmpdir
	fi
}

[ -n "$TST_USE_LEGACY_API" ] && . test.sh || . tst_test.sh

if [ -n "$TST_USE_LEGACY_API" ]; then
	tst_net_read_opts "$@"
else
	if [ "$TST_PARSE_ARGS_CALLER" = "$TST_PARSE_ARGS" ]; then
		tst_res_ TWARN "TST_PARSE_ARGS_CALLER same as TST_PARSE_ARGS, unset it ($TST_PARSE_ARGS)"
		unset TST_PARSE_ARGS_CALLER
	fi
	if [ "$TST_SETUP_CALLER" = "$TST_SETUP" ]; then
		tst_res_ TWARN "TST_SETUP_CALLER same as TST_SETUP, unset it ($TST_SETUP)"
		unset TST_SETUP_CALLER
	fi
	if [ "$TST_USAGE_CALLER" = "$TST_USAGE" ]; then
		tst_res_ TWARN "TST_USAGE_CALLER same as TST_USAGE, unset it ($TST_USAGE)"
		unset TST_USAGE_CALLER
	fi
fi

# detect IPv6 support on lhost for tests which don't use test links
tst_net_detect_ipv6

[ -n "$TST_NET_SKIP_VARIABLE_INIT" ] && return 0

# Management Link
[ -z "$RHOST" ] && TST_USE_NETNS="yes"
export RHOST="$RHOST"
export PASSWD="${PASSWD:-}"
# Don't use it in new tests, use tst_rhost_run() from tst_net.sh instead.
export LTP_RSH="${LTP_RSH:-ssh -nq}"

# Test Links
# IPV{4,6}_{L,R}HOST can be set with or without prefix (e.g. IP or IP/prefix),
# but if you use IP/prefix form, /prefix will be removed by tst_net_vars.
IPV4_LHOST="${IPV4_LHOST:-10.0.0.2/24}"
IPV4_RHOST="${IPV4_RHOST:-10.0.0.1/24}"
IPV6_LHOST="${IPV6_LHOST:-fd00:1:1:1::2/64}"
IPV6_RHOST="${IPV6_RHOST:-fd00:1:1:1::1/64}"

tst_net_setup_network

# More information about network parameters can be found
# in the following document: testcases/network/stress/README

export TST_NET_DATAROOT="$LTPROOT/testcases/bin/datafiles"

export TST_NETLOAD_CLN_REQUESTS="${TST_NETLOAD_CLN_REQUESTS:-10000}"
export TST_NETLOAD_CLN_NUMBER="${TST_NETLOAD_CLN_NUMBER:-2}"
export TST_NETLOAD_BINDTODEVICE="${TST_NETLOAD_BINDTODEVICE-1}"
export TST_NETLOAD_RUN_COUNT="${TST_NETLOAD_RUN_COUNT:-5}"
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
export ROUTE_CHANGE_IP="${ROUTE_CHANGE_IP:-100}"
export ROUTE_CHANGE_NETLINK="${ROUTE_CHANGE_NETLINK:-10000}"

# Warning: make sure to set valid interface names and IP addresses below.
# Set names for test interfaces, e.g. "eth0 eth1"
# This is fallback for LHOST_IFACES in case tst_net_vars finds nothing or we
# want to use more ifaces.
export LHOST_IFACES="${LHOST_IFACES:-eth0}"
export RHOST_IFACES="${RHOST_IFACES:-eth0}"
# Maximum payload size for 'virt' performance tests, by default eqauls to 1.1 * MTU
export TST_NET_MAX_PKT="${TST_NET_MAX_PKT:-$(tst_default_max_pkt)}"
# Set corresponding HW addresses, e.g. "00:00:00:00:00:01 00:00:00:00:00:02"
export LHOST_HWADDRS="${LHOST_HWADDRS:-$(tst_get_hwaddrs lhost)}"
export RHOST_HWADDRS="${RHOST_HWADDRS:-$(tst_get_hwaddrs rhost)}"

export NS_ICMPV4_SENDER_DATA_MAXSIZE=1472
export NS_ICMPV6_SENDER_DATA_MAXSIZE=1452

if [ -z "$TST_USE_LEGACY_API" ] && ! tst_cmd_available ping6; then
	ping6()
	{
		ping -6 $@
	}
	if [ -z "$_tst_net_ping6_warn_printed" ]; then
		tst_res_ TINFO "ping6 binary/symlink is missing, using workaround. Please, report missing ping6 to your distribution."
		export _tst_net_ping6_warn_printed=1
	fi
fi
