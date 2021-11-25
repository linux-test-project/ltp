#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2015-2018 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2001
#
#  PURPOSE: Stresses NFS by opening a large number of files on a nfs
#           mounted filesystem.
#
# Ported by Robbie Williamson (robbiew@us.ibm.com)

TST_TESTFUNC="do_test"

. nfs_lib.sh

do_test()
{
	tst_res TINFO "starting 'nfs01_open_files $NFILES'"
	ROD nfs01_open_files $NFILES
	tst_res TPASS "test finished successfully"
}

tst_run
