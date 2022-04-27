#! /bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2014-2019 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines Corp., 2000
#
# To test the basic functionality of the `ping` command.
#
#  HISTORY:
#    06/06/03 Manoj Iyer manjo@mail.utexas.edu
#    - Modified testcase to use test APIs and also fixed minor bugs
#    03/01 Robbie Williamson (robbiew@us.ibm.com)
#      -Ported

TST_SETUP="do_setup"
TST_TESTFUNC="do_test"


do_setup()
{
	COUNT=${COUNT:-3}
	PACKETSIZES=${PACKETSIZES:-"8 16 32 64 128 256 512 1024 2048 4064"}

	PING_CMD=ping${TST_IPV6}

	tst_require_cmds $PING_CMD
}

do_test()
{
	tst_res TINFO "$PING_CMD with $PACKETSIZES ICMP packets"
	local ipaddr=$(tst_ipaddr rhost)
	local s

	for s in $PACKETSIZES; do
		EXPECT_PASS $PING_CMD -i 0.2 -c $COUNT -s $s $ipaddr \>/dev/null
	done
}

. tst_net.sh
tst_run
