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
# Test-case 1: Local test, check if we can create and then delete ipvlan
#              interface 500 times.

TCID=ipvlan01
TST_TOTAL=2

virt_type="ipvlan"

. test_net.sh
. virt_lib.sh

modes="l2 l3"

start_id=1
virt_count=500

for m in $modes; do
	tst_resm TINFO "add $virt_type with mode '$m'"

	virt_check_cmd virt_add ltp_v0 mode $m || m=""

	virt_add_delete_test "mode $m"

	start_id=$(($start_id + $virt_count))
done

tst_exit
