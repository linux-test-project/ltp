#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines Corp., 2001
# Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
#
# This program tests the file command. The tests are aimed at
# testing if the file command can recognize some of the commonly
# used file formats like, tar, tar.gz, rpm, C, ASCII, ELF etc.

TST_CNT=20
TST_SETUP=setup
TST_TESTFUNC=do_test
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="readelf"

. tst_test.sh

setup()
{
	case $(readelf -h /bin/sh) in
	*Data:*"big endian"*) TEST_ARCH=MSB;;
	*Data:*"little endian"*) TEST_ARCH=LSB;;
        *) tst_brk TBROK "Failed to determine CPU endianess";;
	esac
}

file_test()
{
	local fname="$1"
	local fpath
	local i
	shift

	if ! [ -f "$fname" ]; then
		fpath="$TST_DATAROOT/$fname"
	else
		fpath="$fname"
	fi

	EXPECT_PASS file "$fpath" \> file.out

	while [ $# -gt 0 ]; do
		if grep -q "$1" file.out; then
			break
		fi
		shift
	done

	if [ $# -gt 0 ]; then
		tst_res TPASS "$fname: recognized as $1"
	else
		tst_res TFAIL "$fname: was not recognized"
		cat file.out
	fi
}

do_test()
{
	case $1 in
	 1) file_test in.txt "ASCII text";;
	 2) file_test in.bash "Bourne-Again shell script";;
	 3) file_test in.sh "POSIX shell script, ASCII text executable" \
			    "POSIX shell script text executable" \
			    "POSIX shell script text" \
			    "Bourne shell script text executable";;
	 4) file_test in.ksh "Korn shell script";;
	 5) file_test in.csh "C shell script";;
	 6) file_test in.c "ASCII C program text" "C source, ASCII text";;
	 7) file_test in.pl "[pP]erl script, ASCII text executable" \
			    "[pP]erl script text executable" \
			    "a /usr/bin/perl script text";;
	 8) file_test in.py "[pP]ython3\{0,1\} script, ASCII text executable" \
			    "[pP]ython3\{0,1\} script text executable";;
	 9) file_test in.m4 "M4 macro processor script, ASCII text" \
			    "ASCII M4 macro language pre-processor text";;
	10) file_test in "ELF .*-bit $TEST_ARCH executable, .*" \
			 "ELF .*-bit $TEST_ARCH shared object, .*" \
			 "ELF .*-bit $TEST_ARCH pie executable, .*" \
			 "ELF .*-bit $TEST_ARCH pie shared object, .*";;
	11) file_test in.ar "current ar archive";;
	12) file_test in.tar "tar archive";;
	13) file_test in.tar.gz "gzip compressed data, .*";;
	14) file_test in.tar.bz2 "bzip2 compressed data, .*";;
	15) file_test in.src.rpm "RPM v3 src" "RPM v3.0 src";;
	16) file_test in.jpg "JPEG image data";;
	17) file_test in.png "PNG image data";;
	18) file_test in.wav "RIFF (little-endian) data, WAVE audio, Microsoft PCM";;
	19) file_test in.mp3 "MPEG ADTS, layer III";;
	20) file_test in.zip "Zip archive data";;
	esac
}

tst_run
