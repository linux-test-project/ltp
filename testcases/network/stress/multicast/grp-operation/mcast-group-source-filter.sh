#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2017-2018 Petr Vorel <pvorel@suse.cz>
# Copyright (c) International Business Machines  Corp., 2006
# Author: Mitsuru Chinen <mitch@jp.ibm.com>

TST_NEEDS_ROOT=1
TST_NEEDS_TMPDIR=1
TST_SETUP="mcast_setup_normal"
TST_CLEANUP="mcast_cleanup"
TST_TESTFUNC="do_test"

do_test()
{
	tst_res TINFO "joining and leaving the same IPv$TST_IPVER multicast group with a different source filters on $MCASTNUM_NORMAL sockets in $NS_TIMES times"
	do_multicast_test_join_leave $MCASTNUM_NORMAL true
}

. mcast-lib.sh
tst_run
