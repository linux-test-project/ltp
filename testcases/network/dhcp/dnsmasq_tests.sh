#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2014-2018 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
#
# Author: Alexey Kodanev alexey.kodanev@oracle.com

dhcp_name="dnsmasq"

. dhcp_lib.sh

common_opt="--no-hosts --no-resolv --dhcp-authoritative \
	--log-facility=./tst_dnsmasq.log --interface=$iface0 \
	--dhcp-leasefile=tst_dnsmasq.lease --port=0 --conf-file= "

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
	rm -f tst_dnsmasq.log
}

print_dhcp_log()
{
	cat tst_dnsmasq.log
}

print_dhcp_version()
{
	dnsmasq --version | head -2
}

tst_run
