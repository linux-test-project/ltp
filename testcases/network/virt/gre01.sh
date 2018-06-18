#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2015-2017 Oracle and/or its affiliates.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# It requires remote host. Test will setup IPv4 and IPv6 virtual
# tunnel between two hosts, then will compare TCP performance
# with and without GRE using ping or netstress test.

TST_NEEDS_TMPDIR=1
TST_TESTFUNC=virt_netperf_msg_sizes
TST_SETUP=do_setup
TST_CLEANUP=virt_cleanup
. virt_lib.sh

do_setup()
{
	virt_type="gre"
	[ "$TST_IPV6" ] && virt_type="ip6gre"
	virt_lib_setup

	if [ -z $ip_local -o -z $ip_remote ]; then
		tst_brk TBROK "you must specify IP address"
	fi

	tst_res TINFO "test $virt_type"
	virt_setup "local $(tst_ipaddr) remote $(tst_ipaddr rhost) dev $(tst_iface)" \
	"local $(tst_ipaddr rhost) remote $(tst_ipaddr) dev $(tst_iface rhost)"
}

tst_run
