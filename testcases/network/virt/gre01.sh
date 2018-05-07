#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2015-2017 Oracle and/or its affiliates.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# It requires remote host. Test will setup IPv4 and IPv6 virtual
# tunnel between two hosts, then will compare TCP performance
# with and without GRE using ping or netstress test.

TCID=gre01
TST_TOTAL=1
TST_NEEDS_TMPDIR=1

virt_type="gre"
. virt_lib.sh

TST_CLEANUP="virt_cleanup"

if [ -z $ip_local -o -z $ip_remote ]; then
	tst_brkm TBROK "you must specify IP address"
fi

tst_resm TINFO "test $virt_type"
virt_setup "local $(tst_ipaddr) remote $(tst_ipaddr rhost) dev $(tst_iface)" \
"local $(tst_ipaddr rhost) remote $(tst_ipaddr) dev $(tst_iface rhost)"

virt_netperf_msg_sizes

tst_exit
