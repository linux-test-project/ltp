#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018-2022 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2017 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

TST_NEEDS_TMPDIR=1
TST_TESTFUNC=do_test
TST_SETUP=tst_ipsec_setup
TST_CLEANUP=tst_ipsec_cleanup

do_test()
{
	local opts="-n $2 -N $2"
	local rand_opt="$(echo $2 | cut -d'R' -f2)"

	[ "$2" != "$rand_opt" ] && opts="-A $rand_opt"
	tst_netload -H $(tst_ipaddr rhost) -T sctp $opts \
		-r $IPSEC_REQUESTS -S $(tst_ipaddr)
}

. ipsec_lib.sh
tst_run
