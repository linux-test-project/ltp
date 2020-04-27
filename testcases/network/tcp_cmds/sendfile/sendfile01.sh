#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) Linux Test Project, 2001-2013
# Copyright (c) Manoj Iyer <manjo@mail.utexas.edu> 2003
# Copyright (c) Robbie Williamson <robbiew@us.ibm.com> 2002-2003
# Copyright (c) International Business Machines  Corp., 2000

TST_SETUP=do_setup
TST_CLEANUP=do_cleanup
TST_TESTFUNC=do_test
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="diff ss stat"
. tst_net.sh

do_setup()
{

	tst_res TINFO "copy files from server to client using the sendfile() on IPv$TST_IPVER"

	client="testsf_c${TST_IPV6}"
	server="testsf_s${TST_IPV6}"

	port=$(tst_rhost_run -s -c "tst_get_unused_port ipv$TST_IPVER stream")
	[ -z "$port" ] && tst_brk TBROK "failed to get unused port"

	tst_rhost_run -s -b -c "$server $(tst_ipaddr rhost) $port"
	server_started=1
	tst_res TINFO "wait for the server to start"
	TST_RETRY_FUNC "tst_rhost_run -c 'ss -ltp' | grep -q '$port.*testsf'" 0
}

do_test()
{
	local file lfile size

	for file in $(ls $TST_NET_DATAROOT/ascii.*); do
		lfile="$(basename $file)"
		size=$(stat -c '%s' $file)
		tst_res TINFO "test IP: $(tst_ipaddr rhost), port: $port, file: $lfile"

		ROD $client $(tst_ipaddr rhost) $port $lfile $file $size \> /dev/null
		EXPECT_PASS diff $file $lfile
	done
}

do_cleanup()
{
	[ -n "$server_started" ] && tst_rhost_run -c "pkill $server"
}

tst_run
