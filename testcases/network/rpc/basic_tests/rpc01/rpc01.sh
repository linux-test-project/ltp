#!/bin/sh
# Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2017 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines Corp., 2000

TST_TESTFUNC=do_test
TST_SETUP=do_setup
TST_CLEANUP=do_cleanup
TST_NEEDS_CMDS="pkill rpcinfo"
. rpc_lib.sh

NUMLOOPS=${NUMLOOPS:-3}
DATAFILES="${DATAFILES:-file.1 file.2}"

do_cleanup()
{
	pkill -9 rpc_server > /dev/null 2>&1
}

do_setup()
{
	check_rpc

	tst_res TINFO "start rpc_server"
	ROD rpc_server

	tst_res TINFO "wait for server to be registered"
	for i in $(seq 1 30); do
		rpcinfo -T udp $(tst_ipaddr) 2000333 10 >/dev/null 2>&1 && break
		[ "$i" -eq 30 ] && tst_brk TBROK "server not registered"
		tst_sleep 100ms
	done
}

do_test()
{
	tst_res TINFO "starting client process"

	local cnt=1
	while [ $cnt -le $NUMLOOPS ]; do
		for f in $DATAFILES; do
			EXPECT_RHOST_PASS rpc1 -s $(tst_ipaddr) -f $TST_DATAROOT/$f
		done
		cnt=$(($cnt + 1))
	done
}

tst_run
