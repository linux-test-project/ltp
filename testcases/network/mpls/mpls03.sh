#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Oracle and/or its affiliates. All Rights Reserved.

TST_SETUP="setup"
TST_TESTFUNC="mpls_virt_test"
TST_CLEANUP="mpls_virt_cleanup"

setup()
{
	virt_type="gre"
	if [ -n "$TST_IPV6" ]; then
		tst_kvcmp -lt "4.19" && \
			tst_brk TCONF "mpls + ip6gre requires kernel 4.19+"
		virt_type="ip6gre"
	fi

	mpls_virt_setup
}

. virt_lib.sh
. mpls_lib.sh
tst_run
