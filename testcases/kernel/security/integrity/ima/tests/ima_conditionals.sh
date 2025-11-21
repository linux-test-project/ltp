#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2021 VPI Engineering
# Copyright (c) 2021-2025 Petr Vorel <pvorel@suse.cz>
# Author: Alex Henrie <alexh@vpitech.com>
#
# Verify that conditional rules work.
#
# gid and fgroup options test kernel commit 40224c41661b ("ima: add gid
# support") from v5.16.

TST_NEEDS_CMDS="cat chgrp chown id sg sudo"
TST_SETUP="setup"
TST_CNT=1

setup()
{
	if check_need_signed_policy; then
		tst_brk TCONF "policy have to be signed"
	fi
}

verify_measurement()
{
	local request="$1"
	local user="nobody"
	local test_file="$PWD/test.txt"
	local cmd="cat $test_file > /dev/null"

	local value="$(id -u $user)"
	[ "$request" = 'gid' -o "$request" = 'fgroup' ] && value="$(id -g $user)"

	# needs to be checked each run (not in setup)
	require_policy_writable

	ROD rm -f $test_file

	tst_res TINFO "verify measuring user files when requested via $request"
	ROD echo "measure $request=$value" \> $IMA_POLICY
	ROD echo "$(cat /proc/uptime) $request test" \> $test_file

	case "$request" in
	fgroup)
		chgrp $user $test_file
		sh -c "$cmd"
		;;
	fowner)
		chown $user $test_file
		sh -c "$cmd"
		;;
	gid) sg $user "sh -c '$cmd'";;
	uid) sudo -n -u $user sh -c "$cmd";;
	*) tst_brk TBROK "Invalid res type '$1'";;
	esac

	ima_check $test_file
}

test1()
{
	verify_measurement uid
	verify_measurement fowner

	if tst_kvcmp -lt 5.16; then
		tst_brk TCONF "gid and fgroup options require kernel 5.16 or newer"
	fi

	verify_measurement gid
	verify_measurement fgroup
}

. ima_setup.sh
tst_run
