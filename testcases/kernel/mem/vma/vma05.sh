#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2017 Red Hat, Inc.
# Copyright (C) 2024 Cyril Hrubis <chrubis@suse.cz>
#
# ---
# doc
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
# ---
#
# ---
# env
# {
#  "needs_root": true,
#  "needs_tmpdir": true,
#  "needs_cmds": ["gdb", "uname"],
#  "save_restore": [
#   ["/proc/sys/kernel/core_pattern", "core", "TBROK"],
#   ["/proc/sys/kernel/core_uses_pid", "0", "TBROK"]
#  ],
#  "tags": [
#   ["linux-git", "103efcd9aac1"],
#   ["linux-git", "b6558c4a2378"],
#   ["linux-git", "e5b97dde514f"]
#  ]
# }
# ---

. tst_loader.sh

tst_test()
{
	ulimit -c unlimited
	unset DEBUGINFOD_URLS

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
	[ -f core ] || tst_brk TBROK "missing core file"

	TRACE=$(gdb -silent -ex="thread apply all backtrace" -ex="quit"\
		vma05_vdso ./core* 2> /dev/null)

	# Only check for ?? symbols in application code, not system libraries
	APP_UNKNOWN=$(echo "$TRACE" | grep -F "??" | grep -v -e "from /lib/" -e "from /usr/lib/")
	if [ -n "$APP_UNKNOWN" ]; then
		tst_res TFAIL "[vdso] bug not patched - unknown symbols in application code"
	else
		tst_res TPASS "[vdso] backtrace complete"
	fi
}

. tst_run.sh
