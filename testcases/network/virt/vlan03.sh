#!/bin/sh
# Copyright (c) 2015-2017 Oracle and/or its affiliates.
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
#              with and without VLAN using ping or netstress test.
#
# Test-case 2: The same as above but must fail, because VLAN allows
#              to communicate only within the same VLAN segment.

TCID=vlan03
TST_TOTAL=6
TST_NEEDS_TMPDIR=1

virt_type="vlan"

. test_net.sh
. virt_lib.sh

TST_CLEANUP="virt_cleanup"

if [ -z $ip_local -o -z $ip_remote ]; then
	tst_brkm TBROK "you must specify IP address"
fi

p0="protocol 802.1Q"
p1="protocol 802.1ad"
lb0="loose_binding off"
lb1="loose_binding on"
rh0="reorder_hdr off"
rh1="reorder_hdr on"

opts=" ,$p0 $lb0 $rh1,$p1 $lb1 $rh1"

for n in $(seq 1 3); do
	p="$(echo $opts | cut -d',' -f$n)"

	virt_check_cmd virt_add ltp_v0 id 0 $p || continue

	tst_resm TINFO "networks with the same VLAN ID must work"
	virt_setup "id 4094 $p" "id 4094 $p"
	virt_netperf_msg_sizes
	virt_cleanup_rmt

	tst_resm TINFO "different VLAN ID shall not work together"
	virt_setup "id 4093 $p" "id 4094 $p"
	virt_minimize_timeout
	virt_compare_netperf "fail"
	virt_cleanup_rmt
done

tst_exit
