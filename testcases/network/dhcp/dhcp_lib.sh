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
# Author:       Alexey Kodanev alexey.kodanev@oracle.com

stop_dhcp()
{
	[ "$(pgrep -x $dhcp_name)" ] || return 0

	tst_resm TINFO "stopping $dhcp_name"
	local count=0
	while [ $count -le 10 ]; do
		pkill -x $dhcp_name
		[ "$(pgrep -x $dhcp_name)" ] || return 0
		tst_sleep 100ms
		count=$((count + 1))
	done

	pkill -9 -x $dhcp_name
	tst_sleep 100ms
	[ "$(pgrep -x $dhcp_name)" ] && return 1 || return 0
}

init()
{
	tst_require_root
	tst_check_cmds cat $dhcp_name awk ip pgrep pkill dhclient

	veth_loaded=
	lsmod | grep -q '^veth ' && veth_loaded=1

	tst_resm TINFO "create veth interfaces"
	ip li add $iface0 type veth peer name $iface1 || \
		tst_brkm TBROK "failed to add veth $iface0"

	veth_added=1
	ip li set up $iface0 || tst_brkm TBROK "failed to bring $iface0 up"
	ip li set up $iface1 || tst_brkm TBROK "failed to bring $iface1 up"

	tst_tmpdir

	stop_dhcp || tst_brkm TBROK "Failed to stop dhcp server"

	dhclient_lease="/var/lib/dhclient/dhclient${TST_IPV6}.leases"
	if [ -f $dhclient_lease ]; then
		tst_resm TINFO "backup dhclient${TST_IPV6}.leases"
		mv $dhclient_lease .
	fi

	tst_resm TINFO "add $ip_addr to $iface0"
	ip addr add $ip_addr dev $iface0 || \
		tst_brkm TBROK "failed to add ip address"
}

cleanup()
{
	stop_dhcp

	pkill -f "dhclient -$ipv $iface1"

	cleanup_dhcp

	# restore dhclient leases
	[ $dhclient_lease ] && rm -f $dhclient_lease
	[ -f "dhclient${TST_IPV6}.leases" ] && \
		mv dhclient${TST_IPV6}.leases $dhclient_lease

	[ $veth_added ] && ip li del $iface0

	if [ -z $veth_loaded ]; then
		lsmod | grep -q '^veth ' && rmmod veth
	fi

	tst_rmdir
}

test01()
{
	tst_resm TINFO "starting DHCPv${ipv} server on $iface0"

	start_dhcp$TST_IPV6

	sleep 1

	if [ "$(pgrep '$dhcp_name')" ]; then
		print_dhcp_log
		tst_brkm TBROK "Failed to start $dhcp_name"
	fi

	tst_resm TINFO "starting dhclient -${ipv} $iface1"
	dhclient -$ipv $iface1 || \
		tst_brkm TBROK "dhclient failed"

	# check that we get configured ip address
	ip addr show $iface1 | grep $ip_addr_check > /dev/null
	if [ $? -eq 0 ]; then
		tst_resm TPASS "'$ip_addr_check' configured by DHCPv$ipv"
	else
		tst_resm TFAIL "'$ip_addr_check' not configured by DHCPv$ipv"
		print_dhcp_log
	fi

	stop_dhcp
}

iface0="ltp_veth0"
iface1="ltp_veth1"
ipv=${TST_IPV6:-"4"}

if [ $TST_IPV6 ]; then
	ip_addr="fd00:1:1:2::12/64"
	ip_addr_check="fd00:1:1:2::100/64"
else
	ip_addr="10.1.1.12/24"
	ip_addr_check="10.1.1.100/24"
fi

trap "tst_brkm TBROK 'test interrupted'" INT
