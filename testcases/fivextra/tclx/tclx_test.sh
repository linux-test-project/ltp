#!/bin/sh
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
# File :	tclx_test.sh
#
# Description:	Test the functionality of tclx.
#
# Author:	CSDL
#
# History:	May 20 2004 - Created. 
#################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# utility functions
################################################################################

TCLCMD=""
TESTDIR="tclxtests"

tc_local_setup()
{	
	TCLCMD=`which tclsh`
	if [ "$TCLCMD" = "" ] ; then
		tc_info "no tclsh installed"
		exit 1
	fi
	cd $TESTDIR
	
	TCLCMD  $PWD/prep.test > $TCTMP/prep.txt 2>&1
	grep "can't find package Tclx" $TCTMP/prep.txt
	if [ $? -eq 0 ] ; then
	        tc_info "No Tclx instlled."
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
	
	#cat $TCTMP/testresult.txt
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
TST_TOTAL=48
tc_setup

test_template arrayproc.test
test_template bsearch.test
test_template chartype.test
test_template chmod.test
test_template chroot.test
test_template cmdloop.test
test_template cmdtrace.test
test_template convlib.test
test_template dup.test
test_template fcntl.test
test_template filescan.test
test_template flock.test
test_template fmath.test
test_template forfile.test
test_template fstat.test
test_template ftrunc.test
test_template globrecur.test
#test_template help.test
test_template id.test
test_template init.test
test_template keylist.test
test_template lassign.test
test_template lgets.test
test_template list.test
test_template lmatch.test
test_template loop.test
test_template math.test
test_template msgcat.test
test_template nice.test
test_template pipe.test
test_template process.test
test_template profile.test
test_template pushd.test
test_template readdir.test
test_template select.test
test_template setfuncs.test
test_template showproc.test
test_template signal.test
test_template socket.test
test_template string.test
test_template stringfil.test
test_template sync.test
test_template tcllib.test
test_template tryeval.test
test_template unixcmds.test

test_template "compat/clock.test"
test_template "compat/copyfile.test"
test_template "compat/file.test"
test_template "compat/server.test"

