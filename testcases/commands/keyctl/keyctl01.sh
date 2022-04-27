#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2017 Fujitsu Ltd.
# Ported: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
#
# This is a regression test about potential uninitialized variable,
# the test can crash the buggy kernel, and the bug has been fixed in:
#
#   commit 38327424b40bcebe2de92d07312c89360ac9229a
#   Author: Dan Carpenter <dan.carpenter@oracle.com>
#   Date:   Thu Jun 16 15:48:57 2016 +0100
#
#   KEYS: potential uninitialized variable

TST_SETUP=setup
TST_CLEANUP=cleanup
TST_TESTFUNC=do_test
TST_NEEDS_ROOT=1
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="keyctl"

check_keyctl()
{
	local nosup
	for op in $@; do
		nosup=0

		if ! keyctl 2>&1 | grep -q "keyctl $op"; then
			nosup=1
		fi

		if [ "$op" = "request2" ]; then
			local key=`keyctl request2 user debug:foo bar`
			if [ $? -ne 0 ]; then
				nosup=1
			fi
		fi

		if [ "$op" = "unlink" ]; then
			if ! keyctl unlink $key @s; then
				nosup=1
			fi
		fi

		if [ $nosup -ne 0 ]; then
			tst_brk TCONF "keyctl operation $op not supported"
		fi
	done
}

setup()
{
	check_keyctl negate request2 show unlink

	PATH_KEYSTAT="/proc/key-users"
	PATH_KEYQUOTA="/proc/sys/kernel/keys/root_maxbytes"

	if [ ! -f "$PATH_KEYSTAT" ] || [ ! -f "$PATH_KEYQUOTA" ]; then
		tst_brk TCONF "'${PATH_KEYSTAT}' or '${PATH_KEYQUOTA}' \
			does not exist"
	fi

	ORIG_KEYSZ=`awk -F' +|/' '/ 0:/ {print $8}' $PATH_KEYSTAT`
	ORIG_MAXKEYSZ=`cat $PATH_KEYQUOTA`
}

cleanup()
{
	if [ -n "$ORIG_MAXKEYSZ" ]; then
		echo $ORIG_MAXKEYSZ >$PATH_KEYQUOTA
	fi
}

do_test()
{
	local quota_excd=0
	local maxkeysz=$((ORIG_KEYSZ + 100))

	while [ $maxkeysz -ge 0 ]
	do
		echo $maxkeysz >$PATH_KEYQUOTA

		keyctl request2 user debug:fred negate @t >temp 2>&1
		grep -q -E "quota exceeded" temp
		if [ $? -eq 0 ]; then
			quota_excd=1
			break
		fi

		local key=`keyctl show | awk '/debug:fred/ {print $1}'`
		if [ -z "$key" ]; then
			key=`keyctl show | \
				awk -F ':' '/inaccessible/ {print $1}'`
		fi

		if [ -n "$key" ]; then
			keyctl unlink $key @s >/dev/null
			tst_sleep 50ms
		fi

		maxkeysz=$((maxkeysz - 4))
	done

	if [ $quota_excd -eq 0 ]; then
		tst_res TWARN "Failed to trigger the quota excess"
	fi

	tst_res TPASS "Bug not reproduced"
}

. tst_test.sh
tst_run
