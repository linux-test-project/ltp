#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2001		      ##
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
# File :	gpg.sh
#
# Description:	Test gpg capablities. Tests ported from gpg source.
#
# Author:	Robb Romans <robb@austin.ibm.com>
#
# History:	Mar 03 2003 - Created - RR
#		Mar 16 2003 - Modified by RCP
#			Now uses tc_utils.source
#			Runs both under harness and stand-alone.
#			Only modifies temp directory, not LTP's bin directory.
#			Does not assume root PATH has "."
#			Number of pases+fails now matches TST_COUNT.
#			gpg failure to import keys now is TFAIL, not TBROK.
#		06 Jan 2004 - (RR) updated to tc_utils.source

################################################################################
# source the standard utility functions
################################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

REQUIRED="which cmp cat ls rm"	# required executables

TESTS="version.test mds.test decrypt.test decrypt-dsa.test sigs.test \
	clearsig.test detach.test armsigs.test armdetach.test armdetachm.test \
	detachm.test conventional.test conventional-mdc.test"

# used by and for imported tests
export testdir=""	# directory for individual tests. (set by mysetup)
export srcdir=.		# directory testcases see.

################################################################################
# any utility functions specific to this file can go here
################################################################################

#
# setup specific to this file
#
function tc_local_setup()
{
	tc_exec_or_break $REQUIRED || return

	GPG_DEARMOR="gpg --no-options --no-greeting --batch --quiet --yes --dearmor"
	DATA_FILES="data-500 data-9000 data-32000 data-80000"

	# copy tests to temp directory
	cp -a $LTPBIN/checks $TCTMP/
	testdir=$TCTMP/checks

	# create current timestamp
	echo timestamp > $testdir/prepared.stamp

	# make required directories
	( cd $testdir && ./mkdemodirs &>/dev/null )

	# set up gpg testcase files
	tc_info "It is OK to see warnings about insecure memory"
	$GPG_DEARMOR -o $testdir/pubring.gpg $testdir/pubring.asc
	$GPG_DEARMOR -o $testdir/secring.gpg $testdir/secring.asc
	$GPG_DEARMOR -o $testdir/pubring.pkr $testdir/pubring.pkr.asc
	$GPG_DEARMOR -o $testdir/secring.skr $testdir/secring.skr.asc
	$GPG_DEARMOR -o $testdir/plain-1 $testdir/plain-1o.asc
	$GPG_DEARMOR -o $testdir/plain-2 $testdir/plain-2o.asc
	$GPG_DEARMOR -o $testdir/plain-3 $testdir/plain-3o.asc

	# create data files
	mk-tdata 500 >$testdir/data-500
	mk-tdata 9000 >$testdir/data-9000
	mk-tdata 32000 >$testdir/data-32000
	mk-tdata 80000 >$testdir/data-80000

	tc_info "local setup complete"
}

################################################################################
# the testcase functions
################################################################################

#
# test01	install check
#
function test01()
{
	tc_register "install check"
	tc_executes gpg
	tc_pass_or_fail "$?" "gpg not installed correctly" 
}

#
# test02	import keys
#
function test02()
{
	tc_register "import keys"

	GPG_IMPORT="gpg --homedir $testdir/ --quiet --yes --import"
	$GPG_IMPORT $testdir/pubdemo.asc
	tc_pass_or_fail "$?" "import failed" "rc=$?" 
}

#
# runtests	run the ported tests
#
function runtests()
{
	for tst in $TESTS; do
		tc_register $tst
		( cd $testdir && ./$tst &>$TCTMP/$tst.err )
		local -i rc=$?
		if [ $rc -eq 77 ] ; then
			let TST_TOTAL-=1
			let TST_COUNT-=1
			tc_info "$tst skipped on this platform"
		else
			tc_pass_or_fail $rc "unexpected results" "rc=$rc" \
				"`tail -20 $TCTMP/$tst.err`"
		fi
		rm -f $TCTMP/$tst.err &>/dev/null
	done
}

################################################################################
# main
################################################################################

set $TESTS
let TST_TOTAL=2+$#

tc_setup	# standard setup (exits if bad)

test01 && \
test02 && \
runtests
