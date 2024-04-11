#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2014-2018 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
#
# Author: Alexey Kodanev alexey.kodanev@oracle.com

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
		tst_brk TCONF "dnsmasq doesn't support DHCPv6"

	dnsmasq $common_opt \
	        --dhcp-range=fd00:1:1:2::100,fd00:1:1:2::100 --enable-ra \
	        --dhcp-option=option6:dns-server
}

cleanup_dhcp()
{
	rm -f $log
}

print_dhcp_version()
{
	dnsmasq --version | head -2
}

lease_dir="/var/lib/misc"

dhcp_name="dnsmasq"
log="/var/log/dnsmasq.tst.log"

lease_file="$lease_dir/dnsmasq.tst.leases"

common_opt="--no-hosts --no-resolv --dhcp-authoritative \
	--log-facility=$log --interface=$iface0 \
	--dhcp-leasefile=$lease_file --port=0 --conf-file= "

. dhcp_lib.sh
tst_selinux_enforced && lease_dir="/var/lib/dnsmasq"
tst_run
