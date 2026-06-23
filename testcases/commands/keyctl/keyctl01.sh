#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2017 Fujitsu Ltd.
# Ported: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
# Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
#
# ---
# doc
#
# This is a regression test for a potential uninitialized variable in
# key_reject_and_link(). The test can crash the buggy kernel.
# ---
#
# ---
# env
# {
#  "needs_root": true,
#  "needs_tmpdir": true,
#  "needs_cmds": [
#   {"cmd": "keyctl"}
#  ],
#  "save_restore": [
#   ["/proc/sys/kernel/keys/root_maxbytes", null, "TCONF"]
#  ],
#  "tags": [
#   ["linux-git", "38327424b40bcebe2de92d07312c89360ac9229a"],
#   ["CVE", "2016-4470"]
#  ]
# }
# ---

. tst_loader.sh

TST_SETUP=setup

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

	if [ ! -f "$PATH_KEYSTAT" ]; then
		tst_brk TCONF "'${PATH_KEYSTAT}' does not exist"
	fi

	ORIG_KEYSZ=`awk -F' +|/' '/ 0:/ {print $8}' $PATH_KEYSTAT`
}

tst_test()
{
	PATH_KEYQUOTA="/proc/sys/kernel/keys/root_maxbytes"

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

. tst_run.sh
