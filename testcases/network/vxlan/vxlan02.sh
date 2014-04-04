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
# Test-case 1: Local test, check if we can create and then delete VXLAN
#              interface 5000 times.
#

TCID=vxlan02
TST_TOTAL=1
. test_net.sh
. vxlan_lib.sh

opt="group 239.1.1.1"
tst_resm TINFO "create, delete ltp_vxl1 $vxlan_max times"

for i in $(seq 0 $vxlan_max); do
	safe_run "ip link add ltp_vxl1 type vxlan id $start_vni $opt"
	safe_run "ip link set ltp_vxl1 up"
	safe_run "ip link delete ltp_vxl1"
done
tst_resm TPASS "done"

tst_exit
