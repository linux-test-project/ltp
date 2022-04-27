#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2015-2018 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2003
#
#  PURPOSE: Creates a text file of specified size locally and copies
#           the file to an NFS mountpoint.  The two files are compared
#           and checked for differences.  If the files differ, then
#           the test fails.  By default, this test creates a 10Mb file
#           and runs for one loop.
#
# Created by: Robbie Williamson (robbiew@us.ibm.com)

TST_TESTFUNC="do_test"

do_test()
{
    tst_res TINFO "create 10M file"
    ROD nfs04_create_file 10 nfs04.testfile
    tst_res TPASS "Test finished"
}

. nfs_lib.sh
tst_run
