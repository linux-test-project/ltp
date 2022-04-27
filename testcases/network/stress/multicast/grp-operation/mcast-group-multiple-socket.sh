#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2017-2018 Petr Vorel <pvorel@suse.cz>
# Copyright (c) International Business Machines  Corp., 2006
# Author: Mitsuru Chinen <mitch@jp.ibm.com>

TST_NEEDS_ROOT=1
TST_NEEDS_TMPDIR=1
TST_SETUP="do_setup"
TST_CLEANUP="mcast_cleanup"
TST_TESTFUNC="do_test"

do_setup()
{
	# Increase the maximum number of open file descriptors
	if [ $(ulimit -n) -lt $MCASTNUM_HEAVY ]; then
		ulimit -n $MCASTNUM_HEAVY || tst_brk TCONF \
			"Failed to set the maximum number of open file descriptors to $MCASTNUM_HEAVY"
	fi

	mcast_setup $MCASTNUM_HEAVY
}

do_test()
{
	tst_res TINFO "joining $MCASTNUM_HEAVY IPv$TST_IPVER multicast groups on multiple sockets"
	do_multicast_test_multiple_join $MCASTNUM_HEAVY true
}

. mcast-lib.sh
tst_run
