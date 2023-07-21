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

NCHARS=${NCHARS:-64}
NLINES=${NLINES:-16384}

generate_file()
{
	local file="$1"
	local nchars="$2"
	local nlines="$3"
	local val="$4"
	local exp_size="$((nchars*nlines))"
	local size

	ROD nfs_flock_dgen $file $nchars $nlines $val

	size="$(wc -c $file | awk '{print $1}')"
	if [ $size -ne $exp_size ]; then
		tst_brk TBROK "could not create '$file' (size: $size, expected: $exp_size)"
	fi
}

do_setup()
{
	if ! tst_is_int $NCHARS || [ $NCHARS -lt 1 ]; then
		tst_res TBROK "NCHARS must be > 0"
	fi

	if ! tst_is_int $NLINES || [ $NLINES -lt 1 ]; then
		tst_res TBROK "NLINES must be > 0"
	fi

	nfs_setup

	tst_res TINFO "creating test files (chars: $NCHARS, lines: $NLINES)"

	generate_file flock_data $NCHARS $NLINES 0
	generate_file flock_odata $NCHARS $NLINES 1
}

do_test()
{
	tst_res TINFO "Testing locking"

	ROD cp flock_data flock_idata

	tst_res TINFO "locking 'flock_idata' file and writing data"

	nfs_flock 0 flock_idata $NCHARS $NLINES &
	local pids=$!
	nfs_flock 1 flock_idata $NCHARS $NLINES &
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
