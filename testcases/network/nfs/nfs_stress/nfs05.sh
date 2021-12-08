#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2015-2018 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2001
#
#  DESCRIPTION: This script sets up the NFS directories in the remote machine
#               and invokes the program make_tree with parameters.
#
# Created by: Robbie Williamson (robbiew@us.ibm.com)

DIR_NUM=${DIR_NUM:-"10"}
FILE_NUM=${FILE_NUM:-"30"}
THREAD_NUM=${THREAD_NUM:-"8"}
TST_NEEDS_CMDS="make gcc"
TST_TESTFUNC="do_test"

. nfs_lib.sh

do_test()
{
    tst_res TINFO "start nfs05_make_tree -d $DIR_NUM -f $FILE_NUM -t $THREAD_NUM"
    ROD nfs05_make_tree -d $DIR_NUM -f $FILE_NUM -t $THREAD_NUM

    tst_res TPASS "test finished"
}

tst_run
