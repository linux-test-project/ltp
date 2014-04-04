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

user_name="root"

ip_local=${IPV4_NETWORK}.${LHOST_IPV4_HOST}
ip_vxlan_local="192.168.124.1"
mac_vxlan_local="3A:1C:C0:A8:7C:01"

ip_remote=${IPV4_NETWORK}.${RHOST_IPV4_HOST}
ip_vxlan_remote="192.168.124.2"
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

while getopts :hsx:i:r:c:R:p:n:l:t: opt; do
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

vxlan_setup_subnet()
{
	tst_resm TINFO "virtual bridge & VXLAN, connect 2 hosts"
	local opt="rsc proxy nolearning"

	safe_run "ip link add ltp_vxl0 type vxlan id $1 $opt"
	safe_run "ip link set ltp_vxl0 address $mac_vxlan_local"
	safe_run "ip address add ${ip_vxlan_local}/24 dev ltp_vxl0"
	safe_run "ip link set up ltp_vxl0"

	safe_run "arp -s $ip_vxlan_remote $mac_vxlan_remote -i ltp_vxl0"
	safe_run "bridge fdb add to $mac_vxlan_remote dst $ip_remote \
	          dev ltp_vxl0"

	tst_rhost_run -s -c "ip link add ltp_vxl0 type vxlan id $2 $opt"
	tst_rhost_run -s -c "ip link set ltp_vxl0 address $mac_vxlan_remote"
	tst_rhost_run -s -c "ip address add ${ip_vxlan_remote}/24 dev ltp_vxl0"
	tst_rhost_run -s -c "ip link set up ltp_vxl0"

	tst_rhost_run -s -c "arp -s $ip_vxlan_local $mac_vxlan_local \
	                     -i ltp_vxl0"
	tst_rhost_run -s -c "bridge fdb add to $mac_vxlan_local dst $ip_local \
	                     dev ltp_vxl0"
}

netload_test()
{
	local ip_addr="$1"
	local rfile="$2"
	local ret=0

	case "$net_load" in
	PING)
		tst_resm TINFO "run ping test with rhost '$ip_addr'..."
		local res=
		res=$(ping -f -c $client_requests $ip_addr -w 600 2>&1)
		[ $? -ne 0 ] && return 1
		echo $res | sed -nE 's/.*time ([0-9]+)ms.*/\1/p' > $rfile
	;;
	TFO)
		tst_resm TINFO "run tcp_fastopen test with rhost '$ip_addr'"
		local port=
		port=$(tst_rhost_run -c 'tst_get_unused_port ipv4 stream')
		[ $? -ne 0 ] && tst_brkm TBROK "failed to get unused port"

		tst_rhost_run -s -b -c "tcp_fastopen -R $max_requests -g $port"

		sleep 5

		# run local tcp client
		tcp_fastopen -a $clients_num -r $client_requests -l \
			-H $ip_addr -g $port -d $rfile > /dev/null 2>&1 || ret=1

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
	local res_file="${TMPDIR}/vxlan_result"

	netload_test $ip_vxlan_remote $res_file || ret=1

	safe_run "ip link delete ltp_vxl0"
	tst_rhost_run -s -c "ip link delete ltp_vxl0"
	[ "$ret" -eq 1 ] && return 1
	local vt="$(cat $res_file)"

	netload_test $ip_remote $res_file || return 1

	local lt="$(cat $res_file)"
	tst_resm TINFO "time lan($lt) vxlan($vt) ms"

	per=$(( $vt * 100 / $lt - 100 ))

	if [ "$per" -lt "$vxlan_threshold" ]; then
		tst_resm TINFO "vxlan slower by $per %"
	else
		tst_resm TINFO "vxlan too slow: by $per %"
		ret=1
	fi

	return $ret
}

tst_require_root

tst_kvercmp 3 8 0
if [ $? -eq 0 ]; then
	tst_brkm TCONF "test must be run with kernel 3.8 or newer"
fi

tst_check_cmds "ip"

ip link add ltp_vxl type vxlan id $start_vni > /dev/null 2>&1
if [ $? -ne 0 ]; then
	tst_brkm TCONF "iproute2 or kernel doesn't support vxlan"
fi
safe_run "ip link delete ltp_vxl"
