#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2017 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

TCID=sctp_ipsec
TST_TOTAL=3
TST_NEEDS_TMPDIR=1
TST_CLEANUP="tst_ipsec_cleanup"

. ipsec_lib.sh

do_test()
{
	for p in $IPSEC_SIZE_ARRAY; do
		tst_netload -H $(tst_ipaddr rhost) -T sctp -n $p -N $p \
			-r $IPSEC_REQUESTS -S $(tst_ipaddr)
	done
}

tst_ipsec_setup

do_test

tst_exit
