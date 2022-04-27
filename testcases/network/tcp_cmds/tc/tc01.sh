#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2021 FUJITSU LIMITED. All rights reserved.
# Author: Yang Xu<xuyang2018.jy@fujitsu.com>
#
# When using "tc qdisc add dev teql0 root teql0 command", qdisc_create()
# calls teql_qdisc_init() it imediately fails after check "if (m->dev == dev)"
# because both devices are teql0, and it does not set qdisc_priv(sch)->m
# leaving it zero on error path, then qdisc_create() imediately calls
# teql_destroy() which does not expect zero master pointer and we get OOPS
# on unpatched kernel.
#
# If we enable panic_on_oops, this case may crash.
#
# This kernel bug was introduced by
# commit 87b60cfacf9f ("net_sched: fix error recovery at qdisc creation")
# and has been fixed by
# commit 1ffbc7ea9160 ("net: sched: sch_teql: fix null-pointer dereference")
#

TST_SETUP="setup"
TST_TESTFUNC="do_test"
TST_NEEDS_ROOT=1
TST_NEEDS_DRIVERS="sch_teql"
TST_NEEDS_CMDS="tc modprobe dmesg grep"

setup()
{
	ROD modprobe $TST_NEEDS_DRIVERS
}

do_test()
{
	tst_res TINFO "Use tc qdisc command to trigger a null-pointer dereference"

	EXPECT_FAIL tc qdisc add dev teql0 root teql0

	if dmesg | grep -q 'RIP:.*sch_teql'; then
		tst_res TFAIL "This bug is reproduced."
	else
		tst_res TPASS "This bug is not reproduced."
	fi
}

. tst_test.sh
tst_run
