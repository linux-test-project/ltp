#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab:
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2001		      ##
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
# File :	base.sh
#
# Description:	Test the functions provided by the "base" package
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Feb 06 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		Jul 18 2003 - (rcp) This testcase should not be run on MCP 1.1
#				or MCP 2.0. It has been split into indvidual
#				tests: sed, tar, time, ksymoops.
#		15 Dec 2003 (robert) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

# commands to be tested
names="sed tar time freeramdisk klogconsole ksymoops"

################################################################################
# local utility functions
################################################################################

#
# report unimplemented test
# these are tests that should be implemented eventually
#
#       $@      any additional text to print
#
function unimplemented()
{
        tst_resm TWARN "$TCNAME: not yet implemented. $@"
}

#
# Report untested capability.
# These are tests that will not be tested since they would be
# destructive to the system.
#
function declined()
{
        let TST_TOTAL-=1        # one less test than originally counted
        let TST_COUNT-=1        # one less test than originally counted
        tc_info "$TCNAME: unsafe, declined"
}

################################################################################
# the testcase functions
################################################################################

function do_sed()
{
	tc_exec_or_break cat chmod diff || return
#
	# create source file
	cat > $TCTMP/sed_input <<-EOF
		line one
		line two
		line three
	EOF
#
	# create sed script file
	local sed_cmd="`which sed`"
	cat > $TCTMP/sed_script <<-EOF
		#!$sed_cmd -f
		/two/s/line/LINE/
		/one/a\\
		first inserted line\\
		second inserted line
	EOF
	chmod +x $TCTMP/sed_script
#
	# execute the sed command
	$TCTMP/sed_script $TCTMP/sed_input >$TCTMP/sed_output 2>$stderr
	tc_fail_if_bad $? "bad rc ($?) from sed" || return
#
	# create file of expected results
	cat > $TCTMP/sed_exp <<-EOF
		line one
		first inserted line
		second inserted line
		LINE two
		line three
	EOF
#
	# compare actual and expected results
	diff $TCTMP/sed_exp $TCTMP/sed_output >$TCTMP/sed_diff
	tc_pass_or_fail $? "actual and expected results do not compare" \
		"expected..."$'\n'"`cat $TCTMP/sed_exp`" \
		"actual..."$'\n'"`cat $TCTMP/sed_output`" \
		"difference..."$'\n'"`cat $TCTMP/sed_diff`"
}

function do_tar()
{
	tc_exec_or_break mkdir echo tar diff || return
#
	# create a small directory structure to tar up
	mkdir $TCTMP/tarme
	echo "Hello" > $TCTMP/tarme/hello.txt
	echo "Goodbye" > $TCTMP/tarme/goodbye.txt
	mkdir $TCTMP/tarme/subdir
	echo "White Rabbit" > $TCTMP/tarme/subdir/rabbit
#
	# tar without compression
	tar cf $TCTMP/tarme.tar -C $TCTMP tarme 2>$stderr
	tc_fail_if_bad $? "bad response from tar cf" "rc=$?" || return
#
	# untar without compression
	mkdir $TCTMP/untar
	tar xf $TCTMP/tarme.tar -C $TCTMP/untar 2>$stderr
	tc_fail_if_bad $? "bad response from tar xf" "rc=$?" || return
#
	# compare results:
	local result="`diff -r $TCTMP/untar/tarme $TCTMP/tarme`"
	tc_fail_if_bad $? "bad results from diff of uncompressed tar" "rc=$?" \
		"diff output:"$'\t'"$result" || return
#
	# tar using "z" compression
	tar zcf $TCTMP/tarme.tar.gz -C $TCTMP tarme 2>$stderr
	local rc=$?
	if cat $stderr | grep "not internally support.*busybox" >/dev/null
	then
		tc_info "Skipped test of compression since"
		tc_info "it is not supported by busybox."
		cat /dev/null > $stderr
		tc_pass_or_fail 0 ""
		return
	fi
	tc_fail_if_bad $rc "bad response from tar zcf" "rc=$?" || return
#
	# untar using "z" compression
	mkdir $TCTMP/untarz
	tar zxf $TCTMP/tarme.tar.gz -C $TCTMP/untarz 2>$stderr
	tc_fail_if_bad $? "bad response from tar zxf" "rc=$?" || return
#
	# compare results:
	result="`diff -r $TCTMP/untarz/tarme $TCTMP/tarme`"
	tc_fail_if_bad $? "bad results from diff of z compressed tar" "rc=$?" \
		"diff output:"$'\t'"$result" || return
#
	# tar using "j" compression
	tar jcf $TCTMP/tarme.tar.bz2 -C $TCTMP tarme 2>$stderr
	tc_fail_if_bad $? "bad response from tar jcf" "rc=$?" || return
#
	# untar using "j" compression
	mkdir $TCTMP/untarj
	tar jxf $TCTMP/tarme.tar.bz2 -C $TCTMP/untarj 2>$stderr
	tc_fail_if_bad $? "bad response from tar jxf" "rc=$?" || return
#
	# compare results:
	result="`diff -r $TCTMP/untarz/tarme $TCTMP/tarme`"
	tc_pass_or_fail $? "bad results from diff of j compressed tar" "rc=$?" \
		"diff output:"$'\t'"$result"
}

function do_time()
{
	tc_exec_or_break sleep grep || return
#
	# time sleep 5
	tc_info "timing \"sleep 5\""
	local result="`2>&1 time -p sleep 5`"	# normal output is to stderr!
	echo $result | grep "^real *5" >/dev/null
	tc_pass_or_fail $? "expected to see approx 5 seconds in:" \
		"$resultS"
}

function do_freeramdisk()
{
	tc_exec_or_break || return
#
	declined
}

function do_klogconsole()
{
	tc_exec_or_break || return
#
	tc_info "can be checked manually by"
	tc_info "pressing Ctrl-Alt-F10"
	unimplemented
}

function do_ksymoops()
{
	tc_root_or_break || return
	tc_exec_or_break ls || return
#
	local mapfile="`ls /boot/ | grep System.map`"
	$command 2>$stderr | grep "$mapfile"
	local results="`echo -n \"\" | ksymoops -VKLO -m /boot/$mapfile`"
	#echo -n "" |  ksymoops -VKLO -m /boot/$mapfile | grep x$mapfile
	echo -n "" |  ksymoops -m /boot/$mapfile | grep $mapfile >/dev/null
	tc_pass_or_fail $? "Unexpected output." \
		"Expected to see"$'\n'"$mapfile" \
		"in"$'\n'"$results" "rc=$?"
}

################################################################################
# main
################################################################################

tc_setup

TST_TOTAL=0
for i in $names ; do
	let TST_TOTAL+=1
done

#
# run tests against all commands in sh-utils package.
#
for TCNAME in $names ; do
	tc_register $TCNAME
	do_$TCNAME
done
