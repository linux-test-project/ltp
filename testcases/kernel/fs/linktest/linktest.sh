#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines Corp., 2000
# Copyright (c) Linux Test Project, 2012-2019
# Regression test for max links per file
# Author: Ngie Cooper <yaneurabeya@gmail.com>

TST_NEEDS_TMPDIR=1
TST_TESTFUNC=do_test
TST_OPTS="a:s:"
TST_PARSE_ARGS=parse_args
TST_USAGE=usage

hard_links=1000
soft_links=1000

usage()
{
	echo "Usage: linktest.sh {-a n} {-s n}"
	echo "-a n    Hard link count"
	echo "-s n    Soft link count"
}

parse_args()
{
	tst_is_int "$2" || tst_brk TBROK "-$1 must be integer ($2)"
	[ "$2" -ge 0 ] || tst_brk TBROK "-$1 must be >= 0 ($2)"

	case $1 in
	a) hard_links=$2;;
	s) soft_links=$2;;
	esac
}

do_link()
{
	local prefix="$1"
	local ln_opts="$2"
	local limit="$3"
	local prefix_msg="$4"

	local lerrors=0
	local i=0
	local rtype="TFAIL"

	tst_res TINFO "test $prefix_msg link, limit: $limit"

	cd "${prefix}link.$$"
	while [ $i -lt $limit ]; do
		if ! ln $ln_opts "$PWD/${prefix}file" ${prefix}file${i}; then
			lerrors=$((lerrors + 1))
		fi
		i=$((i + 1))
	done
	cd ..

	[ $lerrors -eq 0 ] && rtype="TPASS"

	tst_res $rtype "errors: $lerrors"
}

do_test()
{
	mkdir hlink.$$ slink.$$
	touch hlink.$$/hfile slink.$$/sfile

	do_link "s" "-s" $soft_links "symbolic"
	do_link "h"   "" $hard_links "hard"

	rm -rf hlink.$$ slink.$$
}

. tst_test.sh
tst_run
