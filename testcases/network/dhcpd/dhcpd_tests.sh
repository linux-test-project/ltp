#!/bin/sh
# Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
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

TST_CLEANUP=cleanup
TST_TOTAL=1
TCID="dhcpd"

. test_net.sh

stop_dhcp()
{
	if [ "$(pgrep -x dhcpd)" ]; then
		tst_resm TINFO "stopping DHCP server"
		pkill -x dhcpd || tst_brkm "failed to stop DHCP server"
	fi
}

# alias ethX to ethX:1 with IP 10.1.1.12
init()
{
	tst_require_root
	tst_tmpdir
	tst_check_cmds cat dhcpd awk ip pgrep pkill diff

	stop_dhcp

	cat > tst_dhcpd.conf <<-EOF
	subnet 10.1.1.0 netmask 255.255.255.0 {
        # default gateway
		range 10.1.1.12 10.1.1.12;
		default-lease-time 600;
		max-lease-time 1200;
		option routers 10.1.1.1;
		option subnet-mask
		255.255.255.0;
		option
		domain-name-servers
		10.1.1.1;
		option
		domain-name
		"dhcptest.net";
	}
	ddns-update-style interim;
	EOF

	if [ $? -ne 0 ]; then
		tst_brkm TBROK "unable to create temp file: tst_dhcpd.conf"
	fi

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

	tst_resm TINFO "add $iface:1 10.1.1.12/24 to create private network"
	ip addr add 10.1.1.12/24 dev $iface label $iface:1
	if [ $? -ne 0 ]; then
		tst_brkm TBROK "failed to add alias"
	fi
}

cleanup()
{
	stop_dhcp
	[ -f dhcpd.conf ] && mv dhcpd.conf $DHCPD_CONF

	ip addr show $iface | grep "$iface:1" > /dev/null &&
		ip addr del 10.1.1.12/24 dev $iface label $iface:1

	tst_rmdir
}

test01()
{
	tst_resm TINFO "start/stop DHCP server"

	cat > tst_dhcpd.exp <<-EOF
	Sending on   Socket/fallback/fallback-net
	EOF
	[ $? -ne 0 ] && tst_brkm TBROK "unable to create expected results"

	tst_resm TINFO "starting DHCP server"
	dhcpd > tst_dhcpd.err 2>&1
	if [ $? -ne 0 ]; then
		cat tst_dhcpd.err
		tst_brkm TBROK "Failed to start dhcpd"
	fi

	cat tst_dhcpd.err | tail -n 1 > tst_dhcpd.out
	[ $? -ne 0 ] && tst_brkm TBROK "unable to create output file"

	diff -iwB tst_dhcpd.out tst_dhcpd.exp
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to start dhcpd"
		return
	fi

	stop_dhcp

	tst_resm TPASS "dhcpd started and stopped successfully"
}

iface=$(tst_iface)

init

test01

tst_exit
