!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab:
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2004                                               ##
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
# File :	itcl_test.sh
#
# Description:	Test the functionality of itcl.
#
# Author:	CSDL
#
# History:	May 13 2004 - Created. 
#################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# utility functions
################################################################################

TCLCMD=""
TESTDIR="itcltests"

tc_local_setup()
{	
	TCLCMD=`which tclsh`
	if [ "$TCLCMD" = "" ] ; then
		tc_info "no tclsh installed"
		exit 1
	fi
	
	cd $TESTDIR
	$TCLCMD  $PWD/prep.test > $TCTMP/prep.txt 2>&1
	grep "can't find package tcltest" $TCTMP/prep.txt
	if [ $? -eq 0 ] ; then
		tc_info "This test need tcltest package."
		exit 1
	fi
}


################################################################################
#  test functions
################################################################################

haveError=0
haveExecuted=0
ErrorInfo=""
ExecSum=""

handle_test_result() 
{ 
	local aline=""

	haveError=1
	haveExecuted=0
	ErrorInfo=""
	ExecSum="There is some error in the test!"
	
#	cat $TCTMP/testresult.txt
	while read aline
	do
		ErrorInfo=`echo $aline | grep "FAILED" | awk '{ if ($4 != "")  print $0}' | cut -d " " -f 2-`
		if [ "$ErrorInfo" != "" ] ; then
			haveError=2
			tc_info "$ErrorInfo"
		fi

		ErrorInfo=`echo $aline | grep Total | grep Passed | grep Skipped | grep Failed`
		if [ "$ErrorInfo" != "" ] ; then 
			haveExecuted=1
			if [ $haveError -eq 1 ] ; then
				haveError=0
			fi
			ExecSum=`echo $aline | cut -d " " -f 2-`
		fi
	done < $TCTMP/testresult.txt
}

test_template()
{
	tc_register	"$1"
	$TCLCMD $PWD/$1 > $TCTMP/testresult.txt 2>&1	
	handle_test_result 
	tc_pass_or_fail $haveError "$ExecSum"	

}

################################################################################
#  main
################################################################################
TST_TOTAL=15
tc_setup

test_template "basic.test"
test_template "body.test"
test_template "chain.test"
test_template "delete.test"
test_template "ensemble.test"
test_template "import.test"
test_template "info.test"
test_template "inherit.test"
test_template "interp.test"
test_template "local.test"
test_template "methods.test"
test_template "mkindex.test"
test_template "namespace.test"
test_template "protection.test"
test_template "scope.test"
