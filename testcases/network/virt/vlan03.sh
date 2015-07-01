#!/bin/sh
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
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
#              with and without VLAN using ping or tcp_fastopen test.
#
# Test-case 2: The same as above but must fail, because VLAN allows
#              to communicate only within the same VLAN segment.

TCID=vlan03
TST_TOTAL=2

virt_type="vlan"

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

tst_resm TINFO "networks with the same VLAN ID must work"

res="TPASS"

virt_setup "id 4094" "id 4094"
virt_compare_netperf || res="TFAIL"

tst_resm $res "done"

tst_resm TINFO "different VLAN ID shall not work together"
res="TPASS"
virt_setup "id 4093" "id 4094"
virt_compare_netperf && res="TFAIL"

tst_resm $res "done"

tst_exit
