#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
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
# File :	pax.sh
#
# Description:	test the pax program
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Sep 04 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		16 Dec 2003 - (rcp) updated to tc_utils.source

################################################################################
# source the standard utility functions
################################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# any utility functions specific to this file can go here
################################################################################

################################################################################
# the testcase functions
################################################################################

#
# test01	installation check
#
function test01()
{
	tc_register	"pax installed?"
	tc_executes pax
	tc_pass_or_fail $? "pax is not installed properly"
}

#
# test02	tar format test
#
function test02()
{
	tc_register	"compressed tar format"

	# untar an existing tar file with pax
	cat topdir.tgz | pax -zr
	tc_fail_if_bad $? "unexpected response from cat topdir.tgz | pax -zr" || return
	
	tc_exec_or_break tar diff || return

	# do the same thing with tar
	mv topdir pax_topdir
	tar zxf topdir.tgz

	# compare the results
	diff -r topdir pax_topdir
	tc_pass_or_fail $? "miscompare of files produced by tar and pax"
}
#
# test03	cpio format
#
function test03()
{
	tc_register	"cpio format"
	tc_exec_or_break cpio diff || return

	rm -rf topdir		# probably left over from the above
	rm -rf pax_topdir	# probably left over from the above
	rm -rf new_topdir	# probably left over from the above

	# create original directory
	tar zxf topdir.tgz

	# use pax to create the cpio file
	pax -w topdir > topdir.cpio
	tc_fail_if_bad $? "unexpected response from pax -w topdir > topdir.cpio" || return

	# use cpio to extract from the fil made by cpio
	mv topdir orig_topdir	# save original for comparison
	cpio --extract < topdir.cpio &>$stdout

	# compare the results
	diff -r orig_topdir topdir >$stdout  2>$stderr
	tc_pass_or_fail $? "miscompare of directory created and extacted in cpio format"
}


#
# test04	pax copy directory
#
function test04()
{
	tc_register	"pax copy a directory"
	tc_exec_or_break tar diff || return

	rm -rf topdir		# probably left over from the above
	rm -rf pax_topdir	# probably left over from the above

	mkdir new_topdir	# destination of copy
	tar zxf topdir.tgz	# create a source directory
	cd topdir		# inside the source directory
	pax -rw . ../new_topdir	# copy the directory
	tc_fail_if_bad $? "unexpected response from pax -rw . ../new_top" || return

	# compare the results
	cd ..
	diff -r topdir new_topdir >$stdout 2>$stderr
	tc_pass_or_fail $? "miscompare of directory copied by pax"
}

################################################################################
# main
################################################################################

TST_TOTAL=4

(
	# standard tc_setup
	tc_setup

	cp topdir.tgz $TCTMP
	cd $TCTMP
	test01 &&
	test02
	test03
	test04
)
