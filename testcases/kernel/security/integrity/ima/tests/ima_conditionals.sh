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

TST_NEEDS_CMDS="cat chgrp chown"
TST_SETUP="setup"
TST_OPTS="r:"
TST_USAGE="usage"
TST_PARSE_ARGS="parse_args"
REQUEST="uid"

parse_args()
{
	REQUEST="$2"
}

usage()
{
	cat << EOF
usage: $0 [-r <uid|fowner|gid|fgroup>]

OPTIONS
-r	Specify the request to be measured. One of:
	uid, fowner, gid, fgroup
	Default: uid
EOF
}

setup()
{
	case "$REQUEST" in
	fgroup|fowner|gid|uid)
		tst_res TINFO "request '$REQUEST'"
		;;
	*) tst_brk TBROK "Invalid -r '$REQUEST', use: -r <uid|fowner|gid|fgroup>";;
	esac

	if check_need_signed_policy; then
		tst_brk TCONF "policy have to be signed"
	fi
}

test()
{
	# needs to be checked each run (not in setup)
	require_policy_writable

	local request="$1"
	local test_file="$PWD/test.txt"
	local cmd="cat $test_file > /dev/null"
	local value="$TST_USR_UID"

	if [ "$REQUEST" = 'gid' -o "$REQUEST" = 'fgroup' ]; then
		if tst_kvcmp -lt 5.16; then
			tst_brk TCONF "gid and fgroup options require kernel 5.16 or newer"
		fi
		value="$TST_USR_GID"
	fi

	ROD rm -f $test_file

	tst_res TINFO "verify measuring user files when requested via $REQUEST"
	ROD echo "measure $REQUEST=$value" \> $IMA_POLICY
	ROD echo "$(cat /proc/uptime) $REQUEST test" \> $test_file

	case "$REQUEST" in
	fgroup)
		chgrp $TST_USR_GID $test_file
		sh -c "$cmd"
		;;
	fowner)
		chown $TST_USR_UID $test_file
		sh -c "$cmd"
		;;
	gid|uid) tst_runas sh -c "$cmd";;
	esac

	ima_check $test_file
}

. ima_setup.sh
tst_run
