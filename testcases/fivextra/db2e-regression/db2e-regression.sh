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
# File :	db2e-regression.sh
#
# Description:	regression test of IBM's db2e
#
# Author:	This script by Robert Paulsen, rpaulsen@us.ibm.com
#		Uses tests provided by	Thomas Fanghänel,
#					DB2 Everyplace Development
#
# History:	Apr 07 2004 (rcp) Created

################################################################################
# source the standard utility functions
################################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

arch_dir=""	# filled in by tc_local_setup

################################################################################
# utility functions specific to this script
################################################################################

#
# tc_local_setup
#
#	This function is called automatically by the "tc_setup" function after
#	it has done the standard setup.
#
function tc_local_setup()
{
	tc_exec_or_break "tar diff uname" || return

	# copy test files to temp directory for execution
	tar -C $TCTMP -zxf db2e-regression.tar.gz
	cd $TCTMP/Regression
	export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH

	# figure out what architecture to use
	# TODO: This should be done at build time, but for now we are
	#	using pre-built binaries supplied by db2e team.
	uname -m | grep -q ppc && arch_dir=PowerPC ||
	uname -m | grep -q 86 && arch_dir=x86
	tc_break_if_bad $? "$(uname -m) is unsupported architecture" || return
	cp ${arch_dir}/* .
	tc_info "testing $arch_dir"

	# count number of tests
	TST_TOTAL=$(ls Results/Res*.db2e | wc -l)
	let TST_TOTAL+-2	# account for test01 and test02
}

#
# tc_local_cleanup
#
#	This function is called automatically when your testcase exits
#	for any reason. It is called before standard cleanup is performed.
#
function tc_local_cleanup()
{
	true	# nothing required yet
}

################################################################################
# the test functions
################################################################################

#
# test01	installation check
#
function test01()
{
	tc_register	"installation check"
	tc_warn "We are currently using db2e libraries supplied by the" \
		"db2e team. In the future MCP may include these libraries" \
		"and this testcase should be modified to use the system's" \
		"db2e libs instead of the ones included with this testcase."
	tc_pass_or_fail $?
}

#
# test02	execute the tests, comparing results will be done in afterward
#
function test02()
{
	tc_register "execute the tests"
	./Regression >$stdout 2>$stderr
	tc_pass_or_fail $? "bad response from test execution"
}

#
# test03	compare results
#
function test03()
{
	# for each expected result, compare to actual
	for e in Results/Res*.db2e ; do
		# e is name of expected result file
		# a is name of actual result file
		a=${e#Results/}; a=${a%.db2e}.LN

		tc_register "$e"
		diff -b -q $e $a >$stdout 2>$stderr
		tc_pass_or_fail $? \
			"expected ---------------------------" \
			"$(cat $e)" \
			"actual -----------------------------" \
			"$(cat $a)" \
			"------------------------------------"
	done
}

################################################################################
# main
################################################################################

tc_setup	# standard setup

test01 &&
test02 &&
test03
