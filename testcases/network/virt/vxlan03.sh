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
# Test-case 1: It requires remote host. Test will setup IPv4 and IPv6 virtual
#              sub-nets between two hosts, then will compare TCP performance
#              with and without VxLAN using ping or netstress test.
#
# Test-case 2: The same as above but must fail, because VXLAN allows
#              to communicate only within the same VXLAN segment.

TCID=vxlan03
TST_TOTAL=4

virt_type="vxlan"
start_id=16700000

# Destination address, can be unicast or multicast address
vxlan_dst_addr="uni"

. test_net.sh
. virt_lib.sh

# In average cases (with small packets less then 150 bytes) VxLAN slower
# by 10-30%. If hosts are too close to each other, e.g. connected to the same
# switch, VxLAN can be much slower when comparing to the performance without
# any encapsulation.
VIRT_PERF_THRESHOLD=${VIRT_PERF_THRESHOLD:-160}
[ "$VIRT_PERF_THRESHOLD" -lt 160 ] && VIRT_PERF_THRESHOLD=160

cleanup()
{
	cleanup_vifaces
	tst_rhost_run -c "ip link delete ltp_v0 2>/dev/null"
}
TST_CLEANUP="cleanup"

if [ -z $ip_local -o -z $ip_remote ]; then
	tst_brkm TBROK "you must specify IP address"
fi

opts=" ,gbp"

for n in $(seq 1 2); do
	p="$(echo $opts | cut -d',' -f$n)"

	virt_check_cmd virt_add ltp_v0 id 0 $p || continue

	tst_resm TINFO "the same VNI must work"
	# VNI is 24 bits long, so max value, which is not reserved, is 0xFFFFFE
	vxlan_setup_subnet_$vxlan_dst_addr "id 0xFFFFFE $p" "id 0xFFFFFE $p"
	virt_compare_netperf

	tst_resm TINFO "different VNI shall not work together"
	vxlan_setup_subnet_$vxlan_dst_addr "id 0xFFFFFE $p" "id 0xFFFFFD $p"
	virt_compare_netperf "fail"
done

tst_exit
