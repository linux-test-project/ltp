#!/bin/sh

# Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
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
# Virtual eXtensible Local Area Network (VXLAN) provides L2 networks
# over existed L3 networks. It is using UDP (port 8472) to encapsulate
# data packets. More information:
# http://tools.ietf.org/html/draft-mahalingam-dutt-dcops-vxlan-08
#
# Warning: Test assumes that machines don't have any existed VXLANs.
#          If machine has VXLANs, the test might fail or eventually delete
#          them in cleanup function. See "start_vni" variable which can
#          solve it.

ip_local=$(tst_ipaddr)
ip_vxlan_local="192.168.124.1"
ip6_vxlan_local="fe80::381c:c0ff:fea8:7c01"
mac_vxlan_local="3A:1C:C0:A8:7C:01"

ip_remote=$(tst_ipaddr rhost)
ip_vxlan_remote="192.168.124.2"
ip6_vxlan_remote="fe80::381c:c0ff:fea8:7c02"
mac_vxlan_remote="3A:1C:C0:A8:7C:02"

vxlan_max=5000
start_vni=16700000
clients_num=2
client_requests=500000
max_requests=3
net_load="TFO"

# In average cases (with small packets less then 150 bytes) vxlan slower
# by 10-30%. If hosts are too close to each one, e.g. connected to the same
# switch the performance can be slower by 50%. Set 60% as default, the above
# will be an error in VXLAN.
vxlan_threshold=60

# Destination address, can be unicast or multicast address
vxlan_dst_addr="uni"

while getopts :hsx:i:r:c:R:p:n:l:t:d:6 opt; do
	case "$opt" in
	h)
		echo "Usage:"
		echo "h        help"
		echo "s        use ssh to run remote cmds"
		echo "x n      n is a number of VXLANs for tc1 and tc2"
		echo "i n      start VNI to use"
		echo "r n      client requests for TCP performance test"
		echo "c n      clients run concurrently in TCP perf"
		echo "R n      num of reqs, after which conn.closed in TCP perf"
		echo "p x      x and x + 1 are ports in TCP perf"
		echo "n x      VXLAN network 192.168.x"
		echo "l x      network load: x is PING or TFO(tcp_fastopen)"
		echo "t x      VXLAN performance threshold, default is 60%"
		echo "d x      VXLAN destination address, 'uni' or 'multi'"
		echo "6        run over IPv6"
		exit 0
	;;
	s) TST_USE_SSH=1 ;;
	x) vxlan_max=$OPTARG ;;
	i) start_vni=$OPTARG ;;
	c) clients_num=$OPTARG ;;
	r) client_requests=$OPTARG ;;
	R) max_requests=$OPTARG ;;
	p) srv_port=$OPTARG ;;
	n)
		ip_vxlan_local="192.168.${OPTARG}.1"
		ip_vxlan_remote="192.168.${OPTARG}.2"
	;;
	l) net_load=$OPTARG ;;
	t) vxlan_threshold=$OPTARG ;;
	d) vxlan_dst_addr=$OPTARG ;;
	6) # skip, test_net library already processed it
	;;
	*)
		tst_brkm TBROK "unknown option: $opt"
	;;
	esac
done

cleanup_vxlans()
{
	tst_resm TINFO "cleanup vxlans..."
	local vxlans=`ip link | sed -nE 's/^[0-9]+: (ltp_vxl[0-9]+):.+/\1/p'`
	for vx in $vxlans; do
		ip link delete $vx
	done
}

TST_CLEANUP="cleanup_vxlans"
trap "tst_brkm TBROK 'test interrupted'" INT

safe_run()
{
	$@ > /dev/null 2>&1
	if [ $? -ne 0 ]; then
		tst_brkm TBROK "cmd failed: $@"
	fi
}

vxlan_setup_subnet_uni()
{
	tst_kvercmp 3 10 0 && \
		tst_brkm TCONF "test must be run with kernel 3.10 or newer"

	[ "$(ip li add type vxlan help 2>&1 | grep remote)" ] || \
		tst_brkm TCONF "iproute doesn't support remote unicast address"

	local opt="remote $(tst_ipaddr rhost)"

	tst_resm TINFO "setup VxLANv$ipver with unicast address: '$opt'"

	safe_run "ip li add ltp_vxl0 type vxlan id $1 $opt"
	safe_run "ip li set ltp_vxl0 address $mac_vxlan_local"
	safe_run "ip addr add ${ip_vxlan_local}/24 dev ltp_vxl0"
	safe_run "ip li set up ltp_vxl0"

	opt="remote $(tst_ipaddr)"

	tst_rhost_run -s -c "ip li add ltp_vxl0 type vxlan id $2 $opt"
	tst_rhost_run -s -c "ip li set ltp_vxl0 address $mac_vxlan_remote"
	tst_rhost_run -s -c "ip addr add ${ip_vxlan_remote}/24 dev ltp_vxl0"
	tst_rhost_run -s -c "ip li set up ltp_vxl0"
}

vxlan_setup_subnet_multi()
{
	local opt=
	[ "$TST_IPV6" ] && opt="group ff05::111" || opt="group 239.1.1.1"

	tst_resm TINFO "setup VxLANv$ipver with multicast address: '$opt'"

	safe_run "ip li add ltp_vxl0 type vxlan id $1 $opt dev $(tst_iface)"
	safe_run "ip li set ltp_vxl0 address $mac_vxlan_local"
	safe_run "ip addr add ${ip_vxlan_local}/24 dev ltp_vxl0"
	safe_run "ip li set up ltp_vxl0"

	tst_rhost_run -s -c "ip li add ltp_vxl0 type vxlan id $2 $opt \
	                     dev $(tst_iface rhost)"
	tst_rhost_run -s -c "ip li set ltp_vxl0 address $mac_vxlan_remote"
	tst_rhost_run -s -c "ip addr add ${ip_vxlan_remote}/24 dev ltp_vxl0"
	tst_rhost_run -s -c "ip li set up ltp_vxl0"
}

netload_test()
{
	local ip_addr="$1"
	local rfile="$2"
	local ret=0

	case "$net_load" in
	PING)
		local ipv6=
		echo "$ip_addr" | grep ":" > /dev/null
		[ $? -eq 0 ] && ipv6=6
		tst_resm TINFO "run ping${ipv6} test with rhost '$ip_addr'..."
		local res=
		res=$(ping${ipv6} -f -c $client_requests $ip_addr -w 600 2>&1)
		[ $? -ne 0 ] && return 1
		echo $res | sed -nE 's/.*time ([0-9]+)ms.*/\1/p' > $rfile
	;;
	TFO)
		local port=
		port=$(tst_rhost_run -c 'tst_get_unused_port ipv6 stream')
		[ $? -ne 0 ] && tst_brkm TBROK "failed to get unused port"

		tst_resm TINFO "run tcp_fastopen with '$ip_addr', port '$port'"
		tst_rhost_run -s -b -c "tcp_fastopen -R $max_requests -g $port"

		sleep 5

		# run local tcp client
		tcp_fastopen -a $clients_num -r $client_requests -l \
			-H $ip_addr -g $port -d $rfile > /dev/null || ret=1

		if [ $ret -eq 0 -a ! -f $rfile ]; then
			tst_brkm TBROK "can't read $rfile"
		fi

		tst_rhost_run -c "pkill -9 tcp_fastopen\$"
	;;
	*) tst_brkm TBROK "invalid net_load type '$net_load'" ;;
	esac

	return $ret
}

vxlan_compare_netperf()
{
	local ret=0
	netload_test $ip_vxlan_remote res_ipv4 || ret=1
	netload_test ${ip6_vxlan_remote}%ltp_vxl0 res_ipv6 || ret=1

	safe_run "ip link delete ltp_vxl0"
	tst_rhost_run -s -c "ip link delete ltp_vxl0"
	[ "$ret" -eq 1 ] && return 1
	local vt="$(cat res_ipv4)"
	local vt6="$(cat res_ipv6)"

	netload_test $ip_remote res_ipv4 || return 1

	local lt="$(cat res_ipv4)"
	tst_resm TINFO "time lan($lt) vxlan IPv4($vt) and IPv6($vt6) ms"

	per=$(( $vt * 100 / $lt - 100 ))
	per6=$(( $vt6 * 100 / $lt - 100 ))

	tst_resm TINFO "IPv4 VxLAN over IPv$ipver slower by $per %"
	tst_resm TINFO "IPv6 VxLAN over IPv$ipver slower by $per6 %"

	[ "$per" -ge "$vxlan_threshold" -o "$per6" -ge "$vxlan_threshold" ] \
		&& ret=1

	return $ret
}

tst_require_root

tst_kvercmp 3 8 0 && \
	tst_brkm TCONF "test must be run with kernel 3.8 or newer"

if [ "$TST_IPV6" ]; then
	tst_kvercmp 3 12 0 && \
		tst_brkm TCONF "test must be run with kernel 3.12 or newer"
fi

ipver=${TST_IPV6:-'4'}

tst_check_cmds "ip"

ip link add ltp_vxl type vxlan id $start_vni > /dev/null 2>&1
if [ $? -ne 0 ]; then
	tst_brkm TCONF "iproute2 or kernel doesn't support vxlan"
fi
safe_run "ip link delete ltp_vxl"
