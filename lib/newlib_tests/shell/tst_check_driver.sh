#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2021 Petr Vorel <pvorel@suse.cz>

TST_TESTFUNC=test
TST_SETUP=setup
TST_CNT=4
TST_NEEDS_CMDS="tst_check_drivers find grep head sed"

MODULES_DIR="${MODULES_DIR:-/lib/modules/$(uname -r)}"

setup()
{
	tst_res TINFO "using modules directory '$MODULES_DIR'"

	[ -d "$MODULES_DIR" ] || \
		tst_brk TCONF "modules directory '$MODULES_DIR' missing"
}

test_drivers()
{
	local paths="$*"
	local drv

	if [ -z "$paths" ]; then
		tst_res TCONF "no modules found"
		return
	fi

	for drv in $paths; do
		drv="$(echo $drv | sed 's/.*\/\([^/]\+\)\.ko.*/\1/')"
		EXPECT_PASS tst_check_drivers $drv
		drv="$(echo $drv | sed 's/_/-/g')"
		EXPECT_PASS tst_check_drivers $drv
	done
}

test1()
{
	tst_res TINFO "check loadable module detection"
	test_drivers $(find $MODULES_DIR | grep -E '_[^/]+\.ko' | head -3)
}

test2()
{
	tst_res TINFO "check non-existing module detection"
	EXPECT_FAIL tst_check_drivers not-existing-kernel-module
}

test3()
{
	local f="$MODULES_DIR/modules.builtin"

	tst_res TINFO "check built-in module detection"

	[ -f "$f" ] || tst_brk TCONF "missing '$f'"

	test_drivers $(grep -E '_[^/]+\.ko' $f | head -3)
}

test4()
{
	local f="$MODULES_DIR/modules.builtin"

	tst_res TINFO "check for x68_64 arch module detection"

	[ -f "$f" ] || tst_brk TCONF "missing '$f'"
	test_drivers $(grep -E '[^/]+[-_]x86[-_]64.*\.ko' $f | head -3)
}

. tst_test.sh
tst_run
