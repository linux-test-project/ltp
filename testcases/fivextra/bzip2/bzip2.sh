#!/bin/bash
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
#									      ##
# File :	bzip2.sh						      ##
#									      ##
# Description:	Test basic functionality of bzip2/bunzip2 commands	      ##
#									      ##
# Test Method:	Use existing reference files to test bzip2 and bunzip2	      ##
#									      ##
# -Existing reference files:						      ##
#(Note: All these files are copied from $LTPROOT/testcases/bin to $TMP in the ##
#	command file - $LTPROOT/runtest/fivextra. )			      ##
#									      ##
#  uncompresssed: sample1.ref, sample2.ref, sample3.ref			      ##
#  compressed	: sample1.bz2, sample2.bz2, sample3.bz2			      ##
#									      ##
# -In the testcases:							      ##
#  Test#1: bzip2 sample1.ref------output--->sample1.rb2			      ##
#	   bunzip2 sample1.bz2 ---output--->sample1.tst			      ##
#	   compare sample1.ref sample1.tst				      ##
#	   compare sample1.bz2 sample1.rb2				      ##
#	   if both "compare" pass, test is successful			      ##
#	   Test#2 and Test#3 use similar method but with different files      ##
#									      ##
# Author:	Yu-Pao Lee						      ##
#									      ##
# History:	Feb 06 2003 - Created - Yu-Pao Lee.			      ##
#		Feb 17 2003 - added new functions and descriptions	      ##
#		Mar 20 2003 - changed to source the untility functions	      ##
#		17 Dec 2003 - (rcp) updated to tc_utils.source		      ##
################################################################################


# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# the testcase functions
################################################################################

function test01()
{
	tc_register "installation check"
	tc_executes bzip2 bunzip2
	tc_pass_or_fail $? "bzip2 not installed properly"
}

function test02()		# test bzip2 and bunzip2
{
	local infile
	local outfile

	tc_register "test bzip2 -1 and bunzip2"

	# check that required commands are present
	tc_exec_or_break cp cmp || return

	cp -p $LTPBIN/sample* $TCTMP 

	# ensure required files exist before testing
	tc_exist_or_break $TCTMP/sample1.ref $TCTMP/sample1.bz2 \
		$TCTMP/sample2.ref $TCTMP/sample2.bz2 \
		$TCTMP/sample3.ref $TCTMP/sample3.bz2  || return

	tc_info "Using bzip2 -1 to compress file..."
	infile=$TCTMP/sample1.ref
	outfile=$TCTMP/sample1.rb2
	bzip2 -1 < $infile > $outfile 2>$stderr
	tc_fail_if_bad $? "bzip2 -1 failed" || return
	
	tc_info "Using bunzip2 to decompress files..."
	infile=$TCTMP/sample1.bz2
	outfile=$TCTMP/sample1.tst
	bunzip2 < $infile > $outfile 2>$stderr
	tc_fail_if_bad $? "bunzip2 failed" || return

	tc_info "1st compare..."
	infile=$TCTMP/sample1.ref
	outfile=$TCTMP/sample1.tst
	cmp $infile $outfile 2>$stderr
	tc_fail_if_bad $? "cmp sample1.ref sample1.tst failed" || return

	tc_info "2nd compare..."
	infile=$TCTMP/sample1.bz2
	outfile=$TCTMP/sample1.rb2
	cmp $infile $outfile 2>$stderr
	tc_pass_or_fail $? "cmp sample1.bz2 sample1.rb2 failed"
}


function test03()
{
	local infile
	local outfile

	tc_register "test bzip2 -2 and bunzip2"

	tc_info "Using bzip2 -2 to compress file...."
	infile=$TCTMP/sample2.ref
	outfile=$TCTMP/sample2.rb2
	bzip2 -2 < $infile > $outfile 2>$stderr
	tc_fail_if_bad $? "bzip2 -2 failed" || return

	tc_info "Using bunzip2 to decompress file...."
	infile=$TCTMP/sample2.bz2
	outfile=$TCTMP/sample2.tst
	bunzip2 < $infile > $outfile 2>$stderr
	tc_fail_if_bad $? "bunzip2 command failed" || return

	tc_info "1st compare..."
	infile=$TCTMP/sample2.ref
	outfile=$TCTMP/sample2.tst
	cmp $infile $outfile 2>$stderr
	tc_fail_if_bad $? "cmp sample2.ref sample2.tst failed" || return

	tc_info "2nd compare..."
	infile=$TCTMP/sample2.bz2
	outfile=$TCTMP/sample2.rb2
	cmp $infile $outfile 2>$stderr
	tc_pass_or_fail $? "cmp sample2.bz2 sample1.rb2 failed"
}


function test04()
{
	local infile
	local outfile

	tc_register "test bzip2 -3 and bunzip2 -s"

	tc_info "Using bzip2 -3 to compress file...."
	infile=$TCTMP/sample3.ref
	outfile=$TCTMP/sample3.rb2
	bzip2 -3 < $infile > $outfile 2>$stderr
	tc_fail_if_bad $? "bzip2 -3 failed" || return

	tc_info "Using bunzip2 -s to decompress file...."
	infile=$TCTMP/sample3.bz2
	outfile=$TCTMP/sample3.tst
	bunzip2 -s < $infile > $outfile 2>$stderr
	tc_fail_if_bad $? "$TCTMP/bzip.err" "bunzip2 command failed" || return

	tc_info "1st compare..."
	infile=$TCTMP/sample3.ref
	outfile=$TCTMP/sample3.tst
	cmp $infile $outfile 2>$stderr
	tc_fail_if_bad $? "cmp sample3.ref sample3.tst failed" || return

	tc_info "2nd compare..."
	infile=$TCTMP/sample3.bz2
	outfile=$TCTMP/sample3.rb2
	cmp $infile $outfile 2>$stderr
	tc_pass_or_fail $? "cmp samp1e3.bz2 sample3.rb2 failed"
}

################################################################################
# Function    :  main
################################################################################

TST_TOTAL=4

# standard tc_setup
tc_setup

test01 &&
test02 &&
test03 &&
test04
