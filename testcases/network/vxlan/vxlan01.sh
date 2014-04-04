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
# Test-case 1: Local test, check if we can create 5000 VXLAN interfaces.
#

TCID=vxlan01
TST_TOTAL=1
. test_net.sh
. vxlan_lib.sh

max=$(( $start_vni + $vxlan_max ))
tst_resm TINFO "create $vxlan_max VXLANs, then delete them"
opt="group 239.1.1.1"

vnis=$(seq $start_vni $max)

for i in $vnis; do
	safe_run "ip link add ltp_vxl${i} type vxlan id $i $opt"
	safe_run "ip link set ltp_vxl${i} up"
done

for i in $vnis; do
	safe_run "ip link set ltp_vxl${i} down"
	safe_run "ip link delete ltp_vxl${i}"
done

tst_resm TPASS "done"

tst_exit
