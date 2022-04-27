#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2006 International Business Machines  Corp.
# Copyright (c) 2020 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Copyright (c) 2022 Petr Vorel <pvorel@suse.cz>
# Author: Mitsuru Chinen <mitch@jp.ibm.com>
#
# Verify the kernel is not crashed when the route is modified by
# ICMP Redirects frequently

TST_SETUP=setup
TST_CLEANUP=cleanup
TST_TESTFUNC=do_test
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="ip"

DST_HOST=
DST_PORT="7"

setup()
{
	local lhost_ifname=$(tst_iface lhost)
	local rhost_ifname=$(tst_iface rhost)
	local rhost_net="$(tst_ipaddr_un -p 1)"

	DST_HOST="$(tst_ipaddr_un 1 5)"

	# Remove the link-local address of the remote host
	tst_rhost_run -s -c "ip addr flush dev $rhost_ifname"

	# Add route to the initial gateway
	ip route add $rhost_net dev $lhost_ifname

	# Make sure the sysctl value is set for accepting the redirect
	sysctl -w net.ipv${TST_IPVER}.conf.${lhost_ifname}.accept_redirects=1 > /dev/null
	[ ! "$TST_IPV6" ] && sysctl -w net.ipv4.conf.${lhost_ifname}.secure_redirects=0 > /dev/null

	tst_rhost_run -s -c "ns-icmp_redirector -I $rhost_ifname -b"
}

cleanup()
{
	tst_rhost_run -c "killall -SIGHUP ns-icmp_redirector"
	route_cleanup
}

do_test()
{
	local cnt=0

	tst_res TINFO "modify route by ICMP redirects $NS_TIMES times"

	while [ $cnt -lt $NS_TIMES ]; do
		ROD ns-udpsender -f $TST_IPVER -D $DST_HOST -p $DST_PORT -o -s 8
		cnt=$((cnt+1))
	done

	tst_res TPASS "test finished successfully"
}

. route-lib.sh
tst_run
