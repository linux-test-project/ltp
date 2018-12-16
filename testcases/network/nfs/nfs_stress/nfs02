#! /bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2015-2018 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2001
#
# PURPOSE: Tests NFS copy of various filesizes, file consistency
#          between copies and preservation of write/nowrite permissions.
#
# Ported by: Robbie Williamson (robbiew@us.ibm.com)

TST_CNT=3
TST_TESTFUNC="do_test"
LTP_DATAFILES="$LTPROOT/testcases/bin/datafiles"

. nfs_lib.sh

do_test1()
{
	tst_res TINFO "do_test1 $TC"
	ROD cp $LTP_DATAFILES/ascii.jmb .
	tst_res TINFO "compare both ascii.jmbs"
	ROD diff $LTP_DATAFILES/ascii.jmb ascii.jmb

	tst_res TPASS "test1 passed"
}

do_test2()
{
	tst_res TINFO "do_test2, copy data files"
	local files="ascii.sm ascii.med ascii.lg"

	for f in $files; do
		tst_res TINFO "copy '$f' file"
		ROD cp $LTP_DATAFILES/$f .
		ROD cp $f ${f}cp
		ROD diff $LTP_DATAFILES/$f ${f}cp
	done

	tst_res TPASS "test2 passed"
}

do_test3()
{
	tst_res TINFO "do_test3, test permissions"
	ROD chmod a-wx ascii.sm
	ROD ls -l ascii.sm | grep -q "r--"
	ROD chmod a+w ascii.sm
	tst_res TPASS "test3 passed"
}

tst_run
