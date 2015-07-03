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
# Test-case 1: Local test, check if we can create 4095 VLAN interfaces.
#

TCID=vlan01
TST_TOTAL=9

virt_type="vlan"

. test_net.sh
. virt_lib.sh

p0="protocol 802.1Q"
p1="protocol 802.1ad"
lb0="loose_binding off"
lb1="loose_binding on"
rh0="reorder_hdr off"
rh1="reorder_hdr on"

opts=" ,$p0 $lb0 $rh0,$p0 $lb0 $rh1,$p0 $lb1 $rh0,$p0 $lb1 $rh1,\
$p1 $lb0 $rh0,$p1 $lb0 $rh1,$p1 $lb1 $rh0,$p1 $lb1 $rh1,"

start_id=1
virt_count=400

for n in $(seq 1 $TST_TOTAL); do
	params="$(echo $opts | cut -d',' -f$n)"

	tst_resm TINFO "add $virt_type with '$params'"

	virt_add ltp_v0 id 0 $params > /dev/null 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TCONF "iproute or kernel doesn't support '$params'"
		params=""
	else
		ROD_SILENT "ip li delete ltp_v0"
	fi

	virt_multiple_add_test "$params"

	start_id=$(($start_id + $virt_count))
done

tst_exit
