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
# Test-case 1: Local test, check if we can create and then delete macvtap
#              interface 500 times.

TCID=macvtap01
TST_TOTAL=4

virt_type="macvtap"

. test_net.sh
. virt_lib.sh

modes="private vepa bridge passthru"

start_id=1
virt_count=500

for m in $modes; do
	tst_resm TINFO "add $virt_type with mode '$m'"

	virt_add ltp_v0 mode $m > /dev/null 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TCONF "iproute or kernel doesn't support mode '$m'"
		m=""
	else
		ROD_SILENT "ip li delete ltp_v0"
	fi

	virt_add_delete_test "mode $m"

	start_id=$(($start_id + $virt_count))
done

tst_exit
