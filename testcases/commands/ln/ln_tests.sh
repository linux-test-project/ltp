#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines Corp., 2001
# Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
# Author: Manoj Iyer <manjo@mail.utexas.edu>
#
# Basic test for ln

TST_CNT=6
TST_TESTFUNC=do_test
TST_SETUP=setup
TST_NEEDS_TMPDIR=1
. tst_test.sh

setup()
{
	ROD mkdir -p dir/subdir
	ROD echo "LTP" > file
	ROD touch dir/file
}

check_dir_link()
{
	local dname="$1"
	local lname="$2"

	ROD ls "$lname" > lname.out
	ROD ls "$dname" > dname.out

	if diff lname.out dname.out; then
		tst_res TPASS "Directory and link content is equal"
	else
		tst_res TFAIL "Directory and link content differs"
		cat lname.out
		echo
		cat dname.out
	fi
}

check_file_link()
{
	local fname="$1"
	local lname="$2"

	if diff $fname $lname; then
		tst_res TPASS "File and link content is equal"
	else
		tst_res TFAIL "File and link content differs"
	fi
}

ln_test()
{
	local args="$1"
	local src="$2"
	local link="$3"

	EXPECT_PASS ln $args $src $link

	if [ -f $src ]; then
		check_file_link $src $link
	else
		check_dir_link $src $link
	fi

	ROD rm $link
}

do_test()
{
	case $1 in
	1) ln_test ""   "file"      "file_link";;
	2) ln_test "-s" "file"      "file_link";;
	3) ln_test "-s" "dir"       "dir_link";;
	4) ln_test ""   "$PWD/file" "file_link";;
	5) ln_test "-s" "$PWD/file" "file_link";;
	6) ln_test "-s" "$PWD/dir"  "dir_link";;
	esac
}

tst_run
