#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines Corp., 2001
# Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
# Author: Manoj Iyer <manjo@mail.utexas.edu>
#
# Creates, lists and extracts an plain, gzip and bzip tar archive.

TST_CNT=6
TST_TESTFUNC=do_test
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="gzip bzip2"

. tst_test.sh

TAR_FILES="file1 file2 file3"

check_listing()
{
	local i
	local verbose=$1
	shift

	if [ -z "$verbose" ]; then
		if [ -s tar.out ]; then
			tst_res TFAIL "Tar produced unexpected output"
			cat tar.out
		else
			tst_res TPASS "Tar produced no output"
		fi

		return
	fi

	if [ $(wc -l < tar.out) != $# ]; then
		tst_res TFAIL "Unexpected number of lines in tar.out"
		cat tar.out
		return
	fi

	for i in $@; do
		if ! grep -q $i tar.out; then
			tst_res TFAIL "File $i missing in listing"
			return
		fi
	done

	tst_res TPASS "Listing in tar.out is correct"
}

check_content()
{
	local fname="$1"
	local verbose="$2"
	shift 2

	EXPECT_PASS tar t${verbose}f "$fname" \> tar.out
	check_listing v $@
}

check_files()
{
	for i in $@; do
		if ! [ -f $i ]; then
			tst_res TFAIL "Missing file $i in extracted archive"
			cat tar.out
			return
		fi
	done

	tst_res TPASS "Files were uncompressed correctly"
}

check_extraction()
{
	local fname="$1"
	local verbose="$2"
	shift 2

	EXPECT_PASS tar x${verbose}f $fname \> tar.out
	check_listing "${verbose}" $@
	check_files $@
	ROD rm $@
}

test_tar()
{
	local comp="$1"
	local verbose="$2"
	local fname="$3"
	local i

	# Create archive
	ROD touch $TAR_FILES
	EXPECT_PASS tar c${verbose}f$comp $fname $TAR_FILES \> tar.out
	check_listing "$verbose" $TAR_FILES

	# Diff filesystem against the archive, should be the same at this point
	EXPECT_PASS tar d${verbose}f $fname \> tar.out
	check_listing "$verbose" $TAR_FILES

	ROD rm $TAR_FILES

	# Check content listing
	check_content $fname "$verbose" $TAR_FILES

	# Check decompression
	check_extraction $fname "$verbose" $TAR_FILES

	# Append to an archive, only possible for uncompressed archive
	if [ -z "$comp" ]; then
		ROD touch file4
		EXPECT_PASS tar r${verbose}f $fname file4 \> tar.out
		check_listing "$verbose" file4
		check_content $fname "$verbose" $TAR_FILES file4
		ROD rm file4

		check_extraction $fname "$verbose" $TAR_FILES file4
	fi

	ROD rm $fname
}

do_test()
{
	case $1 in
	1) test_tar ""  "v" "test.tar";;
	2) test_tar "z" "v" "test.tar.gz";;
	3) test_tar "j" "v" "test.tar.bz2";;
	4) test_tar ""  ""  "test.tar";;
	5) test_tar "z" ""  "test.tar.gz";;
	6) test_tar "j" ""  "test.tar.bz2";;
	esac
}

tst_run
