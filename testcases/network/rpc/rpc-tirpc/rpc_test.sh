# SPDX-License-Identifier: GPL-2.0-or-later
#!/bin/sh
# Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) 2017-2020 Petr Vorel <pvorel@suse.cz>

SERVER=
CLIENT=
CLIENT_EXTRA_OPTS=
CLEANER=
# Program number to register the services to rpcbind
PROGNUMNOSVC=536875000

TST_TESTFUNC=do_test
TST_USAGE=usage
TST_OPTS="c:e:s:"
TST_SETUP=setup
TST_CLEANUP=cleanup
TST_PARSE_ARGS=rpc_parse_args
TST_NEEDS_CMDS="pkill rpcinfo"

usage()
{
	cat << EOF
USAGE: $0 [-s sprog] -c clprog [ -e extra ]

Connect to the remote host and start sprog.
Then execute clprog and passing it the remote host value.

-c clprog client program binary
-s sprog  server program binary
-e extra  extra client options
EOF
}

rpc_parse_args()
{
	case "$1" in
		c) CLIENT="$OPTARG" ;;
		e) tst_check_cmds sed
		   CLIENT_EXTRA_OPTS="$(echo $OPTARG | sed 's/,/ /')" ;;
		s) SERVER="$OPTARG" ;;
	esac
}

setup()
{
	check_rpc

	if [ -n "$SERVER" ]; then
		CLEANER="rpc_cleaner"
		if echo "$SERVER" | grep -q '^tirpc'; then
			CLEANER="tirpc_cleaner"
		fi
	fi

	[ -n "$CLIENT" ] || tst_brk TBROK "client program not set"
	tst_check_cmds $CLIENT $SERVER || tst_brk TCONF "LTP compiled without TI-RPC support?"

	tst_cmd_available ldd which || return
	if ldd $(which $CLIENT) |grep -q /libtirpc\.so; then
		tst_res TINFO "using libtirpc: yes"
	else
		tst_res TINFO "using libtirpc: no (probably using glibc)"
	fi
}

cleanup()
{
	if [ "$SERVER_STARTED" ]; then
		pkill -13 -x $SERVER
		$CLEANER $PROGNUMNOSVC
	fi
}

do_test()
{
	local i

	if [ -n "$SERVER" ]; then
		$SERVER $PROGNUMNOSVC &
		SERVER_STARTED=1

		for i in $(seq 1 10); do
			rpcinfo -p localhost | grep -q $PROGNUMNOSVC && break
			[ "$i" -eq 10 ] && tst_brk TBROK "server not registered"
			tst_sleep 100ms
		done
	fi

	EXPECT_RHOST_PASS $CLIENT $(tst_ipaddr) $PROGNUMNOSVC $CLIENT_EXTRA_OPTS
}

. rpc_lib.sh
tst_run
