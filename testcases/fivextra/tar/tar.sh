#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab:
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003						      ##
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
# File :	sh-utils.sg
#
# Description:	Test the functions provided by sh-utils.
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Feb 06 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		16 Dec 2003 - (robert) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# the testcase function
################################################################################

#
# test01	installation check
#
function test01()
{
	tc_register "is tar installed?"
	tc_executes tar
	tc_pass_or_fail $? "tar not installed"
}

#
# test02	tar/untar w/o compression
#
function test02()
{
	tc_register "tar/untar w/o compression"

	# create a small directory structure to tar up
	mkdir $TCTMP/tarme
	echo "Hello" > $TCTMP/tarme/hello.txt
	echo "Goodbye" > $TCTMP/tarme/goodbye.txt
	mkdir $TCTMP/tarme/subdir
	echo "White Rabbit" > $TCTMP/tarme/subdir/rabbit

	# tar without compression
	tar cf $TCTMP/tarme.tar -C $TCTMP tarme 2>$stderr
	tc_fail_if_bad $? "bad response from tar cf" || return

	# untar without compression
	mkdir $TCTMP/untar
	tar xf $TCTMP/tarme.tar -C $TCTMP/untar 2>$stderr
	tc_fail_if_bad $? "bad response from tar xf" || return

	# compare results
	diff -r $TCTMP/untar/tarme $TCTMP/tarme >$stderr 2>$stdout
	tc_pass_or_fail $? "bad results from diff of uncompressed tar"
}

#
# test03	tar/untar with -z compression
#
function test03()
{
	tc_register "tar/untar with -z compression"

	# busybox tar does not support compression
	if tc_is_busybox tar ; then
		tc_info "Skipped test of compression since"
		tc_info "it is not supported by busybox."
		cat /dev/null > $stderr
		tc_pass_or_fail 0 ""
		return
	fi
	# tar using "z" compression
	tar zcf $TCTMP/tarme.tar.gz -C $TCTMP tarme 2>$stderr
	tc_fail_if_bad $? "bad response from tar zcf" || return

	# untar using "z" compression
	mkdir $TCTMP/untarz
	tar zxf $TCTMP/tarme.tar.gz -C $TCTMP/untarz 2>$stderr
	tc_fail_if_bad $? "bad response from tar zxf" || return

	# compare results
	diff -r $TCTMP/untarz/tarme $TCTMP/tarme >$stdout 2>$stderr
	tc_pass_or_fail  $? "bad results from diff of z compressed tar"
}

#
# test04	tar/untar with -j compression
#
function test04()
{
	tc_register "tar/untar with -j compression"

	# busybox tar does not support compression
	if tc_is_busybox tar ; then
		tc_info "Skipped test of compression since"
		tc_info "it is not supported by busybox."
		cat /dev/null > $stderr
		tc_pass_or_fail 0 ""
		return
	fi

	# tar using "j" compression
	tar jcf $TCTMP/tarme.tar.bz2 -C $TCTMP tarme 2>$stderr
	tc_fail_if_bad $? "bad response from tar jcf" || return

	# untar using "j" compression
	mkdir $TCTMP/untarj
	tar jxf $TCTMP/tarme.tar.bz2 -C $TCTMP/untarj 2>$stderr
	tc_fail_if_bad $? "bad response from tar jxf" || return

	# compare results:
	diff -r $TCTMP/untarz/tarme $TCTMP/tarme >$stdout 2>$stderr
	tc_pass_or_fail $? "bad results from diff of j compressed tar"
}

################################################################################
# main
################################################################################

TST_TOTAL=4
tc_setup

tc_exec_or_break mkdir echo diff || exit

test01 || exit
test02 || exit
test03
test04
