#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2001                 ##
##  Author: Manoj Iyer, manjo@mail.utexas.edu                                 ##
## Copyright (c) Cyril Hrubis <chrubis@suse.cz>                               ##
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
# Test basic functionality of cpio command
#
TST_ID="cpio01"
TST_TESTFUNC=cpio_test
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="cpio"
. tst_test.sh

cpio_test()
{
	ROD mkdir "dir"
	for i in a b c d e f g h i j k l m n o p q r s t u v w x y z; do
		ROD echo "Test" > "dir/$i"
	done

	ROD find dir -type f > filelist
	EXPECT_PASS cpio -o \> cpio.out \< filelist
	ROD mv "dir" "dir_orig"
	ROD mkdir "dir"
	EXPECT_PASS cpio -i \< cpio.out

	if diff -r "dir" "dir_orig"; then
		tst_res TPASS "Directories dir and dir_orig are equal"
	else
		tst_res TFAIL "Directories dir and dir_orig differ"
		ls -R dir_orig
		echo
		ls -R dir
	fi
}

tst_run
