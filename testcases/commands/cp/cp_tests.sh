#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2001                 ##
##  Author:       Manoj Iyer, manjo@mail.utexas.edu                           ##
## Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>                          ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software Foundation,   ##
## Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           ##
##                                                                            ##
################################################################################
#
# Tests basic cp functionality
#
TST_ID="ln01"
TST_CNT=5
TST_TESTFUNC=do_test
TST_SETUP=setup
TST_NEEDS_TMPDIR=1
. tst_test.sh

create_tree()
{
	local dirname=$1
	local dircnt=$2
	local filecnt=$3

	tst_res TINFO "Creating $dircnt directories."
	tst_res TINFO "Filling each dir with $filecnt files".
	while [ $dircnt -gt 0 ]; do
		dirname=$dirname/dir$dircnt
	        ROD mkdir -p $dirname

		local fcnt=0
	        while [ $fcnt -lt $filecnt ]; do
			ROD touch $dirname/file$fcnt
			fcnt=$((fcnt+1))
		done
		dircnt=$((dircnt-1))
	done
}

setup()
{
	create_tree "dir" 10 10
	ROD echo LTP > file
}

compare_dirs()
{
	local src="$1"
	local dst="$2"

	if diff -r $src $dst; then
		tst_res TPASS "Directories $src and $dst are equal"
	else
		tst_res TFAIL "Directories $src and $dst differs"
		ls -R $src
		echo
		ls -R $dst
	fi
}

compare_files()
{
	local src="$1"
	local dst="$2"

	if diff $src $dst; then
		tst_res TPASS "Files $src and $dst are equal"
	else
		tst_res TFAIL "Files $src and $dst differs"
	fi
}

cp_test()
{
	local args="$1"
	local src="$2"
	local dst="$3"
	EXPECT_PASS cp $args $src $dst
	if [ -f $src ]; then
		compare_files $src $dst
	else
		compare_dirs $src $dst
	fi
	ROD rm -r $dst
}

do_test()
{
	case $1 in
	1) cp_test ""  "file" "file_copy";;
	2) cp_test -l  "file" "file_copy";;
	3) cp_test -s  "file" "file_copy";;
	4) cp_test -R  "dir"  "dir_copy";;
	5) cp_test -lR "dir"  "dir_copy";;
	esac
}

tst_run
