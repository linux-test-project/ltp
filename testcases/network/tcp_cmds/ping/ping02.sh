#! /bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2016-2019 Oracle and/or its affiliates. All Rights Reserved.

TST_TESTFUNC="do_test"
TST_NEEDS_ROOT=1


do_test()
{
	tst_ping -s "${PACKETSIZES:-8 16 32 64 128 256 512 1024 2048 4064}" \
		 -p "000102030405060708090a0b0c0d0e0f" -c "${COUNT:-3}"
}

. tst_net.sh
tst_run
