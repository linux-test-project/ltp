#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Oracle and/or its affiliates. All Rights Reserved.

TST_SETUP="do_setup"
TST_CLEANUP="do_cleanup"
TST_NEEDS_DRIVERS="nf_tables"
use_iptables=0
cleanup_table=0
cleanup_chain=0

. iptables_lib.sh

do_setup()
{
	init
	local ip_table="ip${TST_IPV6}"

	if ! nft list table $ip_table filter > /dev/null 2>&1; then
		ROD nft add table $ip_table filter
		cleanup_table=1
	fi
	if ! nft list chain $ip_table filter INPUT > /dev/null 2>&1; then
		ROD nft add chain $ip_table filter INPUT '{ type filter hook input priority 0; }'
		cleanup_chain=1
	fi
}

do_cleanup()
{
	local ip_table="ip${TST_IPV6}"

	[ "$cleanup_chain" = 1 ] && nft delete chain $ip_table filter INPUT >/dev/null 2>&1
	[ "$cleanup_table" = 1 ] && nft delete table $ip_table filter >/dev/null 2>&1
	cleanup
}

tst_run
