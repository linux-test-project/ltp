#!/bin/sh
# Copyright (c) 2014-2015 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2001
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
# Author:       Manoj Iyer, manjo@mail.utexas.edu
# Author:       Alexey Kodanev alexey.kodanev@oracle.com

TST_CLEANUP=cleanup
TST_TOTAL=1
TCID="dhcpd"
dhcp_name="dhcpd"

. test_net.sh
. dhcp_lib.sh

setup_dhcpd_conf()
{
	if [ -f /etc/dhcpd.conf ]; then
		DHCPD_CONF="/etc/dhcpd.conf"
	elif [ -f /etc/dhcp/dhcpd.conf ]; then
		DHCPD_CONF="/etc/dhcp/dhcpd.conf"
	else
		tst_brkm TBROK "failed to find dhcpd.conf"
	fi

	mv $DHCPD_CONF dhcpd.conf
	[ $? -ne 0 ] && tst_brkm TBROK "failed to backup dhcpd.conf"

	mv tst_dhcpd.conf $DHCPD_CONF
	[ $? -ne 0 ] && tst_brkm TBROK "failed to create dhcpd.conf"
}

start_dhcpd()
{
	dhcpd -$ipv $iface0 > tst_dhcpd.err 2>&1
	if [ $? -ne 0 ]; then
		cat tst_dhcpd.err
		tst_brkm TBROK "Failed to start dhcpd"
	fi

}

start_dhcp()
{
	cat > tst_dhcpd.conf <<-EOF
	ddns-update-style none;
	update-static-leases off;
	subnet 10.1.1.0 netmask 255.255.255.0 {
		range 10.1.1.100 10.1.1.100;
		default-lease-time 60;
		max-lease-time 60;
	}
	EOF
	setup_dhcpd_conf
	start_dhcpd
}

start_dhcp6()
{
	cat > tst_dhcpd.conf <<-EOF
	ddns-update-style none;
	update-static-leases off;
	subnet6 fd00:1:1:2::/64 {
		range6 fd00:1:1:2::100 fd00:1:1:2::100;
		default-lease-time 60;
		max-lease-time 60;
	}
	EOF
	setup_dhcpd_conf
	start_dhcpd
}

cleanup_dhcp()
{
	[ -f dhcpd.conf ] && mv dhcpd.conf $DHCPD_CONF
}

print_dhcp_log()
{
	cat tst_dhcpd.err
}

init
test01
tst_exit
