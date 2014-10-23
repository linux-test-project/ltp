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
# Test-case 1: It requires remote host. Test will setup IPv4 and IPv6 virtual
#              sub-nets between two hosts, then will compare TCP performance
#              with and without VxLAN using ping or tcp_fastopen test.
#
# Test-case 2: The same as above but must fail, because VXLAN allows
#              to communicate only within the same VXLAN segment.

TCID=vxlan03
TST_TOTAL=2
. test_net.sh
. vxlan_lib.sh

cleanup()
{
	cleanup_vxlans
	tst_rhost_run -c "ip link delete ltp_vxl0"
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

tst_resm TINFO "networks with the same VNI must work"
# VNI is 24 bits long, so max value, which is not reserved, is 0xFFFFFE
res="TPASS"
vxlan_setup_subnet "0xFFFFFE" "0xFFFFFE"
vxlan_compare_netperf || res="TFAIL"

tst_resm $res "done"

tst_resm TINFO "different VNI shall not work together"
res="TPASS"
vxlan_setup_subnet "0xFFFFFE" "0xFFFFFD"
vxlan_compare_netperf && res="TFAIL"

tst_resm $res "done"

tst_exit
