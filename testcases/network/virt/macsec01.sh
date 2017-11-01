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
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Test-case: setup two MACsec drivers and run TCP traffic over them
# with default MACsec configuration, compare performance with similar
# IPsec configuration on master interface.

TCID=macsec01
TST_TOTAL=16
TST_NEEDS_TMPDIR=1

virt_type="macsec"
VIRT_PERF_THRESHOLD=${VIRT_PERF_THRESHOLD:-100}

. ipsec_lib.sh
. virt_lib.sh

cleanup()
{
	virt_cleanup
	tst_ipsec_cleanup
}
TST_CLEANUP="cleanup"

IPSEC_MODE=transport
IPSEC_PROTO=ah
tst_resm TINFO "setup IPsec $IPSEC_MODE/$IPSEC_PROTO $EALGO"
tst_ipsec lhost $(tst_ipaddr) $(tst_ipaddr rhost)
tst_ipsec rhost $(tst_ipaddr rhost) $(tst_ipaddr)

virt_macsec_setup
virt_netperf_msg_sizes

tst_exit
