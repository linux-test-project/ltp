#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018-2022 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2014-2021 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# VxLAN
# -----
# Virtual eXtensible Local Area Network (VxLAN) provides L2 networks
# over existed L3 networks. It is using UDP (port 8472) to encapsulate
# data packets. More information:
# http://tools.ietf.org/html/draft-mahalingam-dutt-dcops-vxlan-08
#
# Warning: Test assumes that machines don't have any existed VxLANs.
#          If machine has VxLANs, the test might fail or eventually delete
#          them in cleanup function. See "start_vni" variable which can
#          solve it.

TST_SETUP="${TST_SETUP:-virt_lib_setup}"
TST_CLEANUP="${TST_CLEANUP:-cleanup_vifaces}"
TST_NEEDS_ROOT=1

# Max performance loss (%) for virtual devices during network load
VIRT_PERF_THRESHOLD=${VIRT_PERF_THRESHOLD:-80}
if [ -n "$VIRT_PERF_THRESHOLD_MIN" ] && [ "$VIRT_PERF_THRESHOLD" -lt $VIRT_PERF_THRESHOLD_MIN ]; then
	 VIRT_PERF_THRESHOLD="$VIRT_PERF_THRESHOLD_MIN"
fi

virt_lib_usage()
{
	echo "i n     start ID to use"
	echo "d x     VxLAN destination address, 'uni' or 'multi'"
}

virt_lib_parse_args()
{
	case "$1" in
	i) start_id=$2 ;;
	d) vxlan_dst_addr=$2 ;;
	esac
}

virt_lib_setup()
{
	case "$virt_type" in
	vxlan|geneve)
		if [ "$TST_IPV6" ] && tst_kvcmp -lt "3.12"; then
			tst_brk TCONF "test must be run with kernels >= 3.12"
		fi

		# newer versions of 'ip' complain if this option not set
		ip link add type vxlan help 2>&1 | grep -q dstport && vxlan_dstport=1
	;;
	esac

	tst_require_cmds "ip"

	virt_add ltp_v0 || \
		tst_brk TCONF "iproute2 or kernel doesn't support $virt_type"

	ROD_SILENT "ip link delete ltp_v0"
}

cleanup_vifaces()
{
	tst_res TINFO "cleanup virtual interfaces..."
	local viface=`ip link | sed -nE 's/^[0-9]+: (ltp_v[0-9]+)[@:].+/\1/p'`
	for vx in $viface; do
		ip link delete $vx
	done
}

virt_cleanup_rmt()
{
	cleanup_vifaces
	tst_rhost_run -c "ip link delete ltp_v0 2>/dev/null"
	if [ "$virt_tcp_syn" ]; then
		sysctl -q net.ipv4.tcp_syn_retries=$virt_tcp_syn
		virt_tcp_syn=
	fi
}

virt_cleanup()
{
	virt_cleanup_rmt
}

_get_gue_fou_tnl()
{
	local enc_type="$1"
	local tnl=sit

	if [ "$enc_type" = "gue" ]; then
		[ -n "$TST_IPV6" ] && tnl="ip6tnl"
	else
		[ -n "$TST_IPV6" ] && tnl="ip6gre" || tnl="gre"
	fi
	echo "$tnl"
}

virt_add()
{
	local vname=$1
	shift
	local opt="$*"

	case $virt_type in
	vlan|vxlan)
		[ -z "$opt" ] && opt="id 4094"
		[ "$vxlan_dstport" -eq 1 ] && opt="dstport 0 $opt"
		[ "$virt_type" = "vxlan" ] && opt="$opt dev $(tst_iface)"
	;;
	geneve)
		[ -z "$opt" ] && opt="id 4094 remote $(tst_ipaddr rhost)"
	;;
	gre|ip6gre)
		[ -z "$opt" ] && \
			opt="remote $(tst_ipaddr rhost) dev $(tst_iface)"
	;;
	sit|gue|fou)
		[ -z "$opt" ] && opt="remote $(tst_ipaddr rhost) local $(tst_ipaddr)"
	;;
	esac

	case $virt_type in
	vxlan|geneve|sit|wireguard)
		ip link add $vname type $virt_type $opt
	;;
	gre|ip6gre)
		ip -f inet$TST_IPV6 tu add $vname mode $virt_type $opt
	;;
	gue|fou)
		ip link add name $vname type $(_get_gue_fou_tnl $virt_type) $opt
	;;
	*)
		ip link add link $(tst_iface) $vname type $virt_type $opt
	;;
	esac
}

virt_add_rhost()
{
	local opt=""
	case $virt_type in
	vxlan|geneve)
		[ "$virt_type" = "vxlan" ] && opt="dev $(tst_iface rhost)"
		[ "$vxlan_dstport" -eq 1 ] && opt="$opt dstport 0"
		tst_rhost_run -s -c "ip link add ltp_v0 type $virt_type $@ $opt"
	;;
	sit|wireguard)
		tst_rhost_run -s -c "ip link add ltp_v0 type $virt_type $@"
	;;
	gre|ip6gre)
		tst_rhost_run -s -c "ip -f inet$TST_IPV6 tu add ltp_v0 \
				     mode $virt_type $@"
	;;
	gue|fou)
		tst_rhost_run -s -c "ip link add name ltp_v0 \
				     type $(_get_gue_fou_tnl $virt_type) $@"
	;;
	*)
		tst_rhost_run -s -c "ip link add link $(tst_iface rhost) ltp_v0 \
				     type $virt_type $@"
	;;
	esac
}

virt_multiple_add_test()
{
	local opt="$@"
	local max=$(($start_id + $NS_TIMES - 1))
	local i

	tst_res TINFO "add $NS_TIMES $virt_type, then delete"

	for i in $(seq $start_id $max); do
		virt_add ltp_v$i id $i $opt || \
			tst_brk TFAIL "failed to create 'ltp_v0 $opt'"
		ROD_SILENT "ip link set ltp_v$i up"
	done

	for i in $(seq $start_id $max); do
		ROD_SILENT "ip link set ltp_v$i down"
		ROD_SILENT "ip link delete ltp_v$i"
	done

	tst_res TPASS "done"
}

virt_add_delete_test()
{
	local opt="$@"
	local max=$(($NS_TIMES - 1))
	local i

	tst_res TINFO "add/del $virt_type $NS_TIMES times"

	for i in $(seq 0 $max); do
		virt_add ltp_v0 $opt || \
			tst_brk TFAIL "failed to create 'ltp_v0 $opt'"
		ROD_SILENT "ip link set ltp_v0 up"
		ROD_SILENT "ip link delete ltp_v0"
	done
	tst_res TPASS "done"
}

virt_setup()
{
	local opt="$1"
	local opt_r="${2:-$1}"

	tst_res TINFO "setup local ${virt_type} with '$opt'"
	virt_add ltp_v0 $opt || \
		tst_brk TBROK "failed to create 'ltp_v0 $opt'"

	tst_res TINFO "setup rhost ${virt_type} with '$opt_r'"
	virt_add_rhost "$opt_r"

	ROD_SILENT "ip addr add ${ip6_virt_local}/64 dev ltp_v0 nodad"
	tst_rhost_run -s -c "ip addr add ${ip6_virt_remote}/64 dev ltp_v0 nodad"

	ROD_SILENT "ip addr add ${ip_virt_local}/24 dev ltp_v0"
	tst_rhost_run -s -c "ip addr add ${ip_virt_remote}/24 dev ltp_v0"

	ROD_SILENT "sysctl -q net.ipv6.conf.ltp_v0.accept_dad=0"
	tst_rhost_run -s -c "sysctl -q net.ipv6.conf.ltp_v0.accept_dad=0"

	ROD_SILENT "ip link set up ltp_v0"
	tst_rhost_run -s -c "ip link set up ltp_v0"
}

virt_tcp_syn=
virt_minimize_timeout()
{
	local mac_loc="$(cat /sys/class/net/ltp_v0/address)"
	local mac_rmt="$(tst_rhost_run -c 'cat /sys/class/net/ltp_v0/address')"

	if [ "$mac_loc" ]; then
		ROD_SILENT "ip neigh replace $ip_virt_remote lladdr \
			    $mac_rmt nud permanent dev ltp_v0"
		tst_rhost_run -s -c "ip neigh replace $ip_virt_local lladdr \
				     $mac_loc nud permanent dev ltp_v0"
	fi

	virt_tcp_syn=$(sysctl -n net.ipv4.tcp_syn_retries)
	ROD sysctl -q net.ipv4.tcp_syn_retries=1
}

vxlan_setup_subnet_uni()
{
	[ "$(ip link add type $virt_type help 2>&1 | grep remote)" ] || \
		tst_brk TCONF "iproute doesn't support remote unicast address"

	local opt="$1 remote $(tst_ipaddr rhost)"
	local opt_r="$2 remote $(tst_ipaddr)"

	virt_setup "$opt" "$opt_r"
}

vxlan_setup_subnet_multi()
{
	tst_require_cmds "od"
	local b1=$(($(od -An -d -N1 /dev/urandom) % 254 + 1))
	local b2=$(($(od -An -d -N1 /dev/urandom) % 254 + 1))
	local b3=$(($(od -An -d -N1 /dev/urandom) % 254 + 1))

	local grp=
	if [ "$TST_IPV6" ]; then
		grp="group ff05::$(printf '%x:%x%x' $b1 $b2 $b3)"
	else
		grp="group 239.$b1.$b2.$b3"
	fi

	local opt="$1 $grp"
	local opt_r="$2 $grp"

	virt_setup "$opt" "$opt_r"
}

virt_compare_netperf()
{
	local ret1="pass"
	local ret2="pass"
	local expect_res="${1:-pass}"
	local opts="$2"

	tst_netload -H $ip_virt_remote $opts -d res_ipv4 -e $expect_res \
		-D ltp_v0 || ret1="fail"

	tst_netload -H ${ip6_virt_remote} $opts -d res_ipv6 -e $expect_res \
		-D ltp_v0 || ret2="fail"

	[ "$ret1" = "fail" -o "$ret2" = "fail" ] && return

	local vt="$(cat res_ipv4)"
	local vt6="$(cat res_ipv6)"

	tst_netload -H $(tst_ipaddr rhost) $opts -d res_lan

	local lt="$(cat res_lan)"
	tst_res TINFO "time lan IPv${TST_IPVER}($lt) $virt_type IPv4($vt) and IPv6($vt6) ms"

	tst_netload_compare $lt $vt "-$VIRT_PERF_THRESHOLD"
	tst_netload_compare $lt $vt6 "-$VIRT_PERF_THRESHOLD"
}

virt_check_cmd()
{
	$@ > /dev/null 2>&1
	if [ $? -ne 0 ]; then
		tst_res TCONF "'$@' option(s) not supported, skipping it"
		return 1
	fi
	ROD_SILENT "ip link delete ltp_v0"
	return 0
}

virt_netperf_msg_sizes()
{
	local sizes="100 1000 2000 $TST_NET_MAX_PKT"
	local s

	for s in $sizes; do
		virt_compare_netperf pass "-n $s -N $s"
	done
}

virt_netperf_rand_sizes()
{
	local max_pkt_size="$TST_NET_MAX_PKT"
	local types="tcp udp udp_lite"

	for t in $types; do
		virt_compare_netperf pass "-A $max_pkt_size -T $t"
	done
}

# Check if we can create then delete virtual interface.
virt_test_01()
{
	start_id="${start_id:-1}"

	tst_res TINFO "add $virt_type with '$2'"
	virt_check_cmd virt_add ltp_v0 id 0 $2 || return
	virt_multiple_add_test "$2"
}

# Check if we can create then delete virtual interface.
virt_test_02()
{
	start_id="${start_id:-1}"

	tst_res TINFO "add and then delete $virt_type with '$2'"
	virt_check_cmd virt_add ltp_v0 $2 || return
	virt_add_delete_test "$2"
	start_id=$(($start_id + $NS_TIMES))
}

virt_gre_setup()
{
	virt_type="gre"
	[ "$TST_IPV6" ] && virt_type="ip6gre"
	virt_lib_setup

	tst_res TINFO "test $virt_type"
	virt_setup "local $(tst_ipaddr) remote $(tst_ipaddr rhost) dev $(tst_iface)" \
	"local $(tst_ipaddr rhost) remote $(tst_ipaddr) dev $(tst_iface rhost)"
}

. tst_net.sh

ip_virt_local="$(TST_IPV6= tst_ipaddr_un)"
ip6_virt_local="$(TST_IPV6=6 tst_ipaddr_un)"

ip_virt_remote="$(TST_IPV6= tst_ipaddr_un rhost)"
ip6_virt_remote="$(TST_IPV6=6 tst_ipaddr_un rhost)"

vxlan_dstport=0
