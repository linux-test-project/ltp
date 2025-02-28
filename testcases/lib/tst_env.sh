#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2024-2025 Cyril Hrubis <chrubis@suse.cz>
#
# This is a minimal test environment for a shell scripts executed from C by
# tst_run_shell() function. Shell tests must use the tst_loader.sh instead!
#

tst_script_name=$(basename $0)

# bash does not expand aliases in non-iteractive mode, enable it
if [ -n "$BASH_VERSION" ]; then
	shopt -s expand_aliases
fi

# dash does not support line numbers even though this is mandated by POSIX
if [ -z "$LINENO" ]; then
	LINENO=-1
fi

tst_brk_()
{
	tst_res_ "$@"

	case "$3" in
		"TBROK") exit 2;;
		*) exit 0;;
	esac
}

alias tst_res="tst_res_ $tst_script_name \$LINENO"
alias tst_brk="tst_brk_ $tst_script_name \$LINENO"
