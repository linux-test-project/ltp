#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2017 Red Hat, Inc.
# Regression test if the vsyscall and vdso VMA regions are reported correctly.
#
# While [vsyscall] is mostly deprecated with newer systems, there is
# still plenty of kernels compiled with CONFIG_LEGACY_VSYSCALL_NATIVE and
# CONFIG_LEGACY_VSYSCALL_EMULATE (see linux/arch/x86/Kconfig for option
# descriptions). First part of the test will check eligible kernels for
# regression for a bug fixed by commit 103efcd9aac1 (fix perms/range of
# vsyscall vma in /proc/*/maps).
#
# Second part of test checks [vdso] VMA permissions (fixed with commits
# b6558c4a2378 (fix [vdso] page permissions) and e5b97dde514f (Add
# VM_ALWAYSDUMP)). As a consequence of this bug, VMAs were not included
# in core dumps which resulted in eg. incomplete backtraces and invalid
# core dump files created by gdb.

TST_SETUP=setup
TST_CLEANUP=cleanup
TST_TESTFUNC=vma_report_check
TST_NEEDS_ROOT=1
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="gdb"

CORE_LIMIT=$(ulimit -c)
CORE_PATTERN=$(cat /proc/sys/kernel/core_pattern)

setup()
{
	ulimit -c unlimited
	echo "core" > /proc/sys/kernel/core_pattern
}

cleanup()
{
	ulimit -c "$CORE_LIMIT"
	echo "$CORE_PATTERN" > /proc/sys/kernel/core_pattern
}

vma_report_check()
{
	if [ $(uname -m) = "x86_64" ]; then
		if LINE=$(grep "vsyscall" /proc/self/maps); then
			RIGHT="ffffffffff600000-ffffffffff601000[[:space:]][r-]-xp"
			if echo "$LINE" | grep -q "$RIGHT"; then
				tst_res TPASS "[vsyscall] reported correctly"
			else
				tst_res TFAIL "[vsyscall] reporting wrong"
			fi
		fi
	fi

	rm -rf core*
	{ vma05_vdso; } > /dev/null 2>&1
	TRACE=$(gdb -silent -ex="thread apply all backtrace" -ex="quit"\
		vma05_vdso ./core* 2> /dev/null)
	if echo "$TRACE" | grep -qF "??"; then
		tst_res TFAIL "[vdso] bug not patched"
	else
		tst_res TPASS "[vdso] backtrace complete"
	fi
}

. tst_test.sh
tst_run
