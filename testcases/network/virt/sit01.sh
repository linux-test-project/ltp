#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Oracle and/or its affiliates.

TST_NEEDS_TMPDIR=1
TST_TESTFUNC=virt_netperf_msg_sizes
TST_SETUP=do_setup
TST_CLEANUP=virt_cleanup
virt_type="sit"
. virt_lib.sh

do_setup()
{
	[ -n "$TST_IPV6" ] && tst_res TBROK "invalid option '-6' for sit tunnel"

	virt_lib_setup

	tst_res TINFO "test $virt_type"
	virt_setup "local $(tst_ipaddr) remote $(tst_ipaddr rhost)" \
		   "local $(tst_ipaddr rhost) remote $(tst_ipaddr)"
}

tst_run
