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
# File :	jpeg.sh
#
# Description:	Test the jpeg package
#		Tests adapted from "make check" in the source tree.
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Sep 04 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		Oct 13 2003 - RC Paulsen. Exit test if installation check fails
#				(BUG 4896)
#		16 Dec 2003 (rcp) updated to tc_utils.source

################################################################################
# source the standard utility functions
################################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# the testcase functions
################################################################################

#
# test01	installation check
#
function test01()
{
	tc_register	"is jpeg installed"
	tc_executes djpeg cjpeg jpegtran 
	tc_pass_or_fail $? "jpeg not installed properly"
}

#
# test02	"jpg to ppm:
#
function test02()
{
	tc_register	"jpg to ppm"
	tc_exec_or_break cmp || return
	local cmd="djpeg -dct int -ppm -outfile testout.ppm  ./testorig.jpg"
	$cmd >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected response from $cmd" || return
	cmp ./testimg.ppm testout.ppm >$stdout 2>$stderr
	tc_pass_or_fail $? "bad compare of image files"
}

#
# test03	"jpg to bmp:
#
function test03()
{
	tc_register	"jpg to bmp"
	tc_exec_or_break cmp || return
	local cmd="djpeg -dct int -bmp -colors 256 -outfile testout.bmp  ./testorig.jpg"
	$cmd >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected response from $cmd" || return
	cmp ./testimg.bmp testout.bmp >$stdout 2>$stderr
	tc_pass_or_fail $? "bad compare of image files"
}

#
# test04	"ppm to jpg:
#
function test04()
{
	tc_register	"ppm to jpg"
	tc_exec_or_break cmp || return
	local cmd="cjpeg -dct int -outfile testout.jpg  ./testimg.ppm"
	$cmd >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected response from $cmd" || return
	cmp ./testimg.jpg testout.jpg >$stdout 2>$stderr
	tc_pass_or_fail $? "bad compare of image files"
}

#
# test05	"jpg to ppm"
#
function test05()
{
	tc_register	"jpg to ppm"
	tc_exec_or_break cmp || return
	local cmd="djpeg -dct int -ppm -outfile testoutp.ppm ./testprog.jpg"
	$cmd >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected response from $cmd" || return
	cmp ./testimg.ppm testoutp.ppm >$stdout 2>$stderr
	tc_pass_or_fail $? "bad compare of image files"
}

#
# test06	"ppm to jpg progressive"
#
function test06()
{
	tc_register	"ppm to jpg progressive"
	tc_exec_or_break cmp || return
	local cmd="cjpeg -dct int -progressive -opt -outfile testoutp.jpg ./testimg.ppm"
	$cmd >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected response from $cmd" || return
	cmp ./testimgp.jpg testoutp.jpg >$stdout 2>$stderr
	tc_pass_or_fail $? "bad compare of image files"
}

#
# test07	"jpg to jpg"
#
function test07()
{
	tc_register	"jpg to jpg"
	tc_exec_or_break cmp || return
	local cmd="jpegtran -outfile testoutt.jpg ./testprog.jpg"
	$cmd >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected response from $cmd" || return
	cmp ./testorig.jpg testoutt.jpg >$stdout 2>$stderr
	tc_pass_or_fail $? "bad compare of image files"
}

################################################################################
# main
################################################################################

TST_TOTAL=7

(
	# standard tc_setup
	tc_setup
	cp jpg_test_images/* $TCTMP
	cd $TCTMP

	test01 || exit
	test02
	test03
	test04
	test05
	test06
	test07
)
