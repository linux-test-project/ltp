#!/bin/sh
# Copyright (c) 2014-2016 Oracle and/or its affiliates. All Rights Reserved.
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
ip_virt_local="192.168.124.1"
ip6_virt_local="fe80::381c:c0ff:fea8:7c01"
mac_virt_local="3A:1C:C0:A8:7C:01"

ip_remote=$(tst_ipaddr rhost)
ip_virt_remote="192.168.124.2"
ip6_virt_remote="fe80::381c:c0ff:fea8:7c02"
mac_virt_remote="3A:1C:C0:A8:7C:02"

# Max performance loss (%) for virtual devices during network load
VIRT_PERF_THRESHOLD=${VIRT_PERF_THRESHOLD:-80}
vxlan_dstport=0

clients_num=2
client_requests=500000
max_requests=20

while getopts :hsx:i:r:c:R:p:n:t:d:6 opt; do
	case "$opt" in
	h)
		echo "Usage:"
		echo "h        help"
		echo "s        use ssh to run remote cmds"
		echo "x n      n is a number of interfaces for tc1 and tc2"
		echo "i n      start ID to use"
		echo "r n      client requests for TCP performance test"
		echo "c n      clients run concurrently in TCP perf"
		echo "R n      num of reqs, after which conn.closed in TCP perf"
		echo "p x      x and x + 1 are ports in TCP perf"
		echo "n x      virtual network 192.168.x"
		echo "t x      performance threshold, default is 60%"
		echo "d x      VxLAN destination address, 'uni' or 'multi'"
		echo "6        run over IPv6"
		exit 0
	;;
	s) TST_USE_SSH=1 ;;
	x) virt_count=$OPTARG ;;
	i) start_id=$OPTARG ;;
	c) clients_num=$OPTARG ;;
	r) client_requests=$OPTARG ;;
	R) max_requests=$OPTARG ;;
	p) srv_port=$OPTARG ;;
	n)
		ip_virt_local="192.168.${OPTARG}.1"
		ip_virt_remote="192.168.${OPTARG}.2"
	;;
	t) VIRT_PERF_THRESHOLD=$OPTARG ;;
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
		ip li add $vname type $virt_type $opt
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
		[ "$vxlan_dstport" -eq 1 ] && opt="dstport 0"
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
	local max=$(($start_id + $virt_count - 1))

	tst_resm TINFO "add $virt_count $virt_type, then delete"

	for i in $(seq $start_id $max); do
		ROD_SILENT "virt_add ltp_v$i id $i $opt"
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
	local max=$(($virt_count - 1))

	tst_resm TINFO "add/del $virt_type $virt_count times"

	for i in $(seq 0 $max); do
		ROD_SILENT "virt_add ltp_v0 $opt"
		ROD_SILENT "ip link set ltp_v0 up"
		ROD_SILENT "ip link delete ltp_v0"
	done
	tst_resm TPASS "done"
}

virt_setup()
{
	local opt="$1"
	local opt_r="$2"

	tst_resm TINFO "setup local ${virt_type} with '$opt'"
	ROD_SILENT "virt_add ltp_v0 $opt"

	tst_resm TINFO "setup rhost ${virt_type} with '$opt_r'"
	virt_add_rhost "$opt_r"

	case $virt_type in
	gre|ip6gre)
		# We can't set hwaddr to GRE tunnel, add IPv6 link local
		# addresses manually.
		ROD_SILENT "ip addr add ${ip6_virt_local}/64 dev ltp_v0"
		tst_rhost_run -s -c "ip ad add ${ip6_virt_remote}/64 dev ltp_v0"
	;;
	*)
		ROD_SILENT "ip li set ltp_v0 address $mac_virt_local"
		tst_rhost_run -s -c "ip li set ltp_v0 address $mac_virt_remote"
	;;
	esac

	ROD_SILENT "ip addr add ${ip_virt_local}/24 dev ltp_v0"
	tst_rhost_run -s -c "ip addr add ${ip_virt_remote}/24 dev ltp_v0"

	ROD_SILENT "ip li set up ltp_v0"
	tst_rhost_run -s -c "ip li set up ltp_v0"
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

	local opt="$1 $grp dev $(tst_iface)"
	local opt_r="$2 $grp dev $(tst_iface rhost)"

	virt_setup "$opt" "$opt_r"
}

virt_compare_netperf()
{
	local ret1="pass"
	local ret2="pass"
	local expect_res="${1:-pass}"

	tst_netload -H $ip_virt_remote -a $clients_num -R $max_requests \
		-r $client_requests -d res_ipv4 -e $expect_res || ret1="fail"

	tst_netload -H ${ip6_virt_remote}%ltp_v0 -a $clients_num \
		-R $max_requests -r $client_requests -d res_ipv6 \
		-e $expect_res || ret2="fail"

	ROD_SILENT "ip link delete ltp_v0"
	tst_rhost_run -s -c "ip link delete ltp_v0"

	[ "$ret1" = "fail" -o "$ret2" = "fail" ] && return

	local vt="$(cat res_ipv4)"
	local vt6="$(cat res_ipv6)"

	tst_netload -H $ip_remote -a $clients_num -R $max_requests \
		-r $client_requests -d res_ipv4

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

# Check if we can create then delete virtual interface n times.
# virt_test_01 [OPTIONS]
# OPTIONS - different options separated by comma.
virt_test_01()
{
	start_id=${start_id:-"1"}
	virt_count=${virt_count:-"400"}

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

		start_id=$(($start_id + $virt_count))
	done
}

# Check if we can create then delete virtual interface n times.
# virt_test_02 [OPTIONS]
# OPTIONS - different options separated by comma.
virt_test_02()
{
	start_id=${start_id:-"1"}
	virt_count=${virt_count:-"500"}

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

		start_id=$(($start_id + $virt_count))
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
