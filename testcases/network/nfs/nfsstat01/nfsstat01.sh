#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2016-2018 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2001

TST_SETUP="nfsstat_setup"
TST_TESTFUNC="do_test"
TST_NEEDS_CMDS="nfsstat"
NS_STAT_RHOST=0

nfsstat_setup()
{
	nfs_setup

	if tst_net_use_netns && [ -z "$LTP_NFS_NETNS_USE_LO" ]; then
		tst_rhost_run -c "test -r /proc/net/rpc/nfs" && NS_STAT_RHOST=1
	fi
}

get_calls()
{
	local name=$1
	local field=$2
	local nfs_f=$3
	local netns=${4:-rhost}
	local type="lhost"
	local calls opt

	[ "$name" = "rpc" ] && opt="r" || opt="n"
	[ "$nfs_f" = "nfsd" ] && opt="-s$opt" || opt="-c$opt"

	if tst_net_use_netns; then
		# In netns setup, rhost is the client
		[ "$nfs_f" = "nfs" ] && [ $NS_STAT_RHOST -ne 0 ] && \
			type="$netns"
	else
		[ "$nfs_f" != "nfs" ] && type="rhost"
	fi

	if [ "$type" = "lhost" ]; then
		calls="$(grep $name /proc/net/rpc/$nfs_f | cut -d' ' -f$field)"
		ROD nfsstat $opt | grep -q "$calls"
	else
		calls=$(tst_rhost_run -c "grep $name /proc/net/rpc/$nfs_f" | \
			cut -d' ' -f$field)
		tst_rhost_run -s -c "nfsstat $opt" | grep -q "$calls"
	fi

	if ! tst_is_int "$calls"; then
		if [ "$type" = "lhost" ]; then
			tst_res TINFO "lhost /proc/net/rpc/$nfs_f"
			cat /proc/net/rpc/$nfs_f >&2
		else
			tst_res TINFO "rhost /proc/net/rpc/$nfs_f"
			tst_rhost_run -c "cat /proc/net/rpc/$nfs_f" >&2
		fi

		tst_res TWARN "get_calls: failed to get integer value (args: $@)"
	fi

	echo "$calls"
}

# PURPOSE:  Performs simple copies and removes to verify statistic
#           tracking using the 'nfsstat' command and /proc/net/rpc
do_test()
{
	local client_calls server_calls new_server_calls new_client_calls
	local client_field server_field root_calls new_root_calls
	local client_v=$VERSION server_v=$VERSION

	tst_res TINFO "checking RPC calls for server/client"

	server_calls="$(get_calls rpc 2 nfsd)"
	client_calls="$(get_calls rpc 2 nfs)"
	root_calls="$(get_calls rpc 2 nfs lhost)"

	tst_res TINFO "calls $server_calls/$client_calls"

	tst_res TINFO "Checking for tracking of RPC calls for server/client"
	cat /proc/cpuinfo > nfsstat01.tmp

	new_server_calls="$(get_calls rpc 2 nfsd)"
	new_client_calls="$(get_calls rpc 2 nfs)"
	new_root_calls="$(get_calls rpc 2 nfs lhost)"
	tst_res TINFO "new calls $new_server_calls/$new_client_calls"

	if [ "$new_server_calls" -le "$server_calls" ]; then
		tst_res TFAIL "server RPC calls not increased"
	else
		tst_res TPASS "server RPC calls increased"
	fi

	if [ "$new_client_calls" -le "$client_calls" ]; then
		tst_res TFAIL "client RPC calls not increased"
	else
		tst_res TPASS "client RPC calls increased"
	fi

	if [ $NS_STAT_RHOST -ne 0 ]; then
		tst_res TINFO "Root NS client RPC calls: $root_calls => $new_root_calls"

		if [ $root_calls -ne $new_root_calls ]; then
			tst_res TFAIL "RPC stats leaked between net namespaces"
		else
			tst_res TPASS "RPC stats stay within net namespaces"
		fi
	fi

	tst_res TINFO "checking NFS calls for server/client"
	case $VERSION in
	2) client_field=13 server_field=13
	;;
	3) client_field=15 server_field=15
	;;
	4*) client_field=24 server_field=31 client_v=4 server_v=4ops
	;;
	esac

	server_calls="$(get_calls proc$server_v $server_field nfsd)"
	client_calls="$(get_calls proc$client_v $client_field nfs)"
	tst_res TINFO "calls $server_calls/$client_calls"

	tst_res TINFO "Checking for tracking of NFS calls for server/client"
	rm -f nfsstat01.tmp

	new_server_calls="$(get_calls proc$server_v $server_field nfsd)"
	new_client_calls="$(get_calls proc$client_v $client_field nfs)"
	tst_res TINFO "new calls $new_server_calls/$new_client_calls"

	if [ "$new_server_calls" -le "$server_calls" ]; then
		tst_res TFAIL "server NFS calls not increased"
	else
		tst_res TPASS "server NFS calls increased"
	fi

	if [ "$new_client_calls" -le "$client_calls" ]; then
		tst_res TFAIL "client NFS calls not increased"
	else
		tst_res TPASS "client NFS calls increased"
	fi
}

. nfs_lib.sh
tst_run
