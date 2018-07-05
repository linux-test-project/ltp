#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

TST_TESTFUNC=do_test
TST_SETUP=do_setup
TST_CLEANUP=tst_ipsec_cleanup
. ipsec_lib.sh

do_setup()
{
	tst_ipsec_setup_vti
	PING_MAX="$IPSEC_REQUESTS"
	tst_res TINFO "Sending ICMP messages"
}

do_test()
{
	tst_ping $tst_vti $ip_rmt_tun $2
}

tst_run
