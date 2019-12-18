#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Oracle and/or its affiliates. All Rights Reserved.

TST_SETUP="do_setup"
TST_CLEANUP="do_cleanup"
use_iptables=0
cleanup_table=0
cleanup_chain=0

. iptables_lib.sh

do_setup()
{
	if ! nft list table ip filter > /dev/null 2>&1; then
		ROD nft add table ip filter
		cleanup_table=1
	fi
	if ! nft list chain ip filter INPUT > /dev/null 2>&1; then
		ROD nft add chain ip filter INPUT '{ type filter hook input priority 0; }'
		cleanup_chain=1
	fi
	init
}

do_cleanup()
{
	[ "$cleanup_chain" = 1 ] && nft delete chain ip filter INPUT >/dev/null 2>&1
	[ "$cleanup_table" = 1 ] && nft delete table ip filter >/dev/null 2>&1
	cleanup
}

tst_run
