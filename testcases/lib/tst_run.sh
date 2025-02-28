#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2025 Cyril Hrubis <chrubis@suse.cz>
# Copyright (c) 2025 Petr Vorel <pvorel@suse.cz>

. tst_env.sh

if [ -n "$TST_CLEANUP" ]; then
	if command -v $TST_CLEANUP >/dev/null 2>/dev/null; then
		trap $TST_CLEANUP EXIT
	else
		tst_res TWARN "TST_CLEANUP=$TST_CLEANUP declared, but function not defined (or cmd not found)"
	fi
fi

if [ -n "$TST_SETUP" ]; then
	if command -v $TST_SETUP >/dev/null 2>/dev/null; then
		$TST_SETUP
	else
		tst_brk TBROK "TST_SETUP=$TST_SETUP declared, but function not defined (or cmd not found)"
	fi
fi

tst_test
