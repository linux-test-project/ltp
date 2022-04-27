#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2016-2018 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2001
#
#  PURPOSE: Runs the fsx-linux tool with a 50000 iterations setting to
#	    attempt to uncover the "doread:read input/output" error
#	    received if the latest NFS patches for 2.4.17 from Trond
#	    are not applied. http://nfs.sf.net

TST_TESTFUNC="do_test"

do_test()
{
	ITERATIONS=${ITERATIONS:=50000}
	tst_res TINFO "starting fsx-linux -N $ITERATIONS..."
	fsx-linux -N $ITERATIONS testfile > fsx-out.log 2>&1
	if [ "$?" -ne 0 ]; then
		tst_res TFAIL "Errors have resulted from this test"
		cat fsx-out.log
	else
		tst_res TPASS "fsx-linux test passed"
	fi
}

. nfs_lib.sh
tst_run
