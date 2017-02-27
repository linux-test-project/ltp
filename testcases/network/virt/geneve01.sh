#!/bin/sh
# Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
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

TCID=geneve01
TST_TOTAL=1

virt_type="geneve"
start_id=16700000

# Setting GENEVE tunnel with 'ip' command is very similar to VxLAN
# that is why using here 'vxlan_*' library functions.
vxlan_dst_addr="uni"

. test_net.sh
. virt_lib.sh

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

tst_resm TINFO "the same VNI must work"
# VNI is 24 bits long, so max value, which is not reserved, is 0xFFFFFE
vxlan_setup_subnet_$vxlan_dst_addr "id 0xFFFFFE" "id 0xFFFFFE"
virt_compare_netperf

tst_resm TINFO "different VNI shall not work together"
vxlan_setup_subnet_$vxlan_dst_addr "id 0xFFFFFE" "id 0xFFFFFD"
virt_compare_netperf "fail"

tst_exit
