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

do_setup()
{
	IPSEC_SIZE_ARRAY="${IPSEC_SIZE_ARRAY:-10 100 1000 10000 65507}"

	if_loc=$(tst_iface)
	if_rmt=$(tst_iface rhost)

	ip_loc=$(tst_ipaddr)
	ip_rmt=$(tst_ipaddr rhost)

	tst_vti="ltp_vti0"

	tst_resm TINFO "Test vti$TST_IPV6 + IPsec[$IPSEC_PROTO/$IPSEC_MODE]"

	tst_ipsec_vti lhost $ip_loc $ip_rmt $tst_vti
	tst_ipsec_vti rhost $ip_rmt $ip_loc $tst_vti

	local mask=
	if [ "$TST_IPV6" ]; then
		ip_loc_tun="${IPV6_NET32_UNUSED}::1";
		ip_rmt_tun="${IPV6_NET32_UNUSED}::2";
		mask=64
		ROD ip -6 route add ${IPV6_NET32_UNUSED}::/$mask dev $tst_vti
	else
		ip_loc_tun="${IPV4_NET16_UNUSED}.1.1";
		ip_rmt_tun="${IPV4_NET16_UNUSED}.1.2";
		mask=30
		ROD ip route add ${IPV4_NET16_UNUSED}.1.0/$mask dev $tst_vti
	fi

	tst_resm TINFO "Add IPs to vti tunnel, " \
		       "loc: $ip_loc_tun/$mask, rmt: $ip_rmt_tun/$mask"

	ROD ip a add $ip_loc_tun/$mask dev $tst_vti
	tst_rhost_run -s -c "ip a add $ip_rmt_tun/$mask dev $tst_vti"
}

do_test()
{
	tst_resm TINFO "Sending ICMP messages..."
	EXPECT_PASS tst_ping $tst_vti $ip_rmt_tun $IPSEC_SIZE_ARRAY
}

do_setup
do_test

tst_exit
