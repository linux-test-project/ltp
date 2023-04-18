#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2016-2018 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2003
#
#  PURPOSE: Runs fsstress over an NFS mount point for a specified amount
#           of time. The purpose of this test is to stress the NFS kernel
#           code and possibly the underlying filesystem where the export
#           resides.  A PASS is if the test completes.

TST_TESTFUNC="do_test"
TST_CLEANUP="do_cleanup"

THREAD_NUM="${THREAD_NUM:-2}"
OPERATION_NUM="${OPERATION_NUM:-1000}"

do_cleanup()
{
	[ -n "$pids" ] && kill -9 $pids
	nfs_cleanup
}

do_test()
{
	tst_res TINFO "Starting fsstress processes on NFS mounts"

	local n=0
	local pids
	for i in $VERSION; do
		fsstress -l 1 -d $TST_TMPDIR/$i/$n -n $OPERATION_NUM -p $THREAD_NUM -r -c > /dev/null &
		pids="$pids $!"
		n=$(( n + 1 ))
	done

	tst_res TINFO "waiting for pids:$pids"
	for p in $pids; do
		wait $p || tst_brk TFAIL "fsstress process failed"
		tst_res TINFO "fsstress '$p' completed"
	done
	pids=

	tst_res TPASS "all fsstress processes completed on '$n' NFS mounts"
}

. nfs_lib.sh
tst_run
