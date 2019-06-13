#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines Corp., 2001
# Copyright (c) Cyril Hrubis <chrubis@suse.cz>
# Author: Manoj Iyer <manjo@mail.utexas.edu>
#
# Tests basic functionality of unzip command.

TST_SETUP=setup
TST_TESTFUNC=do_test
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="unzip"
. tst_test.sh

setup()
{
	cat > unzip_exp.out <<EOF
Archive:  $TST_DATAROOT/test.zip
   creating: dir/
   creating: dir/d1/
   creating: dir/d2/
   creating: dir/d3/
   creating: dir/d4/
 extracting: dir/d1/f1
 extracting: dir/d1/f2
 extracting: dir/d1/f3
   creating: dir/d2/d1/
   creating: dir/d2/d2/
   creating: dir/d2/d3/
 extracting: dir/d2/f1
 extracting: dir/d2/f2
 extracting: dir/d2/f3
   creating: dir/d3/d1/
   creating: dir/d3/d2/
   creating: dir/d3/d3/
EOF
}

stable_ls()
{
	local i

	for i in $(echo "$1/*" | sort); do

		if ! [ -e "$i" ]; then
			return
		fi

		echo "$i"

		if [ -d "$i" ]; then
			stable_ls "$i"
		fi
	done
}

do_test()
{
	EXPECT_PASS unzip "$TST_DATAROOT/test.zip" \> unzip.out

	if diff -w unzip_exp.out unzip.out; then
		tst_res TPASS "Unzip output is correct"
	else
		tst_res TFAIL "Unzip output is incorrect"
		cat unzip.out
	fi

	stable_ls "dir" > dir.out

	if diff "$TST_DATAROOT/dir.out" dir.out; then
		tst_res TPASS "Files unzipped correctly"
	else
		tst_res TFAIL "Files unzipped incorrectly"
		cat dir.out
	fi

	ROD rm -rf "dir/"
}

tst_run
