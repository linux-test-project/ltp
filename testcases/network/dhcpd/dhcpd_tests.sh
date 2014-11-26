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

. test_net.sh

stop_dhcp()
{
	if [ "$(pgrep -x dhcpd)" ]; then
		tst_resm TINFO "stopping DHCP server"
		pkill -x dhcpd
		sleep 1
	fi
}

setup_conf()
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
}

setup_conf6()
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
}

init()
{
	tst_require_root
	tst_check_cmds cat dhcpd awk ip pgrep pkill dhclient

	dummy_loaded=
	lsmod | grep -q '^dummy '
	if [ $? -eq 0 ]; then
		dummy_loaded=1
	else
		modprobe dummy || tst_brkm TCONF "failed to find dummy module"
	fi

	tst_resm TINFO "create dummy interface"
	ip li add $iface type dummy || \
		tst_brkm TBROK "failed to add dummy $iface"

	ip li set up $iface || tst_brkm TBROK "failed to bring $iface up"

	tst_tmpdir

	stop_dhcp

	setup_conf$TST_IPV6

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

	dhclient_lease="/var/lib/dhclient/dhclient${TST_IPV6}.leases"
	if [ -f $dhclient_lease ]; then
		tst_resm TINFO "backup dhclient${TST_IPV6}.leases"
		mv $dhclient_lease .
	fi

	tst_resm TINFO "add $ip_addr to $iface to create private network"
	ip addr add $ip_addr dev $iface || \
		tst_brkm TBROK "failed to add ip address"
}

cleanup()
{
	stop_dhcp

	pkill -f "dhclient -$ipv $iface"

	[ -f dhcpd.conf ] && mv dhcpd.conf $DHCPD_CONF

	# restore dhclient leases
	rm -f $dhclient_lease
	[ -f "dhclient${TST_IPV6}.leases" ] && \
		mv dhclient${TST_IPV6}.leases $dhclient_lease

	if [ "$dummy_loaded" ]; then
		ip li del $iface
	else
		rmmod dummy
	fi

	tst_rmdir
}

test01()
{
	tst_resm TINFO "starting DHCPv${ipv} server on $iface"
	dhcpd -$ipv $iface > tst_dhcpd.err 2>&1
	if [ $? -ne 0 ]; then
		cat tst_dhcpd.err
		tst_brkm TBROK "Failed to start dhcpd"
	fi

	sleep 1

	if [ "$(pgrep 'dhcpd -$ipv $iface')" ]; then
		cat tst_dhcpd.err
		tst_brkm TBROK "Failed to start dhcpd"
	fi

	tst_resm TINFO "starting dhclient -${ipv} $iface"
	dhclient -$ipv $iface || \
		tst_brkm TBROK "dhclient failed"

	# check that we get configured ip address
	ip addr show $iface | grep $ip_addr_check > /dev/null
	if [ $? -eq 0 ]; then
		tst_resm TPASS "'$ip_addr_check' configured by DHCPv$ipv"
	else
		tst_resm TFAIL "'$ip_addr_check' not configured by DHCPv$ipv"
	fi

	stop_dhcp
}

iface="ltp_dummy"
ipv=${TST_IPV6:-"4"}

if [ $TST_IPV6 ]; then
	ip_addr="fd00:1:1:2::12/64"
	ip_addr_check="fd00:1:1:2::100/64"
else
	ip_addr="10.1.1.12/24"
	ip_addr_check="10.1.1.100/24"
fi

trap "tst_brkm TBROK 'test interrupted'" INT

init
test01
tst_exit
