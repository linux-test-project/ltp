#!/bin/bash
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2003						      ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File :        perltest.sh
#
# Description:  Test suite to exhaustively test perl
#
# Author:       Robb Romans <robb@austin.ibm.com>
#
# History:      Feb 11 2003 - 1st version
#		Apr 16 2003 - revised -RR
#		07 Jan 2004 - (RR) updated to tc_utils.source

################################################################################
# global variables
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

TESTDIR="$LTPBIN/t"	# directory for individual testcases
TEST_CMD="harness"	# name of test engine

BASE_TESTS="`ls $TESTDIR/base/*.t`"
CMD_TESTS="`ls $TESTDIR/cmd/*.t`"
COMP_TESTS="`ls $TESTDIR/comp/*.t`"
IO_TESTS="`ls $TESTDIR/io/*.t`"
LIB_TESTS="`ls $TESTDIR/lib/*.t`"
OP_TESTS="`ls $TESTDIR/op/*.t`"
PRAGMA_TESTS="`ls $TESTDIR/pragma/*.t`"
RUN_TESTS="`ls $TESTDIR/run/*.t`"

WHICH_TESTS="$BASE_TESTS $CMD_TESTS $COMP_TESTS $IO_TESTS $LIB_TESTS $OP_TESTS"

# required executables
REQUIRED="which basename cut grep ls perl rm tail"

################################################################################
# testcase functions
################################################################################

#
# Function run_tests
#
# Description	- run all perl standard testcases
#
# Return	- zero on success
#		- return value from testcase on failure ($RC)
function run_tests {
	set $WHICH_TESTS
	TST_TOTAL=$#

	local test
	local name
	for test in $WHICH_TESTS ; do
		name="`basename $test`"	
		tc_register "$name"
		perl $TESTDIR/$TEST_CMD $test &> $TCTMP/$name.out
		tc_pass_or_fail $? "`tail -20 $TCTMP/$name.out`"
		rm -f $name.out
	done
}


################################################################################
# main
################################################################################

# Function:	main
#
# Description:  Execute all tests, report results
#
# Exit:         zero on success
#               non-zero on failure
tc_setup
tc_exec_or_break $REQUIRED || exit
tc_info "Setting up perl test environment"
cd $LTPBIN

PERL_VER=$(perl -v | grep 'v5' | cut -d' ' -f4 | cut -c2-6)
tc_info "PERL_VERSION = $PERL_VER"

if [[ ! -L $LTPBIN/t/perl ]] ; then
        tc_info "Creating link to /bin/perl"
	ln -s $(which perl) $LTPBIN/t/perl 2>$stderr
	[[ $? == 0 ]]
	tc_break_if_bad $? "Unable to create symbolic link" || exit
fi
if [[ ! -L $LTPBIN/lib ]] ; then
        tc_info "Creating link to /usr/lib/perl5/$PERL_VER"
	ln -s /usr/lib/perl5/$PERL_VER $LTPBIN/lib 2>$stderr
	[[ $? == 0 ]]
	tc_break_if_bad $? "Unable to create library link" || exit
fi
for dir in $LTPBIN/t/* ; do
	if [[ -d $dir ]] && [[ ! -L $dir/perl ]] ; then
		ln -s $(which perl) $dir/perl
	fi
done

run_tests

