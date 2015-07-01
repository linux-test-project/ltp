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
# Test-case 1: Local test, check if we can create 5000 VXLAN interfaces.
#

TCID=vxlan01
TST_TOTAL=1

virt_type="vxlan"
start_id=16700000
virt_max=5000

. test_net.sh
. vxlan_lib.sh

max=$(( $start_id + $virt_max ))
tst_resm TINFO "create $virt_max VXLANs, then delete them"
opt="group 239.1.1.1"

vnis=$(seq $start_id $max)

for i in $vnis; do
	ROD_SILENT "ip link add ltp_v${i} type vxlan id $i $opt"
	ROD_SILENT "ip link set ltp_v${i} up"
done

for i in $vnis; do
	ROD_SILENT "ip link set ltp_v${i} down"
	ROD_SILENT "ip link delete ltp_v${i}"
done

tst_resm TPASS "done"

tst_exit
