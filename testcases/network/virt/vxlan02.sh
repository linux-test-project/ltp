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
# Local test, check if we can create and then delete VXLAN
# interface 5000 times.
#

TCID=vxlan02
TST_TOTAL=1

virt_type="vxlan"
start_id=16700000

. test_net.sh
. virt_lib.sh

[ "$TST_IPV6" ] && mult_addr="ff02::abc" || mult_addr="239.1.1.1"
opt="group $mult_addr"

virt_add_delete_test "id $start_id $opt dev $(tst_iface)"

tst_exit
