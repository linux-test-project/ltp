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
# File :	gdb.sh
#
# Description:	Test that gdb are used properly
#
# Author:	Jue Xie, xiejue@cn.ibm.com
#

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# main
################################################################################

tc_setup			# standard tc_setup
tc_exec_or_break expect cat chmod grep gdb  || exit 1
tc_info "`gdb -v`"
expcmd=`which expect`

gdb_tests="run all-types int-type annota1 bitfields break call-ar-st call-strs cvexpr long_long exprs"
for testfile in $gdb_tests ; do
	tc_exec_or_break gdb.base/$testfile  || exit 1
	tc_info "Begin gdb.base/$testfile.exp to test gdb.base/$testfile"
	tc_register	"gdb test $testfile"
	$expcmd -f gdb.base/$testfile.exp gdb.base/$testfile >$stdout 2>$stderr
	tc_pass_or_fail $? "gdb test $testfile FAIL " 
done

tc_info "Begin the gdb test using gdb.base/bitops.exp"
tc_register	"gdb test bitops.exp "
$expcmd -f gdb.base/bitops.exp >$stdout 2>$stderr
tc_pass_or_fail $? "gdb test $testfile FAIL " 
