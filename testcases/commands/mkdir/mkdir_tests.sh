#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2001                 ##
##  Author:       Manoj Iyer, manjo@mail.utexas.edu                           ##
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
# Basic mkdir tests
#
TST_ID="mkdir01"
TST_CNT=3
TST_SETUP=setup
TST_TESTFUNC=test
TST_NEEDS_TMPDIR=1
. tst_test.sh

setup()
{
	ROD mkdir "dir"
	LONG_PATH="some/long/path/of/several/directories"
}

test1()
{
	EXPECT_FAIL mkdir "dir" 2\> mkdir.out

	if grep -q "dir.*File exists" mkdir.out; then
		tst_res TPASS "Got correct error message"
	else
		tst_res TFAIL "Got wrong error message"
		cat mkdir.out
	fi
}

test2()
{
	EXPECT_FAIL mkdir "$LONG_PATH" 2\> mkdir.out

	if grep -q "$LONG_PATH.*No such file or directory" mkdir.out; then
		tst_res TPASS "Got correct error message"
	else
		tst_res TFAIL "Got wrong error message"
		cat mkdir.out
	fi

	ROD rm -rf "$LONG_PATH"
}

test3()
{
	EXPECT_PASS mkdir -p "$LONG_PATH"

	ROD rm -rf "$LONG_PATH"
}

tst_run
