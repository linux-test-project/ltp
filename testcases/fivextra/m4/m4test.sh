#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
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
## WITHOUT ANY WARRANTY; without even the implied warranty of MEECHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File :        m4test.sh
#
# Description:  Test m4 package
#
# Author:      CSDL  hejianj@cn.ibm.com
#
# History:      12 Mar 2004 created.
#		15 Mar 2004 changed.
#		07 May 2004 (rcp) make command is not required. Also, m4
#			should not be listed as a required command. If m4
#			is missing we want the test to FAIL, not BROK.
#	
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# environment functions
################################################################################

#
# local setup
#
function tc_local_setup()
{
	tc_exec_or_break cmp test || return
	test -z "$MAKE" && MAKE=make
	test -z "$M4" && M4=/usr/bin/m4
	test -z "$CMP" && CMP=cmp
#	test -z "$VERBOSE" && {exec > /dev/null 2>&1}

# Setting nls related vars. Override them in the test when needed.
	export srcdir=m4test
	export M4
	export CMP
	LANGUAGE=C
	export LANGUAGE
	LC_ALL=C
	export LC_ALL
	LANG=C
	export LANG
}

#
# local cleanup
#
function tc_local_cleanup
{
	rm -f ok okerr out err in
}

################################################################################
# testcase functions
################################################################################

# run the m4testcases
function run_m4test() 
{
# run each testcase
for tst in $TESTS ; do
	tst=${tst##*m4test/}
	tc_register "$tst"

	cat /dev/null > ok
	cat /dev/null > okerr
	cat /dev/null > out
	cat /dev/null > err

	$TESTDIR/$tst
	tc_pass_or_fail $? "output unexpected." || {
		[ -s ok ] && tc_info \
			"===================== expected stdout ===================" \
			"$(cat ok)" \
			"====================== actual stdout ====================" \
			"$(cat out)" \
			"========================================================="
		[ -s okerr ] && tc_info \
			"===================== expected stderr ===================" \
			"$(cat okerr)" \
			"====================== actual stderr ====================" \
			"$(cat err)" \
			"========================================================="
	}
done
}

################################################################################
# MAIN
################################################################################

TESTDIR="m4test" # testdir we store individual test files in
TESTS=`find ./$TESTDIR -type f -name "*.test"`

set $TESTS
TST_TOTAL=$#

tc_setup
run_m4test
