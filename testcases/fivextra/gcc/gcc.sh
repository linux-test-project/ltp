#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2004						      ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MEECHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File :        gcc.sh
#
# Description:  Test gcc package
#
# Author:       
#
# History:      
#	
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# testcase functions
################################################################################

# Function:             runtests-compile
#
# Description:          - test  gcc compile 
#
# Parameters:           - none
#

function runtests-compile() {

local additional_flags=""
local torture_compile_xfail=""
local torture_compile_xfail_flag="0"
local gcc_compile_ret=""
local testdir="gcc-test/gcc.c-torture" # testdir we store individual test files in
local testfile 
local testcase_dir

# run each testcase in "compile" 
for tst in $testdir/compat/*.c $testdir/unsorted/*.c $testdir/compile/*.c ; do
	# We check the $tst.x file to make the xfail or options
	testfile=`basename $tst|cut -d "." -f 1`
	testcase_dir=`dirname $tst`
	torture_compile_xfail=""
	additional_flags=""
#we source the mcp-gcc-exclude.x to skip some testcase in some mcpversion
        [ -f gcc-test/mcp-gcc-exclude.x ] && source gcc-test/mcp-gcc-exclude.x
# we source the PERMFAILED testcase configuration file 
        [ -f $testcase_dir/${testfile}.x ] && source $testcase_dir/${testfile}.x
	# compile it using CFLAGS variable
	for gcc_cflags in  "-O0"  "-O1" "-O2" "-O3 -fomit-frame-pointer" " -O3 -g"  "-Os" ; do
		torture_compile_xfail_flag="0" && [ -n "$torture_compile_xfail" ] && isXFAIL "$torture_compile_xfail" "$gcc_cflags" && torture_compile_xfail_flag="1"
		tc_register "$GCC -w $gcc_cflags $additional_flags -c -o $TCTMP/gcctest.o $tst"
		[ "$torture_compile_xfail_flag" = "1" ] && tc_info "XFAIL test: $GCC -w  $gcc_cflags $additional_flags -c $tst"
		$GCC -w $gcc_cflags $additional_flags -c -o $TCTMP/gcctest.o $tst 2>$stderr >$stdout
		gcc_compile_ret=$?
		if [ "$torture_compile_xfail_flag" = "0" ] ;  then
			tc_pass_or_fail $gcc_compile_ret "output unexpected."
		else 
			[ "$gcc_compile_ret" = "$torture_compile_xfail_flag" ] 2>$stderr  >$stdout
			tc_pass_or_fail $? "expect XFAIL but it PASS. $gcc_compile_ret=$torture_compile_xfail_flag"
		fi 
	done

	if `grep "for*(\|while*(" $tst > /dev/null` ; then
		for gcc_cflags in "-O3 -fomit-frame-pointer -funroll-loops" "-O3 -fomit-frame-pointer -funroll-all-loops -finline-functions" ; do
		torture_compile_xfail_flag="0" && [ -n "$torture_compile_xfail" ] && isXFAIL "$torture_compile_xfail" "$gcc_cflags" && torture_compile_xfail_flag="1"
		tc_register "$GCC -w $gcc_cflags $additional_flags -c -o $TCTMP/gcctest.o $tst"
		[ "$torture_compile_xfail_flag" = "1" ] && tc_info "XFAIL test: $GCC -w $gcc_cflags $additional_flags $tst"
		$GCC  -w  $gcc_cflags $additional_flags -c -o $TCTMP/gcctest.o $tst 2>$stderr >$stdout
		gcc_compile_ret=$?
		if [ "$torture_compile_xfail_flag" = "0" ] ;  then
			tc_pass_or_fail $gcc_compile_ret "output unexpected."
		else 
			[ "$gcc_compile_ret" = "$torture_compile_xfail_flag" ] 2>$stderr  >$stdout
				tc_pass_or_fail $? "expect XFAIL but it PASS." 
			fi 
	done
	fi
done
}

# Function:             runtests-execute
#
# Description:          - test  gcc compile  and execute
#
# Parameters:           - none
#

function runtests-execute() {

local additional_flags=""
local torture_compile_xfail=""
local torture_execute_xfail=""
local torture_compile_xfail_flag="0"
local torture_execute_xfail_flag="0"
local gcc_compile_ret=""
local gcc_execute_ret=""
local testdir="gcc-test/gcc.c-torture" # testdir we store individual test files in
local testfile 
local testcase_dir

# run each testcase in "execute" 
for tst in $testdir/execute/*.c $testdir/execute/ieee/*.c ; do
#for tst in $testdir/test/*.c ; do
	# We check the $tst.x file to make the xfail or options
	testfile=`basename $tst|cut -d "." -f 1`
	testcase_dir=`dirname $tst`
	torture_compile_xfail=""
	torture_execute_xfail=""
	additional_flags=""
        [ -f gcc-test/mcp-gcc-exclude.x ] && source gcc-test/mcp-gcc-exclude.x
        [ -f $testcase_dir/${testfile}.x ] && source $testcase_dir/${testfile}.x
	# compile it using CFLAGS variable
	for gcc_cflags in  "-O0"  "-O1" "-O2" "-O3 -fomit-frame-pointer" " -O3 -g"  "-Os" ; do
		torture_compile_xfail_flag="0" && [ -n "$torture_compile_xfail" ] && isXFAIL "$torture_compile_xfail" "$gcc_cflags $additional_flags" && torture_compile_xfail_flag="1"
		tc_register " $GCC  -w  $gcc_cflags $additional_flags -lm  -o $TCTMP/gcctest.x1  $tst"
		[ "$torture_compile_xfail_flag" = "1" ] && tc_info "XFAIL test: $GCC  -w  $gcc_cflags $additional_flags $tst"
		$GCC  -w  $gcc_cflags $additional_flags -lm  -o $TCTMP/gcctest.x1 $tst 2>$stderr &>/dev/null
		gcc_compile_ret=$?
		if [ "$torture_compile_xfail_flag" = "0" ] ;  then
			tc_pass_or_fail $gcc_compile_ret "$tst output unexpected."
		else 
			[ "$gcc_compile_ret" = "$torture_compile_xfail_flag" ]  2>$stderr  >$stdout
			tc_pass_or_fail $? "$tst expect XFAIL but it PASS." 
		fi 
		if  [ "$gcc_compile_ret" = "0" -a "$torture_compile_xfail_flag" = "0" ]; then
		# test execute 
		torture_execute_xfail_flag="0" && [ -n "$torture_execute_xfail" ] && isXFAIL "$torture_execute_xfail" "$gcc_cflags $additional_flags" && torture_execute_xfail_flag="1"
		[ "$torture_execute_xfail_flag" = "1" ] && tc_info "XFAIL execute test: $GCC  -w -lm  $gcc_cflags $additional_flags $tst"
		tc_register " Execute the result of $GCC  -w  $gcc_cflags $additional_flags -lm $tst"
		$TCTMP/gcctest.x1 
		gcc_compile_ret=$?
		if [ "$torture_execute_xfail_flag" = "0" ] ;  then
			tc_pass_or_fail $gcc_compile_ret "$tst execute unexpected fail."
		else 
			[ $gcc_compile_ret -gt  0 ]  2>$stderr  >$stdout 
			tc_pass_or_fail $? "$tst expect XFAIL but it PASS." 
		fi 

		fi
	done

	if `grep  "for*(\|while*("  $tst > /dev/null` ; then
		for gcc_cflags in  "-O3 -fomit-frame-pointer -funroll-loops" "-O3 -fomit-frame-pointer -funroll-all-loops -finline-functions" ; do
		torture_compile_xfail_flag="0" && [ -n "$torture_compile_xfail" ] && isXFAIL "$torture_compile_xfail" "$gcc_cflags $additional_flags" && torture_compile_xfail_flag="1"
		tc_register " $GCC  -w  $gcc_cflags $additional_flags -lm  -o $TCTMP/gcctest.x1  $tst"
		[ "$torture_compile_xfail_flag" = "1" ] && tc_info "XFAIL test: $GCC  -w  $gcc_cflags $additional_flags $tst"
		$GCC  -w  $gcc_cflags $additional_flags -lm  -o $TCTMP/gcctest.x1 $tst 2>$stderr &>/dev/null
		gcc_compile_ret=$?
		if [ "$torture_compile_xfail_flag" = "0" ] ;  then
			tc_pass_or_fail $gcc_compile_ret "$tst output unexpected."
		else 
			[ "$gcc_compile_ret" = "$torture_compile_xfail_flag" ]  2>$stderr  >$stdout
			tc_pass_or_fail $? "$tst expect XFAIL but it PASS." 
		fi 
		if  [ "$gcc_compile_ret" = "0" -a "$torture_compile_xfail_flag" = "0" ]; then
		# test execute 
		torture_execute_xfail_flag="0" && [ -n "$torture_execute_xfail" ] && isXFAIL "$torture_execute_xfail" "$gcc_cflags $additional_flags " && torture_execute_xfail_flag="1"
		[ "$torture_execute_xfail_flag" = "1" ] && tc_info "XFAIL execute test: $GCC  -w  $gcc_cflags $additional_flags $tst"
		tc_register " Execute the result of $GCC  -w  $gcc_cflags $additional_flags -lm $tst"
		$TCTMP/gcctest.x1 
		gcc_compile_ret=$?
		if [ "$torture_execute_xfail_flag" = "0" ] ;  then
			tc_pass_or_fail $gcc_compile_ret "$tst execute unexpected fail."
		else 
			[ $gcc_compile_ret -gt  0 ] 2>$stderr  >$stdout 
			tc_pass_or_fail $? "$tst expect XFAIL but it PASS." 
		fi 

		fi
	done
	fi
done
}

# Function:             istarget
#
# Description:          - determin the target 
#
# Parameters:           - $1 is the string of expect target, such as i386 i686 powerpc ia64 i?86
#
# Return                - zero on success
#                       - return 1 means that it is not the expected target
#			- return 2 means that there is no argument
#
function istarget() {
	local expected_target=$1
	[ -n "$expected_target" ] || return 2
	if  `echo "$gcc_target"|grep "$expected_target-" > /dev/null ` ; then
                       return 0 
	else
		return 1
	fi

}

# Function:             isXFAIL
#
# Description:          - determin the XFAIL
#
# Parameters:           - $1 is the string of expect XFAIL options, such as "O2 O1 O3", $2 is the current gcc CFLAGS 
#
# Return                - zero on it is XFAIL(It just means XFAIL conditions match the CFLAGS)
#			- 1 on not XFAIL or paramenters missing

function isXFAIL() {
	local xfail_options=$1
	local compile_options=$2
	local tst_options

	[ -n "$compile_options" ] || return 1
	[ "$xfail_options" = "*" ] && return 0 
	for tst_options in $xfail_options ; do
	if  `echo "$compile_options"|grep "\-$tst_options" > /dev/null ` ; then
                       return 0 
	fi
	done
	return 1	
}
# Function:             istestcase
#
# Description:          - determin the testcase file name
#
# Parameters:           - $1 is the string of expect testcasefile
#
# Return                - zero on success
#                       - return 1 means that it is not the expected target
#			- return 2 means that there is no argument
#
function istestcase() {
	local expected_testcase=$1
	[ -n "$expected_testcase" ] || return 2
	if  [ "$tst" = "gcc-test/$expected_testcase" ] ; then
			tc_info "Skip the $tst test!"
                       return 0 
	else
		return 1
	fi

}

####################################################################################
# MAIN
####################################################################################

# optional command line argument specifies the gcc executable
GCC=$1
[[ "$GCC" ]] || GCC="gcc"

tc_setup		# exits if bad

tc_exec_or_break $GCC grep basename cut || exit

#check the gcc version and gcc info
gcc_target=`$GCC -dumpmachine `
gcc_version=`$GCC -dumpversion`
tc_info "We suppose that our gcc target is $gcc_target, gcc version $gcc_version"

runtests-compile
runtests-execute
