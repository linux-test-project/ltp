#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Linux Test Project, 2002-2023
# Copyright (c) 2016-2018 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2001
#
#  PURPOSE:
#           Two processes open FLOCK_IDATA file simultaneously
#           each one locks odd and even lines of the file simultaneously
#           and fill them with '0's and '1's. After they find eof, the
#           datafiles are compared.

TST_SETUP="do_setup"
TST_TESTFUNC="do_test"

do_setup()
{
	local nchars=64
	local nlines=16384
	local exp_size="$((nchars*nlines))"

	nfs_setup

	tst_res TINFO "creating test files"
	ROD nfs_flock_dgen flock_data $nchars $nlines 0
	ROD nfs_flock_dgen flock_odata $nchars $nlines 1

	[ "$(wc -c flock_data | awk '{print $1}')" -ne $exp_size ] && \
		tst_brk TBROK "could not create 'flock_data'"

	[ "$(wc -c flock_odata | awk '{print $1}')" -ne $exp_size ] && \
		tst_brk TBROK "could not create 'flock_odata'"
}

do_test()
{
	tst_res TINFO "Testing locking"

	ROD cp flock_data flock_idata

	tst_res TINFO "locking 'flock_idata' file and writing data"

	nfs_flock 0 flock_idata &
	local pids=$!
	nfs_flock 1 flock_idata &
	pids="$pids $!"

	tst_res TINFO "waiting for pids: $pids"
	for p in $pids; do
		wait $p
		if [ $? -ne 0 ]; then
			tst_brk TFAIL "nfs_lock process failed"
		else
			tst_res TINFO "$p completed"
		fi
	done

	diff flock_odata flock_idata
	if [ $? -ne 0 ]; then
		tst_res TFAIL "content is different"
	else
		tst_res TPASS "content is the same"
	fi
}

. nfs_lib.sh
tst_run
