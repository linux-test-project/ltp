#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2014-2018 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
# Copyright (c) International Business Machines  Corp., 2001
#
# Author:       Manoj Iyer, manjo@mail.utexas.edu
# Author:       Alexey Kodanev alexey.kodanev@oracle.com

TST_SETUP="setup_dhcp"

dhcp_name="dhcpd"
lease_dir="/var/lib/misc"
lease_file="$lease_dir/dhcpd.leases_tst"

setup_dhcp()
{
	[ "$TST_IPV6" ] && lease="$lease_dir/dhcpd6.leases_tst"
	dhcp_lib_setup
}

setup_dhcpd_conf()
{
	if [ -f /etc/dhcpd.conf ]; then
		DHCPD_CONF="/etc/dhcpd.conf"
	elif [ -f /etc/dhcp/dhcpd.conf ]; then
		DHCPD_CONF="/etc/dhcp/dhcpd.conf"
	else
		tst_brk TBROK "failed to find dhcpd.conf"
	fi

	mv $DHCPD_CONF dhcpd.conf
	[ $? -ne 0 ] && tst_brk TBROK "failed to backup dhcpd.conf"

	mv tst_dhcpd.conf $DHCPD_CONF
	[ $? -ne 0 ] && tst_brk TBROK "failed to create dhcpd.conf"
}

start_dhcpd()
{
	touch $lease_file
	dhcpd -lf $lease_file -$TST_IPVER $iface0 > $log 2>&1
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
	rm -f $lease_file
}

print_dhcp_version()
{
	dhcpd --version 2>&1
}

. dhcp_lib.sh
tst_run
