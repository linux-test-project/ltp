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
# File :	unzip.sh
#
# Description:	Test unzip
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Feb 06 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		Feb 10 2003 - Updates after code review.
#		16 Dec 2003 - (robert) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# the testcase functions
################################################################################

#
# test01	Test that unzip can uncompress .zip file.
#
function test01()
{
	tc_register "uncompress"
	tc_exec_or_break diff || return

	# create expected output files. tst_unzip.exp
	cat > $TCTMP/tst_unzip.out.exp <<-EOF
		    Archive: $LTPBIN/tst_unzip_file.zip
		   creating: $TCTMP/tmp/tst_unzip.dir/
		   creating: $TCTMP/tmp/tst_unzip.dir/d.0/
		 extracting: $TCTMP/tmp/tst_unzip.dir/d.0/f.0
		 extracting: $TCTMP/tmp/tst_unzip.dir/d.0/f.1
		 extracting: $TCTMP/tmp/tst_unzip.dir/d.0/f.2
		   creating: $TCTMP/tmp/tst_unzip.dir/d.0/d.1/
		 extracting: $TCTMP/tmp/tst_unzip.dir/d.0/d.1/f.0
		 extracting: $TCTMP/tmp/tst_unzip.dir/d.0/d.1/f.1
		 extracting: $TCTMP/tmp/tst_unzip.dir/d.0/d.1/f.2
		   creating: $TCTMP/tmp/tst_unzip.dir/d.0/d.1/d.2/
		 extracting: $TCTMP/tmp/tst_unzip.dir/d.0/d.1/d.2/f.0
		 extracting: $TCTMP/tmp/tst_unzip.dir/d.0/d.1/d.2/f.1
		 extracting: $TCTMP/tmp/tst_unzip.dir/d.0/d.1/d.2/f.2
	EOF
	unzip -d $TCTMP $LTPBIN/tst_unzip_file.zip \
		2>$stderr >$TCTMP/tst_unzipped.out
	tc_fail_if_bad $? "unzip failed" || return

	local expected="expected:"$'\n'"`cat $TCTMP/tst_unzip.out.exp`"
	local actual="got:"$'\n'"`cat $TCTMP/tst_unzipped.out`"
	diff -iwb $TCTMP/tst_unzipped.out $TCTMP/tst_unzip.out.exp >/dev/null
	tc_fail_if_bad $? "Bad stdout from unzip." "$expected" "$actual" || return

	[ -d $TCTMP/tmp/tst_unzip.dir ]
	tc_pass_or_fail $? "Unzip did not create subdirectory"
}

################################################################################
# main
################################################################################

# standard tc_setup
tc_setup

TST_TOTAL=1
test01

