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
# Test-case 1: Local test, check if we can create and then delete VXLAN
#              interface 5000 times.
#

TCID=vxlan02
TST_TOTAL=1

virt_type="vxlan"
start_id=16700000
virt_max=5000

. test_net.sh
. virt_lib.sh

opt="group 239.1.1.1"
tst_resm TINFO "create, delete ltp_v0 $virt_max times"

for i in $(seq 0 $virt_max); do
	ROD_SILENT "ip link add ltp_v0 type vxlan id $start_id $opt"
	ROD_SILENT "ip link set ltp_v0 up"
	ROD_SILENT "ip link delete ltp_v0"
done
tst_resm TPASS "done"

tst_exit
