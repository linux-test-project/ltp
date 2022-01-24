#!/bin/sh
# Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2017 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines Corp., 2000

TST_TESTFUNC=do_test
TST_SETUP=do_setup
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="rpcinfo wc"
. rpc_lib.sh

do_setup()
{
	check_rpc

	# Create file with 1 tcp and 1 udp line. Use for variable assignments.
	rpcinfo -p $(tst_ipaddr) | grep tcp | sed -n 2p > rpc_out
	rpcinfo -p $(tst_ipaddr) | grep udp | sed -n 2p >> rpc_out

	wc -l rpc_out | grep -q "2" || \
		tst_brk TBROK "Not enough programs registered on $(tst_ipaddr)"

	# Using "rpc_out" file created above, assign variables for rpcinfo opts
	TPNUM=$(grep tcp rpc_out | awk '{print $1}')
	TVNUM=$(grep tcp rpc_out | awk '{print $2}')
	TCPNAME=$(grep tcp rpc_out | awk '{print $5}')
	UPNUM=$(grep udp rpc_out | awk '{print $1}')
	UVNUM=$(grep udp rpc_out | awk '{print $2}')
	UDPNAME=$(grep udp rpc_out | awk '{print $5}')
}

do_test()
{
	local thost="$(tst_ipaddr)"

	EXPECT_RHOST_PASS rpcinfo -p $thost | grep -q portmapper
	EXPECT_RHOST_PASS rpcinfo -t $thost $TPNUM
	EXPECT_RHOST_PASS rpcinfo -t $thost $TPNUM $TVNUM
	EXPECT_RHOST_PASS rpcinfo -t $thost $TCPNAME
	EXPECT_RHOST_PASS rpcinfo -t $thost $TCPNAME $TVNUM
	EXPECT_RHOST_PASS rpcinfo -u $thost 100000
	EXPECT_RHOST_PASS rpcinfo -u $thost 100000 2
	EXPECT_RHOST_PASS rpcinfo -u $thost portmapper
	EXPECT_RHOST_PASS rpcinfo -u $thost portmapper 2

	tst_res TINFO "Test rpcinfo with missing or bad options"
	EXPECT_RHOST_FAIL rpcinfo -p bogushost
	EXPECT_RHOST_FAIL rpcinfo -bogusflag
	EXPECT_RHOST_FAIL rpcinfo -t $thost
	EXPECT_RHOST_FAIL rpcinfo -u $thost
	EXPECT_RHOST_FAIL rpcinfo -u $thost bogusprog
	EXPECT_RHOST_FAIL rpcinfo -u $thost 11579
	EXPECT_RHOST_FAIL rpcinfo -u $thost 100000 5
}

tst_run
