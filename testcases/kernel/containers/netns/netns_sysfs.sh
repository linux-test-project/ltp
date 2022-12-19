#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Köry Maincent <kory.maincent@bootlin.com> 2020
# Copyright (c) 2015 Red Hat, Inc.
#
# Tests that a separate network namespace cannot affect sysfs contents
# of the main namespace.

TST_NEEDS_DRIVERS="dummy"
TST_CLEANUP=do_cleanup
TST_SETUP=do_setup
TST_TESTFUNC=do_test
TST_NEEDS_TMPDIR=1

do_setup()
{
	NS_TYPE="net,mnt"
	DUMMYDEV_HOST="dummy_test0"
	DUMMYDEV="dummy_test1"

	setns_check
	if [ $? -eq 32 ]; then
		tst_brk TCONF "setns not supported"
	fi

	NS_HANDLE=$(ns_create $NS_TYPE)
	if [ $? -eq 1 ]; then
		tst_res TINFO "$NS_HANDLE"
		tst_brk TBROK "unable to create a new network namespace"
	fi

	ip link add $DUMMYDEV_HOST type dummy || \
		tst_brk TBROK "failed to add a new (host) dummy device"

	ns_exec $NS_HANDLE $NS_TYPE mount --make-rprivate /sys
	ns_exec $NS_HANDLE $NS_TYPE ip link add $DUMMYDEV type dummy || \
		tst_brk TBROK "failed to add a new dummy device"
	ns_exec $NS_HANDLE $NS_TYPE mount -t sysfs none /sys 2>/dev/null
}

do_cleanup()
{
	ip link del $DUMMYDEV_HOST 2>/dev/null
	ip link del $DUMMYDEV 2>/dev/null
	kill -9 $NS_HANDLE 2>/dev/null
}

do_test()
{
	EXPECT_PASS ns_exec $NS_HANDLE $NS_TYPE test -e /sys/class/net/$DUMMYDEV
	EXPECT_FAIL ns_exec $NS_HANDLE $NS_TYPE test -e /sys/class/net/$DUMMYDEV_HOST
	EXPECT_FAIL test -e /sys/class/net/$DUMMYDEV
}

. tst_test.sh
tst_run
