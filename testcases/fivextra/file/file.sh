#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2004						      ##
##									      ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or	      ##
## (at your option) any later version.					      ##
##									      ##
## This program is distributed in the hope that it will be useful, but	      ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.							      ##
##									      ##
## You should have received a copy of the GNU General Public License	      ##
## along with this program;  if not, write to the Free Software		      ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##									      ##
################################################################################
#
# File:           file.sh
#
# Description: This program tests the file command. The tests are aimed at
#              testing if the file command can recognize some of the commonly
#              used file formats like, tar, tar.gz, rpm, C, ASCII, ELF etc. 
#
# Author:      Robert Paulsen. Based on ideas by Manoj Iyer
#
# History:     May 11 2004 - Created.
#
# TODO: rmp and vmlinuz (but probably not worth the effort) 

################################################################################
# source the standard utility functions
################################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

REQUIRED="grep"
ME=$0

################################################################################
# utility functions specific to this script
################################################################################

#
# tc_local_setup
#
function tc_local_setup()
{
	tc_exec_or_break $REQUIRED
}

#
# tc_local_cleanup
#
function tc_local_cleanup()
{
	true	# nothing here yet!
}

################################################################################
# the test functions
################################################################################

#
# test01	check that file is installed
#
function test01()
{
	tc_register	"installation check"
	tc_executes file
	tc_pass_or_fail $? "file not installed"
}

#
# test02	ASCII text
#
function test02()
{
	local expected="ASCII English text"
	tc_register	"$expected"
	cat > $TCTMP/testfile02.xyz <<-EOF
		The time has come, the Walrus said, to talk of many things
		Of shoes and ships and sealing wax, of cabbages and kings,
		Of why the sea is boiling hot and whether pigs have wings.
	EOF
	file $TCTMP/testfile02.xyz >$stdout 2>$stderr
	grep -q "$expected" $stdout 2>>$stderr
	tc_pass_or_fail $? "expected to see \"$expected\" in output"
}

#
# test03	bash shell script
#
function test03()
{
	local expected="Bourne-Again shell script"
	tc_register	"$expected"
	cat > $TCTMP/testfile03.xyz <<-EOF
		#!/bin/bash
		echo "Hello, Sailor!"
	EOF
	file $TCTMP/testfile03.xyz >$stdout 2>$stderr
	grep -q "$expected" $stdout 2>>$stderr
	tc_pass_or_fail $? "expected to see \"$expected\" in output"
}

#
# test04	korn shell script
#
function test04()
{
	local expected="Korn shell script"
	tc_register	"$expected"
	cat > $TCTMP/testfile04.xyz <<-EOF
		#!/bin/ksh
		echo "Hello, Sailor!"
	EOF
	file $TCTMP/testfile04.xyz >$stdout 2>$stderr
	grep -q "$expected" $stdout 2>>$stderr
	tc_pass_or_fail $? "expected to see \"$expected\" in output"
}

#
# test05	C shell script
#
function test05()
{
	local expected="C shell script"
	tc_register	"$expected"
	cat > $TCTMP/testfile05.xyz <<-EOF
		#!/bin/csh
		echo "Hello, Sailor!"
	EOF
	file $TCTMP/testfile05.xyz >$stdout 2>$stderr
	grep -q "$expected" $stdout 2>>$stderr
	tc_pass_or_fail $? "expected to see \"$expected\" in output"
}

#
# test06	ASCII C program text
#
function test06()
{
	local expected="ASCII C program text"
	tc_register	"$expected"
	cat > $TCTMP/testfile06.xyz <<-EOF
		#include <stdio.h>
		int main()
		{
			printf("Hello, Sailor!");
			return 0;
		}
	EOF
	file $TCTMP/testfile06.xyz >$stdout 2>$stderr
	grep -q "$expected" $stdout 2>>$stderr
	tc_pass_or_fail $? "expected to see \"$expected\" in output"
}

#
# test07	ELF executable
#
function test07()
{
	tc_register	"ELF Executable"
	file file-test-cprog >$stdout 2>$stderr
	grep -q "ELF .*executable" $stdout 2>>$stderr
	tc_pass_or_fail $? "expected to see \"ELF ... executable\" in output"
}

#
# test08	tar file
#
function test08()
{
	local expected="GNU tar archive"
	tc_register	"$expected"
	file file-tests.tar >$stdout 2>$stderr
	grep -q "$expected" $stdout 2>>$stderr
	tc_pass_or_fail $? "expected to see \"$expected\" in output"
}

#
# test09	tar gzip file
#
function test09()
{
	local expected="GNU tar archive"
	tc_register	"gzipped $expected"
	file file-tests.tgz >$stdout 2>$stderr
	grep -q "$expected" $stdout 2>>$stderr
	tc_pass_or_fail $? "expected to see \"$expected\" in output"
}

#
# test10	gziped file
#
function test10()
{
	local expected="gzip compressed data"
	tc_register	"gzipped $expected"
	file file-test.xxx.gz >$stdout 2>$stderr
	grep -q "$expected" $stdout 2>>$stderr
	tc_pass_or_fail $? "expected to see \"$expected\" in output"
}

################################################################################
# main
################################################################################

TST_TOTAL=10	# there are three tests in this testcase

tc_setup	# standard setup

test01 || exit
test02
test03
test04 
test05 
test06 
test07 
test08 
test09 
test10
