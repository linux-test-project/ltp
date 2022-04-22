#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
# Author: Petr Vorel <pvorel@suse.cz>

CMD="${CMD:-ip}"

if [ -z "$TST_SETUP" ]; then
	TST_SETUP="if_setup"
	TST_CLEANUP="${TST_CLEANUP:-netstress_cleanup}"
fi

TST_TESTFUNC="test_body"
TST_PARSE_ARGS="if_parse_args"
TST_USAGE="if_usage"
TST_OPTS="c:"
. tst_net_stress.sh

if_usage()
{
	echo "-c      Test command (ip, $IF_CMD)"
}

if_parse_args()
{
	case $1 in
	c) CMD="$2";;
	esac
}

if_setup()
{
	if [ "$CMD" != 'ip' -a "$CMD" != "$IF_CMD" ]; then
		tst_brk TBROK "Missing or wrong -c parameter: '$CMD', use 'ip' or '$IF_CMD'"
	fi

	tst_require_cmds "$CMD"
	netstress_setup
}

if_cleanup_restore()
{
	netstress_cleanup
	restore_ipaddr
	restore_ipaddr rhost
}
