#!/bin/sh
# Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>

. tst_net.sh

check_portmap_rpcbind()
{
	if pgrep portmap > /dev/null; then
		PORTMAPPER="portmap"
	else
		pgrep rpcbind > /dev/null && PORTMAPPER="rpcbind" || \
			tst_brk TCONF "portmap or rpcbind is not running"
	fi
	tst_res TINFO "Using $PORTMAPPER"
}
