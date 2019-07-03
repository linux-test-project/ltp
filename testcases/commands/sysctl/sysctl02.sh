#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>
# Author: Yang Xu<xuyang2018.jy@cn.fujitsu.com>
#
# Test for these regressions causing buffer overflow when writing into
# /proc/sys/fs/file-max:
# 7f2923c4f73f ("sysctl: handle overflow in proc_get_long")
# 32a5ad9c2285 ("sysctl: handle overflow for file-max")
#
# This bug has been fixed in 9002b21465fa ("kernel/sysctl.c: fix
# out-of-bounds access when setting file-max")
#
# We test in sysctl02.sh setting 2^64, 2^64-1, 2^63 and 0.

TST_TESTFUNC=do_test
TST_SETUP=setup
TST_CLEANUP=cleanup
TST_CNT=4
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="sysctl"
sys_name="fs.file-max"
sys_file="/proc/sys/fs/file-max"
syms_file="/proc/kallsyms"

. tst_test.sh

setup()
{
	[ ! -f "$sys_file" ] && tst_brk TCONF "$sys_file not enabled"
	orig_value=$(cat "$sys_file")
}

do_test()
{
	case $1 in
	1) sysctl_test_overflow 18446744073709551616 ;;
	2) sysctl_test_overflow 18446744073709551615 ;;
	3) sysctl_test_overflow 9223372036854775808 ;;
	4) sysctl_test_zero ;;
	esac
}

sysctl_test_overflow()
{
	local test_value="$1"
	local old_value="$(cat $sys_file)"

	tst_res TINFO "trying to set $sys_name=$test_value"
	sysctl -w -q $sys_name=$test_value 2>/dev/null
	local new_value="$(cat $sys_file)"

	if [ "$new_value" = "$old_value" ]; then
		tst_res TPASS "$sys_file keeps old value ($old_value)"
	else
		tst_res TFAIL "$sys_file overflows and is set to $new_value"
	fi
	cleanup
}

sysctl_test_zero()
{
	[ ! -f "$syms_file" ] && tst_brk TCONF "$syms_file not enabled"
	ROD sysctl -w -q $sys_name=0

	if grep -q kasan_report $syms_file; then
		if dmesg | grep -q "KASAN: global-out-of-bounds in __do_proc_doulongvec_minmax"; then
			tst_res TFAIL "$sys_file is set 0 and trigger a KASAN error"
		else
			tst_res TPASS "$sys_file is set 0 and doesn't trigger a KASAN error"
		fi
	else
		tst_res TCONF "kernel doesn't support KASAN"
	fi
}

cleanup()
{
	[ -n "$orig_value" ] && sysctl -w -q $sys_name=$orig_value
}

tst_run
