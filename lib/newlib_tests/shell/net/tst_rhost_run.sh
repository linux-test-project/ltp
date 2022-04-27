#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>

TST_TESTFUNC=do_test
PATH="$(dirname $0)/../../../../testcases/lib/:$PATH"

export TST_NET_RHOST_RUN_DEBUG=1

do_test()
{
	local file="/etc/fstab"

	tst_rhost_run -c 'which grep' > /dev/null || \
		tst_brk TCONF "grep not found on rhost"

	tst_rhost_run -c "[ -f $file ]" || \
		tst_brk TCONF "$file not found on rhost"

	tst_rhost_run -s -c "grep -q \"[^ ]\" $file"
	tst_rhost_run -s -c "grep -q '[^ ]' $file"

	tst_res TPASS "tst_rhost_run is working"
}

. tst_net.sh
tst_run
