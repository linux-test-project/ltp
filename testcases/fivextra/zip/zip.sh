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
# File :	zip.sh
#
# Description:	Test zip
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Jul 16 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#				Based on unzip.sh
#		16 Dec 2003 - (robert) updated to tc_utils.source

# source the utility functions
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
	tc_register	"installation check"
	tc_executes zip
	tc_pass_or_fail $? "zip not properly installed"
}

#
# test02	zip up a set of directories
#
function test02()
{
	tc_register	"zip"
	tc_exec_or_break tar || return

	tar zx -C $TCTMP -f $LTPBIN/tst_zip_dir.tgz
	tc_break_if_bad $? "failed to extract tar file to initialize test"

	zip -r $TCTMP/tst_zip_dir $TCTMP/tst_zip_dir >$stdout 2>$stderr
	tc_pass_or_fail $? "unexpected results from zip"
}

#
# test03	Test that unzip can uncompress .zip file.
#
function test03()
{
	tc_register "unzip"
	tc_exec_or_break diff || return

	# create expected output files. tst_unzip.exp
	cat > $TCTMP/tst_unzip.out.exp <<-EOF
		Archive: $TCTMP/tst_zip_dir.zip
		   creating: $TCTMP$TCTMP/tst_zip_dir/
		   creating: $TCTMP$TCTMP/tst_zip_dir/d.0/
		 extracting: $TCTMP$TCTMP/tst_zip_dir/d.0/f.0
		 extracting: $TCTMP$TCTMP/tst_zip_dir/d.0/f.1
		 extracting: $TCTMP$TCTMP/tst_zip_dir/d.0/f.2
		   creating: $TCTMP$TCTMP/tst_zip_dir/d.0/d.1/
		 extracting: $TCTMP$TCTMP/tst_zip_dir/d.0/d.1/f.0
		 extracting: $TCTMP$TCTMP/tst_zip_dir/d.0/d.1/f.1
		 extracting: $TCTMP$TCTMP/tst_zip_dir/d.0/d.1/f.2
		   creating: $TCTMP$TCTMP/tst_zip_dir/d.0/d.1/d.2/
		 extracting: $TCTMP$TCTMP/tst_zip_dir/d.0/d.1/d.2/f.0
		 extracting: $TCTMP$TCTMP/tst_zip_dir/d.0/d.1/d.2/f.1
		 extracting: $TCTMP$TCTMP/tst_zip_dir/d.0/d.1/d.2/f.2
	EOF

	unzip -d $TCTMP $TCTMP/tst_zip_dir >$stdout 2>$stderr
	tc_fail_if_bad $? "unzip failed" || return

	local expected="`cat $TCTMP/tst_unzip.out.exp`"
	diff -wbq $stdout $TCTMP/tst_unzip.out.exp
	tc_fail_if_bad $? "Expected the following in stdout" $'\n'"$expected" || return

	[ -d $TCTMP/tst_zip_dir ]
	tc_pass_or_fail $? "Unzip did not create subdirectory"
}

################################################################################
# main
################################################################################

TST_TOTAL=3

# standard tc_setup
tc_setup

test01 &&
test02 &&
test03

