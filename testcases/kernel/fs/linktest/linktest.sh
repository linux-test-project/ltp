#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines Corp., 2000
# Copyright (c) Linux Test Project, 2012-2019
# Regression test for max links per file
# linktest.sh <number of symlinks> <number of hardlinks>
# Author: Ngie Cooper <yaneurabeya@gmail.com>

TST_NEEDS_TMPDIR=1
TST_POS_ARGS=2
TST_TESTFUNC=do_test

. tst_test.sh

if [ $# -ne 2 ]; then
	tst_res TBROK "usage: $0 {softlink count} {hardlink count}"
	exit 1
fi

validate_parameter()
{
	if ! tst_is_int "$2"; then
		tst_brk TBROK "$1 must be integer"
	fi

	if [ "$2" -lt 0 ]; then
		tst_brk TBROK "$1 must be >= 0"
	fi
}

validate_parameter "softlink count" $1
validate_parameter "hardlink count" $2

soft_links=$1
hard_links=$2

do_link()
{
	local prefix="$1"
	local ln_opts="$2"
	local limit="$3"
	local prefix_msg="$4"

	local lerrors=0
	local i=0
	local rtype="TFAIL"

	cd "${prefix}link.$$"
	while [ $i -lt $limit ]; do
		if ! ln ${ln_opts} "$PWD/${prefix}file" ${prefix}file${i}; then
			lerrors=$((lerrors + 1))
		fi
		i=$((i + 1))
	done
	cd ..

	if [ $lerrors -eq 0 ]; then
		rtype=TPASS
	fi

	tst_res $rtype "$prefix_msg link errors: $lerrors"
}

do_test()
{
	mkdir hlink.$$ slink.$$
	touch hlink.$$/hfile slink.$$/sfile

	do_link s "-s" $soft_links "symbolic"
	do_link h   "" $hard_links "hard"

	rm -rf hlink.$$ slink.$$
}

tst_run
