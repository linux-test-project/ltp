#!/bin/sh
# Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
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

TCID=icmp-uni-vti
TST_TOTAL=1
TST_CLEANUP="tst_ipsec_cleanup"

. ipsec_lib.sh

do_test()
{
	PING_MAX="$IPSEC_REQUESTS"

	tst_resm TINFO "Sending ICMP messages..."
	tst_ping $tst_vti $ip_rmt_tun $IPSEC_SIZE_ARRAY
}

tst_ipsec_setup_vti

do_test

tst_exit
