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
# It requires remote host. Test will setup IPv4 and IPv6 virtual
# tunnel between two hosts, then will compare TCP performance
# with and without GRE using ping or netstress test.

TCID=gre01
TST_TOTAL=1

. test_net.sh

virt_type="gre"
[ "$TST_IPV6" ] && virt_type="ip6gre"

. virt_lib.sh

cleanup()
{
	cleanup_vifaces
	tst_rhost_run -c "ip link delete ltp_v0 2>/dev/null"
}
TST_CLEANUP="cleanup"

if [ -z $ip_local -o -z $ip_remote ]; then
	tst_brkm TBROK "you must specify IP address"
fi

tst_resm TINFO "test $virt_type"
virt_setup "local $(tst_ipaddr) remote $(tst_ipaddr rhost) dev $(tst_iface)" \
"local $(tst_ipaddr rhost) remote $(tst_ipaddr) dev $(tst_iface rhost)"

virt_compare_netperf

tst_exit
