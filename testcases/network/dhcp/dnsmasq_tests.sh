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
# along with this program; if not, write the Free Software Foundation,
# Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# Author: Alexey Kodanev alexey.kodanev@oracle.com

TST_CLEANUP=cleanup
TST_TOTAL=1
TCID="dnsmasq"
dhcp_name="dnsmasq"

. test_net.sh
. dhcp_lib.sh

start_dhcp()
{
	dnsmasq $common_opt \
	        --dhcp-range=10.1.1.100,10.1.1.100,255.255.255.0,2m \
	        --dhcp-option=option:router --dhcp-option=option:dns-server
}

start_dhcp6()
{
	# check that dnsmasq supports IPv6
	dnsmasq --dhcp-range=fd00::1,fd00::1 --test > /dev/null 2>&1 || \
		tst_brkm TCONF "dnsmasq doesn't support DHCPv6"

	dnsmasq $common_opt \
	        --dhcp-range=fd00:1:1:2::100,fd00:1:1:2::100 --enable-ra \
	        --dhcp-option=option6:dns-server
}

cleanup_dhcp()
{
	rm -f tst_dnsmasq.log
}

print_dhcp_log()
{
	cat tst_dnsmasq.log
}

init

common_opt="--no-hosts --no-resolv --dhcp-authoritative \
	--log-facility=$(pwd)/tst_dnsmasq.log --interface=$iface0 \
	--dhcp-leasefile=$(pwd)/tst_dnsmasq.lease --conf-file= "

test01
tst_exit
