#!/bin/sh
# Copyright (c) 2014-2015 Oracle and/or its affiliates. All Rights Reserved.
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
# Test-case 1: It requires remote host. Test will setup IPv4 and IPv6 virtual
#              sub-nets between two hosts, then will compare TCP performance
#              with and without VxLAN using ping or tcp_fastopen test.
#
# Test-case 2: The same as above but must fail, because VXLAN allows
#              to communicate only within the same VXLAN segment.

TCID=vxlan03
TST_TOTAL=4

virt_type="vxlan"
start_id=16700000

# In average cases (with small packets less then 150 bytes) vxlan slower
# by 10-30%. If hosts are too close to each one, e.g. connected to the same
# switch the performance can be slower by 50%. Set 60% as default, the above
# will be an error in VXLAN.
virt_threshold=60

# Destination address, can be unicast or multicast address
vxlan_dst_addr="uni"

. test_net.sh
. virt_lib.sh

cleanup()
{
	cleanup_vifaces
	tst_rhost_run -c "ip link delete ltp_v0"
	if [ "$net_load" = "TFO" ]; then
		tst_rhost_run -c "pkill -9 tcp_fastopen\$"
		pkill -9 "tcp_fastopen\$"
	fi
}
TST_CLEANUP="cleanup"

if [ "$net_load" = "TFO" ]; then
	tst_check_cmds "tcp_fastopen"
fi

if [ -z $ip_local -o -z $ip_remote ]; then
	tst_brkm TBROK "you must specify IP address"
fi

opts=" ,dstport 0 gbp"

for n in $(seq 1 2); do
	params="$(echo $opts | cut -d',' -f$n)"

	virt_add ltp_v0 id 0 $params > /dev/null 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TCONF "iproute or kernel doesn't support '$params'"
		continue
	fi
	ROD_SILENT "ip li delete ltp_v0"

	tst_resm TINFO "networks with the same VNI must work"
	# VNI is 24 bits long, so max value, which is not reserved, is 0xFFFFFE
	res="TPASS"

	if [ "$vxlan_dst_addr" != 'uni' -a $vxlan_dst_addr != 'multi' ]; then
		tst_brkm TBROK "wrong dst address, can be 'uni' or 'multi'"
	fi

	vxlan_setup_subnet_$vxlan_dst_addr "0xFFFFFE" "0xFFFFFE"
	vxlan_compare_netperf || res="TFAIL"

	tst_resm $res "done"

	tst_resm TINFO "different VNI shall not work together"
	res="TPASS"
	vxlan_setup_subnet_$vxlan_dst_addr "0xFFFFFE" "0xFFFFFD"
	vxlan_compare_netperf && res="TFAIL"

	tst_resm $res "done"
done

tst_exit
