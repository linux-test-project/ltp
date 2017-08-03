#!/bin/sh
# Copyright (c) 2017 Oracle and/or its affiliates. All Rights Reserved.
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
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

TCID=udp_ipsec_vti
TST_TOTAL=3
TST_CLEANUP="tst_ipsec_cleanup"

. ipsec_lib.sh

do_test()
{
	for p in $IPSEC_SIZE_ARRAY; do
		tst_netload -H $ip_rmt_tun -T udp -n $p -N $p \
			-r $IPSEC_REQUESTS
	done
}

tst_ipsec_setup_vti

do_test

tst_exit
