#!/bin/sh
# Copyright (c) 2014-2017 Oracle and/or its affiliates. All Rights Reserved.
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

ip_local=$(tst_ipaddr)
ip_virt_local="$(TST_IPV6= tst_ipaddr_un)"
ip6_virt_local="$(TST_IPV6=6 tst_ipaddr_un)"

ip_remote=$(tst_ipaddr rhost)
ip_virt_remote="$(TST_IPV6= tst_ipaddr_un rhost)"
ip6_virt_remote="$(TST_IPV6=6 tst_ipaddr_un rhost)"

# Max performance loss (%) for virtual devices during network load
VIRT_PERF_THRESHOLD=${VIRT_PERF_THRESHOLD:-80}
vxlan_dstport=0

while getopts :hi:d:6 opt; do
	case "$opt" in
	h)
		echo "Usage:"
		echo "h        help"
		echo "i n      start ID to use"
		echo "d x      VxLAN destination address, 'uni' or 'multi'"
		echo "6        run over IPv6"
		exit 0
	;;
	i) start_id=$OPTARG ;;
	d) vxlan_dst_addr=$OPTARG ;;
	6) # skip, test_net library already processed it
	;;
	*)
		tst_brkm TBROK "unknown option: $opt"
	;;
	esac
done

cleanup_vifaces()
{
	tst_resm TINFO "cleanup virtual interfaces..."
	local viface=`ip li | sed -nE 's/^[0-9]+: (ltp_v[0-9]+)[@:].+/\1/p'`
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
	[ "$TST_NEEDS_TMPDIR" = 1 ] && tst_rmdir
}

TST_CLEANUP="cleanup_vifaces"
trap "tst_brkm TBROK 'test interrupted'" INT

virt_add()
{
	local vname=$1
	shift
	local opt="$*"

	case $virt_type in
	vlan|vxlan)
		[ -z "$opt" ] && opt="id 4094"
		[ "$vxlan_dstport" -eq 1 ] && opt="dstport 0 $opt"
	;;
	geneve)
		[ -z "$opt" ] && opt="id 4094 remote $(tst_ipaddr rhost)"
	;;
	gre|ip6gre)
		[ -z "$opt" ] && \
			opt="remote $(tst_ipaddr rhost) dev $(tst_iface)"
	;;
	esac

	case $virt_type in
	vxlan|geneve)
		ip li add $vname type $virt_type $opt dev $(tst_iface)
	;;
	gre|ip6gre)
		ip -f inet$TST_IPV6 tu add $vname mode $virt_type $opt
	;;
	*)
		ip li add link $(tst_iface) $vname type $virt_type $opt
	;;
	esac
}

virt_add_rhost()
{
	local opt=""
	case $virt_type in
	vxlan|geneve)
		opt="dev $(tst_iface rhost)"
		[ "$vxlan_dstport" -eq 1 ] && opt="$opt dstport 0"
		tst_rhost_run -s -c "ip li add ltp_v0 type $virt_type $@ $opt"
	;;
	gre|ip6gre)
		tst_rhost_run -s -c "ip -f inet$TST_IPV6 tu add ltp_v0 \
				     mode $virt_type $@"
	;;
	*)
		tst_rhost_run -s -c "ip li add link $(tst_iface rhost) ltp_v0 \
				     type $virt_type $@"
	;;
	esac
}

virt_multiple_add_test()
{
	local opt="$@"
	local max=$(($start_id + $NS_TIMES - 1))

	tst_resm TINFO "add $NS_TIMES $virt_type, then delete"

	for i in $(seq $start_id $max); do
		virt_add ltp_v$i id $i $opt || \
			tst_brkm TFAIL "failed to create 'ltp_v0 $opt'"
		ROD_SILENT "ip link set ltp_v$i up"
	done

	for i in $(seq $start_id $max); do
		ROD_SILENT "ip link set ltp_v$i down"
		ROD_SILENT "ip link delete ltp_v$i"
	done

	tst_resm TPASS "done"
}

virt_add_delete_test()
{
	local opt="$@"
	local max=$(($NS_TIMES - 1))

	tst_resm TINFO "add/del $virt_type $NS_TIMES times"

	for i in $(seq 0 $max); do
		virt_add ltp_v0 $opt || \
			tst_brkm TFAIL "failed to create 'ltp_v0 $opt'"
		ROD_SILENT "ip link set ltp_v0 up"
		ROD_SILENT "ip link delete ltp_v0"
	done
	tst_resm TPASS "done"
}

virt_setup()
{
	local opt="$1"
	local opt_r="${2:-$1}"

	tst_resm TINFO "setup local ${virt_type} with '$opt'"
	virt_add ltp_v0 $opt || \
		tst_brkm TBROK "failed to create 'ltp_v0 $opt'"

	tst_resm TINFO "setup rhost ${virt_type} with '$opt_r'"
	virt_add_rhost "$opt_r"

	ROD_SILENT "ip addr add ${ip6_virt_local}/64 dev ltp_v0 nodad"
	tst_rhost_run -s -c "ip ad add ${ip6_virt_remote}/64 dev ltp_v0 nodad"

	ROD_SILENT "ip addr add ${ip_virt_local}/24 dev ltp_v0"
	tst_rhost_run -s -c "ip addr add ${ip_virt_remote}/24 dev ltp_v0"

	ROD_SILENT "sysctl -q net.ipv6.conf.ltp_v0.accept_dad=0"
	tst_rhost_run -s -c "sysctl -q net.ipv6.conf.ltp_v0.accept_dad=0"

	ROD_SILENT "ip li set up ltp_v0"
	tst_rhost_run -s -c "ip li set up ltp_v0"
}

virt_tcp_syn=
virt_minimize_timeout()
{
	local mac_loc="$(cat /sys/class/net/ltp_v0/address)"
	local mac_rmt="$(tst_rhost_run -c 'cat /sys/class/net/ltp_v0/address')"

	ROD_SILENT "ip ne replace $ip_virt_remote lladdr \
		    $mac_rmt nud permanent dev ltp_v0"
	tst_rhost_run -s -c "ip ne replace $ip_virt_local lladdr \
			     $mac_loc nud permanent dev ltp_v0"
	virt_tcp_syn=$(sysctl -n net.ipv4.tcp_syn_retries)
	ROD sysctl -q net.ipv4.tcp_syn_retries=1
}

vxlan_setup_subnet_uni()
{
	if tst_kvcmp -lt "3.10"; then
		tst_brkm TCONF "test must be run with kernel 3.10 or newer"
	fi

	[ "$(ip li add type $virt_type help 2>&1 | grep remote)" ] || \
		tst_brkm TCONF "iproute doesn't support remote unicast address"

	local opt="$1 remote $(tst_ipaddr rhost)"
	local opt_r="$2 remote $(tst_ipaddr)"

	virt_setup "$opt" "$opt_r"
}

vxlan_setup_subnet_multi()
{
	tst_check_cmds "od"
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

	tst_netload -H $ip_virt_remote $opts -d res_ipv4 -e $expect_res || \
		ret1="fail"

	tst_netload -H ${ip6_virt_remote} $opts -d res_ipv6 -e $expect_res || \
		ret2="fail"

	[ "$ret1" = "fail" -o "$ret2" = "fail" ] && return

	local vt="$(cat res_ipv4)"
	local vt6="$(cat res_ipv6)"

	tst_netload -H $ip_remote $opts -d res_ipv4

	local lt="$(cat res_ipv4)"
	tst_resm TINFO "time lan($lt) $virt_type IPv4($vt) and IPv6($vt6) ms"

	per=$(( $vt * 100 / $lt - 100 ))
	per6=$(( $vt6 * 100 / $lt - 100 ))

	case "$virt_type" in
	vxlan|geneve)
		tst_resm TINFO "IP4 $virt_type over IP$ipver slower by $per %"
		tst_resm TINFO "IP6 $virt_type over IP$ipver slower by $per6 %"
	;;
	*)
		tst_resm TINFO "IP4 $virt_type slower by $per %"
		tst_resm TINFO "IP6 $virt_type slower by $per6 %"
	esac

	if [ "$per" -ge "$VIRT_PERF_THRESHOLD" -o \
	     "$per6" -ge "$VIRT_PERF_THRESHOLD" ]; then
		tst_resm TFAIL "Test failed, threshold: $VIRT_PERF_THRESHOLD %"
	else
		tst_resm TPASS "Test passed, threshold: $VIRT_PERF_THRESHOLD %"
	fi
}

virt_check_cmd()
{
	$@ > /dev/null 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TCONF "'$@' option(s) not supported, skipping it"
		return 1
	fi
	ROD_SILENT "ip li delete ltp_v0"
	return 0
}

# virt_macsec_setup [OPTIONS]
# OPTIONS - [ cipher { default | gcm-aes-128 } ] [ encrypt { on | off } ]
#           [ protect { on | off } ] [ replay { on | off } ] [ window WINDOW ]
#           [ validate { strict | check | disabled } ]
virt_macsec_setup()
{
	local keyid0=01
	local keyid1=02
	local sa=0
	local h0=$(tst_hwaddr)
	local h1=$(tst_hwaddr rhost)
	local cmd="ip macsec add ltp_v0"
	local key0="01234567890123456789012345678901"
	local key1="98765432109876543210987612343434"

	virt_setup "icvlen 16 encodingsa $sa $@"

	ROD $cmd tx sa $sa pn 100 on key $keyid0 $key0
	ROD $cmd rx address $h1 port 1
	ROD $cmd rx address $h1 port 1 sa $sa pn 100 on key $keyid1 $key1

	tst_rhost_run -s -c "$cmd tx sa $sa pn 100 on key $keyid1 $key1"
	tst_rhost_run -s -c "$cmd rx address $h0 port 1"
	tst_rhost_run -s -c \
		"$cmd rx address $h0 port 1 sa $sa pn 100 on key $keyid0 $key0"
}

virt_netperf_msg_sizes()
{
	local sizes="${@:-100 1000 2000 10000}"
	client_requests=20000

	for s in $sizes; do
		virt_compare_netperf pass "-n $s -N $s"
	done
}

# Check if we can create then delete virtual interface n times.
# virt_test_01 [OPTIONS]
# OPTIONS - different options separated by comma.
virt_test_01()
{
	start_id=${start_id:-"1"}
	local opts=${1:-""}
	local n=0

	while true; do
		n=$((n + 1))
		p="$(echo $opts | cut -d',' -f$n)"
		if [ -z "$p" -a $n -gt 1 ]; then
			break
		fi

		tst_resm TINFO "add $virt_type with '$p'"

		virt_check_cmd virt_add ltp_v0 id 0 $p || continue

		virt_multiple_add_test "$p"

		start_id=$(($start_id + $NS_TIMES))
	done
}

# Check if we can create then delete virtual interface n times.
# virt_test_02 [OPTIONS]
# OPTIONS - different options separated by comma.
virt_test_02()
{
	start_id=${start_id:-"1"}
	local opts=${1:-""}
	local n=0

	while true; do
		n=$((n + 1))
		p="$(echo $opts | cut -d',' -f$n)"
		if [ -z "$p" -a $n -gt 1 ]; then
			break
		fi

		tst_resm TINFO "add and then delete $virt_type with '$p'"

		virt_check_cmd virt_add ltp_v0 $p || continue

		virt_add_delete_test "$p"

		start_id=$(($start_id + $NS_TIMES))
	done
}

tst_require_root

case "$virt_type" in
vxlan|geneve)
	if tst_kvcmp -lt "3.8"; then
		tst_brkm TCONF "test must be run with kernel 3.8 or newer"
	fi

	if [ "$TST_IPV6" ] && tst_kvcmp -lt "3.12"; then
		tst_brkm TCONF "test must be run with kernels >= 3.12"
	fi

	# newer versions of 'ip' complain if this option not set
	ip li add type vxlan help 2>&1 | grep -q dstport && vxlan_dstport=1
;;
esac

ipver=${TST_IPV6:-'4'}

tst_check_cmds "ip"

virt_add ltp_v0 || \
	tst_brkm TCONF "iproute2 or kernel doesn't support $virt_type"

ROD_SILENT "ip link delete ltp_v0"
