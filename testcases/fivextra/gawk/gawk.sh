#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003		      ##
##									      ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or	      ##
## (at your option) any later version.					      ##
##									      ##
## This program is distributed in the hope that it will be useful, but	      ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MEECHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.							      ##
##									      ##
## You should have received a copy of the GNU General Public License	      ##
## along with this program;  if not, write to the Free Software		      ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##									      ##
################################################################################
#
# File :	gawk.sh
#
# Description:	Test gawk package
#
# Author:	Robb Romans <robb@austin.ibm.com>
#
# History:	Mar 18 2003 - Created - RR
#		Apr 07 2003 - Modified per team review
#
#
#		06 Jan 2004 - (robb) updated to tc_utils.source
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

# global variables
#
REQUIRED="which awk cat cmp ls rm"
TST_TOTAL=0

################################################################################
# testcase functions
################################################################################

# Function:		runtests
#
# Description:		- exercise modified gawk "make check" tests
#
# Parameters:		- none
#
# Return		- zero on success
#			- return value from commands on failure
#
function runtests() {

local AWK=$(which awk)
local CMP=$(which cmp)

# non architecture specific tests
local basic_tests="addcomma anchgsub arrayref arynasty \
	arysubnm asgext backgsub clobber convfmt datanonl delarprm \
	dynlj eofsplit fldchg fldchgnf fnarydel fnparydl fsrs funsemnl \
	funstack getlnbuf getnr2tb getnr2tm gsubtest hsprint intest \
	intprec leadnl math nasty nasty2 nfset nlfldsep nlinstr nlstrina \
	numindex numsubstr octsub ofmt ofmtbig ofmts opasnidx opasnslf \
	paramtyp pcntplus prdupval printf1 prmreuse prt1eval prtoeval \
	psx96sub rand rebt8b1 rebt8b2 regeq reindops reparse rsnul1nl \
	rswhite splitargv splitdef splitvar splitwht sprintfc strtod \
	subslash substr swaplns tradanch tweakfld zeroflag"

# testcases that don't easily automate: (skipped for beta)
local special_tests="argarray arrayparm arynocls awkpath back89 childin clsflnam \
	compare defref fnamedat fnarray fnaryscl fnasgnm fsbs \
	fstabplus funsmnam getline gsubasgn leaddig litoct longwrds \
	messages mmap8k negexp nfldstr noeffect nofmtch nonl \
	noparms nors paramdup parseme prmarscl redfilnm resplit rs \
	sclforin sclifin"

# unix specific tests
local unix_tests="getlnhd pipeio1"

# testcases that don't easily automate: (skipped for beta)
local special_unix_tests="fflush pid pipeio2 poundbang strftlng"

# test gawk extended functionality
local gawk_extensions="clos1way fsfwfs gensub gnuops2 gnureops igncdym igncfs \
	nondec procinfs regx8bit sort1"

# testcases that don't easily automate: (skipped for beta)
local special_gawk_extensions="argtest badargs fieldwdth ignrcase lint \
	manyfiles posix reint shadow strftime"

local srcdir="gawk_tests" # testdir we store individual test files in

# count total tests and set harness variable
set $basic_tests $unix_tests $gawk_extensions
TST_TOTAL=$#

# run each testcase
for tst in $basic_tests $unix_tests $gawk_extensions ; do

	# housekeeping
	tc_register "$tst"

	# make sure test files exist
	if [ ! -f $srcdir/$tst.awk ] || [ ! -f $srcdir/$tst.ok ] ; then
		tc_break_if_bad 1 "The test '$tst' does not exist."
		continue;
	fi

	# runit
	if [ -f $srcdir/$tst.in ] ; then # check for input file
		$AWK -f $srcdir/$tst.awk $srcdir/$tst.in > $TCTMP/$tst.out
	else
		$AWK -f $srcdir/$tst.awk > $TCTMP/$tst.out
	fi

	# results
	$CMP $srcdir/$tst.ok $TCTMP/$tst.out >$stderr
	tc_pass_or_fail $? "$tst output unexpected."
	rm -f $TCTMP/$tst.out

done

}

####################################################################################
# MAIN
####################################################################################

# Function:	main
#
# Description:	- Execute all tests, report results
#
# Exit:		- zero on success
#		- non-zero on failure
#
tc_setup
tc_exec_or_break $REQUIRED || exit
runtests
