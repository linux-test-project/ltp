#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2022 Akihiko Odaki <akihiko.odaki@daynix.com>
# Copyright (c) 2003 Manoj Iyer <manjo@mail.utexas.edu>
# Copyright (c) 2001 Robbie Williamson <robbiew@us.ibm.com>

TST_TESTFUNC=do_test
TST_CNT=4
TST_NEEDS_CMDS='awk ftp'
TST_NEEDS_TMPDIR=1

RUSER="${RUSER:-root}"
RHOST="${RHOST:-localhost}"

do_test()
{
    case $1 in
    1) test_get binary;;
    2) test_get ascii;;
    3) test_put binary;;
    4) test_put ascii;;
    esac
}

list_files()
{
    case $1 in
    ascii) echo 'ascii.sm ascii.med ascii.lg ascii.jmb';;
    binary) echo 'bin.sm bin.med bin.lg bin.jmb';;
    esac
}

test_get()
{
    local file sum1 sum2

    for file in $(list_files $1); do
        {
            echo user $RUSER $PASSWD
            echo $1
            echo cd $TST_NET_DATAROOT
            echo get $file
            echo quit
        } | ftp -nv $RHOST

        sum1="$(ls -l $file | awk '{print $5}')"
        sum2="$(ls -l $TST_NET_DATAROOT/$file | awk '{print $5}')"
        rm -f $file
        EXPECT_PASS "[ '$sum1' = '$sum2' ]"
    done
}

test_put()
{
    local file sum1 sum2

    for file in $(list_files $1); do
        {
            echo user $RUSER $PASSWD
            echo lcd $TST_NET_DATAROOT
            echo $1
            echo cd $TST_TMPDIR
            echo put $file
            echo quit
        } | ftp -nv $RHOST

        sum1="$(tst_rhost_run -c "sum $TST_TMPDIR/$file" -s | awk '{print $1}')"
        sum2="$(sum $TST_NET_DATAROOT/$file | awk '{print $1}')"
        tst_rhost_run -c "rm -f $TST_TMPDIR/$file"
        EXPECT_PASS "[ '$sum1' = '$sum2' ]"
    done
}

. tst_net.sh
tst_run
