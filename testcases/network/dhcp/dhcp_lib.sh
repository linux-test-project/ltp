#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2014-2018 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) 2018-2022 Petr Vorel <pvorel@suse.cz>
# Author:       Alexey Kodanev alexey.kodanev@oracle.com

TST_SETUP="${TST_SETUP:-dhcp_lib_setup}"
TST_CLEANUP="${TST_CLEANUP:-dhcp_lib_cleanup}"
TST_TESTFUNC="test01"
TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="cat $dhcp_name awk ip pgrep pkill dhclient"

. tst_net.sh
. daemonlib.sh

iface0="ltp_veth0"
iface1="ltp_veth1"

stop_dhcp()
{
	[ "$(pgrep -x $dhcp_name)" ] || return 0

	tst_res TINFO "stopping $dhcp_name"
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

dhcp_lib_setup()
{
	if [ $HAVE_SYSTEMCTL -eq 1 ] && \
		systemctl --no-pager -p Id show network.service | grep -q Id=wicked.service; then
		[ $TST_IPV6 ] && tst_brk TCONF "wicked not supported on IPv6"
		is_wicked=1
	fi

	[ -z "$log" ] && log="$PWD/$(basename $0 '.sh').log"

	if [ $TST_IPV6 ]; then
		ip_addr="fd00:1:1:2::12/64"
		ip_addr_check_noprefix="fd00:1:1:2::100"
		ip_addr_check="$ip_addr_check_noprefix/128"
	else
		ip_addr="10.1.1.12/24"
		ip_addr_check_noprefix="10.1.1.100"
		ip_addr_check="$ip_addr_check_noprefix/24"
	fi

	lsmod | grep -q '^veth ' && veth_loaded=yes || veth_loaded=no

	tst_res TINFO "create veth interfaces"
	ip link add $iface0 type veth peer name $iface1 || \
		tst_brk TBROK "failed to add veth $iface0"

	veth_added=1
	ip link set up $iface0 || tst_brk TBROK "failed to bring $iface0 up"
	ip link set up $iface1 || tst_brk TBROK "failed to bring $iface1 up"

	stop_dhcp || tst_brk TBROK "Failed to stop dhcp server"

	dhclient_lease="/var/lib/dhclient/dhclient${TST_IPV6}.leases"
	[ -f $dhclient_lease ] || dhclient_lease="/var/lib/dhcp/dhclient${TST_IPV6}.leases"
	if [ -f $dhclient_lease ]; then
		tst_res TINFO "backup dhclient${TST_IPV6}.leases"
		mv $dhclient_lease .
	fi

	tst_res TINFO "add $ip_addr to $iface0"
	ip addr add $ip_addr dev $iface0 || \
		tst_brk TBROK "failed to add ip address"

	if [ ! -d "$lease_dir" ]; then
		mkdir -p $lease_dir
		lease_dir_added=1
	fi
}

dhcp_lib_cleanup()
{
	[ -z "$veth_loaded" ] && return

	[ "$lease_dir_added" = 1 ] && rm -rf $lease_dir
	rm -f $lease_file

	stop_dhcp

	pkill -f "dhclient -$TST_IPVER $iface1"

	cleanup_dhcp

	# restore dhclient leases
	[ $dhclient_lease ] && rm -f $dhclient_lease
	[ -f "dhclient${TST_IPV6}.leases" ] && \
		mv dhclient${TST_IPV6}.leases $dhclient_lease

	[ $veth_added ] && ip link del $iface0

	[ "$veth_loaded" = "no" ] && lsmod | grep -q '^veth ' && rmmod veth
}

print_dhcp_log()
{
	[ -f "$log" ] && cat $log
}

test01()
{
	local wicked_cfg="/etc/sysconfig/network/ifcfg-$iface1"
	local wicked_cleanup

	tst_res TINFO "testing DHCP server $dhcp_name: $(print_dhcp_version)"
	tst_res TINFO "using DHCP client: $(dhclient --version 2>&1)"

	tst_res TINFO "starting DHCPv$TST_IPVER server on $iface0"

	start_dhcp$TST_IPV6
	if [ $? -ne 0 ]; then
		print_dhcp_log
		tst_brk TBROK "Failed to start $dhcp_name"
	fi

	sleep 1

	if [ "$(pgrep '$dhcp_name')" ]; then
		print_dhcp_log
		tst_brk TBROK "Failed to start $dhcp_name"
	fi

	if [ "$is_wicked" ]; then
		tst_res TINFO "wicked is running, don't start dhclient"
		if [ ! -f "$wicked_cfg" ]; then
			cat <<EOF > $wicked_cfg
BOOTPROTO='dhcp'
NAME='LTP card'
STARTMODE='auto'
USERCONTROL='no'
EOF
			wicked_cleanup=1
		else
			tst_res TINFO "wicked config file $wicked_cfg already exist"
		fi

		tst_res TINFO "restarting wicked"
		systemctl restart wicked
	else
		tst_res TINFO "starting dhclient -$TST_IPVER $iface1"
		dhclient -$TST_IPVER $iface1 || tst_brk TBROK "dhclient failed"
	fi

	# check that we get configured ip address
	ip addr show $iface1 | grep -q $ip_addr_check
	if [ $? -eq 0 ]; then
		tst_res TPASS "'$ip_addr_check' configured by DHCPv$TST_IPVER"
	else
		if ip addr show $iface1 | grep -q $ip_addr_check_noprefix; then
			tst_res TFAIL "'$ip_addr_check_noprefix' configured but has wrong prefix, expect '$ip_addr_check'"
			ip addr show $iface1
		else
			tst_res TFAIL "'$ip_addr_check' not configured by DHCPv$TST_IPVER"
			print_dhcp_log
		fi
	fi

	[ "$wicked_cleanup" ] && rm -f $wicked_cfg

	stop_dhcp
}
