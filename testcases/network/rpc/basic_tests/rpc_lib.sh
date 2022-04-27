#!/bin/sh
# Copyright (c) 2020-2022 Petr Vorel <pvorel@suse.cz>

TST_NEEDS_CMDS="rpcinfo $TST_NEEDS_CMDS"

check_rpc()
{
	local services

	tst_res TINFO "check registered RPC with rpcinfo"

	services=$(rpcinfo -p)

	if [ $? -ne 0 ] || ! echo "$services" | grep -q '[0-9]'; then
		tst_brk TCONF "no RPC services, is rpcbind/portmap running?"
	fi

	tst_res TINFO "registered RPC:"
	echo "$services"
}

. tst_net.sh
