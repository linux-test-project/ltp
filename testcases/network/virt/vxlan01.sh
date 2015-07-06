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
# Local test, check if we can create multiple VXLAN interfaces.
#

TCID=vxlan01
TST_TOTAL=5

virt_type="vxlan"
start_id=16700000

. test_net.sh
. virt_lib.sh

options="l2miss l3miss,norsc nolearning noproxy,\
ttl 0x01 tos 0x01,ttl 0xff tos 0xff,gbp"

virt_test_01 "$options"

tst_exit
