#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2004						      ##
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
# File :        pegasus_test.sh
#
# Description:  Test pegasus tool
#
# Author:      	CSDL  James He <hejianj@cn.ibm.com>
#
# History:      28 Jun 2004 created.
#		1  July 2004 add cimserver cimconfig test cases.
#		2  July 2004 add cimuser cimprovider test cases.
#		20 July 2004 add tc_local_cleanup and chang "ps -ef" to "ps -e".
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

testdir="/usr/bin"
################################################################################
# environment functions
################################################################################

#
# local setup
#
function tc_local_setup()
{
	export PEGASUS_HOME=/var/lib/pegasus
	tc_exec_or_break cimserver cimconfig grep || return

}

function tc_local_cleanup()
{
	killall cimserver
}
################################################################################
# testcase functions
################################################################################

function commonfunc()
{
local tests="TestAbstract TestArray TestBase64 TestCGIQueryString TestClassDecl \
	TestCompare TestConfig TestDateTime TestDynamicLibrary \
	TestFlavor TestFormatter TestHashTable TestInstanceDecl TestL10N TestLogger  \
	TestMethod TestObject TestOperationContext TestParamValue TestParameter \
	TestProperty TestQualifier TestQualifierDecl TestQualifierList TestQueue \
	TestTimeValue TestReference TestResolve TestScope TestStack TestStopwatch \
	TestStrToInstName TestString TestToMof TestValidateClass TestValue TestXmlDump \
	TestXmlParser TestXmlPrint"
set $tests
TST_TOTAL=$#

# run each testcase
for tst in $tests ; do

	tc_register "$tst"

        # make sure test file exists
	[ -f $testdir/$tst ]
	tc_break_if_bad "$?" "The test '$testdir/$tst' does not exist." || continue

	# runit
	echo $tst | grep "Xml"
	if [ $? -eq 0 ] ; then
		$testdir/$tst x.xml >$TCTMP/stdout 2>$TCTMP/stderr
	else
		$testdir/$tst >$TCTMP/stdout 2>$TCTMP/stderr
	fi
	tc_pass_or_fail "$?" "$tst output unexpected."

done	

}

# no in use now, for future test only
function providerfunc()
{
local tests="TestProviderFacade TestProviderManager TestProviderManagerService \
	TestProviderModule TestSafeQueue TestResponseHandler"

set $tests
TST_TOTAL=$#

# run each testcase
for tst in $tests ; do

	tc_register "$tst"

        # make sure test file exists
	[ -f $testdir/$tst ]
	tc_break_if_bad "$?" "The test '$testdir/$tst' does not exist." || continue

	# runit
	$testdir/$tst >$TCTMP/stdout 2>$TCTMP/stderr
	tc_pass_or_fail "$?" "$tst output unexpected."

done

}

function configfunc()
{

local tests="TestConfigFileHandler TestConfigManager"

set $tests
TST_TOTAL=$#

# run each testcase
for tst in $tests ; do

	tc_register "$tst"

        # make sure test file exists
	[ -f $testdir/$tst ]
	tc_break_if_bad "$?" "The test '$testdir/$tst' does not exist." || continue

	# runit
	$testdir/$tst >$TCTMP/stdout 2>$TCTMP/stderr
	tc_pass_or_fail "$?" "$tst output unexpected."

done

}

function securityfunc()
{

local tests="TestAuthenticationManager TestBasicAuthenticationHandler \
	TestLocalAuthFile TestLocalAuthenticationHandler"

set $tests
TST_TOTAL=$#

# run each testcase
for tst in $tests ; do

	tc_register "$tst"

        # make sure test file exists
	[ -f $testdir/$tst ]
	tc_break_if_bad "$?" "The test '$testdir/$tst' does not exist." || continue

	# runit
	$testdir/$tst >$TCTMP/stdout 2>$TCTMP/stderr
	tc_pass_or_fail "$?" "$tst output unexpected."

done

}

function indservfunc()
{

local tests="TestDisableEnable TestDisableEnable2"

set $tests
TST_TOTAL=$#

# run each testcase
for tst in $tests ; do

	tc_register "$tst"

        # make sure test file exists
	[ -f $testdir/$tst ]
	tc_break_if_bad "$?" "The test '$testdir/$tst' does not exist." || continue

	# runit
	$testdir/$tst >$TCTMP/stdout 2>$TCTMP/stderr
	tc_pass_or_fail "$?" "$tst output unexpected."

done

}

function cimservtest()
{
	tc_register "cimserver test"

	[ -f $testdir/cimserver ]
	tc_break_if_bad "$?" "The test '$testdir/cimserv' does not exist." || return
	ps -e | grep -q cimserver 

	if [ $? -eq 0 ]; then
		cimserver -s
		if [ $? -ne 0 ] ; then
			tc_info "cimserver stop incorrectly" && return
		fi
	fi
	cimserver -D $PEGASUS_HOME 1>$TCTMP/stdout 2>$TCTMP/stderr
        tc_pass_or_fail $? "cimserver output unexpected"
	
}

# these test cases are part of pegasus cimconfig test
function cimconftest()
{
	tc_register "cimconfig test"

	touch $TCTMP/noerror

	[ -f $testdir/cimconfig ]
	tc_break_if_bad "$?" "The test '$testdir/$tst' does not exist." || return
		
	#start cimserver first
	ps -e | grep -q cimserver 
	if [ $? -ne 0 ]; then
		cimserver -D $PEGASUS_HOME 1>$TCTMP/stdout 2>$TCTMP/stderr
		if [ $? -ne 0 ] ; then
			tc_info "cimserver start incorrectly" && return
		fi
	fi

#Set the current values:
	cimconfig -s traceLevel=1 -c  1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -s traceLevel=2 -c  1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -s traceLevel=3 -c  1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -s traceLevel=4 -c  1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -s traceComponents=ALL -c   1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -s traceComponents=Config -c   1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -s traceComponents=Config,XmlParser -c   1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -s traceFilePath=/tmp/test.trace -c   1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -s traceFilePath=/tmp/test1.trace -c   1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -s traceFilePath=/tmp/test.trace -c   1>>$TCTMP/output 2>>$TCTMP/errors
#Testing all get options:	
	cimconfig -g traceLevel 1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -g traceComponents 1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -g traceFilePath 1>>$TCTMP/output 2>>$TCTMP/errors
#Get the current values:
	cimconfig -g  traceLevel -c 1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -g  traceComponents -c 1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -g  traceFilePath -c 1>>$TCTMP/output 2>>$TCTMP/errors
#Get the planned values:
	cimconfig -g traceLevel -p 1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -g traceComponents -p 1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -g traceFilePath -p 1>>$TCTMP/output 2>>$TCTMP/errors
#Display properties:
	cimconfig -l -c 1>>$TCTMP/output 2>>$TCTMP/errors
#Set the planned values
	cimconfig -s traceLevel=1 -p 1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -s traceLevel=2 -p 1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -s traceLevel=3 -p 1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -s traceLevel=4 -p 1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -s traceComponents=ALL -p 1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -s traceComponents=Config -p 1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -s traceComponents=Config,XmlParser -p 1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -s traceComponents= -p 1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -s traceFilePath=/tmp/test.trace -p 1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -s traceFilePath=/tmp/test1.trace -p 1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -s traceFilePath=/tmp/test.trace -p 1>>$TCTMP/output 2>>$TCTMP/errors
#Display properties:
	cimconfig -l -p 1>>$TCTMP/output 2>>$TCTMP/errors
#Unset the current values: 
	cimconfig -u traceLevel -c 1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -u traceComponents -c 1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -u traceFilePath -c 1>>$TCTMP/output 2>>$TCTMP/errors
#Display properties:
	cimconfig -l -c 1>>$TCTMP/output 2>>$TCTMP/errors
#Unset the planned values:
	cimconfig -u traceLevel -p 1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -u traceComponents -p 1>>$TCTMP/output 2>>$TCTMP/errors
	cimconfig -u traceFilePath -p 1>>$TCTMP/output 2>>$TCTMP/errors
#Display properties:
	cimconfig -l -p	1>>$TCTMP/output 2>>$TCTMP/errors

	diff $TCTMP/noerror $TCTMP/errors
	tc_pass_or_fail "$?" "cimconfig ouput unexpected" && cat $TCTMP/errors

}

function cimusertest()
{
	tc_register "cimuser add test"

	#start cimserver first
	ps -e | grep -q cimserver 
	if [ $? -ne 0 ]; then
		cimserver -D $PEGASUS_HOME 1>$TCTMP/stdout 2>$TCTMP/stderr
		if [ $? -ne 0 ] ; then
			tc_info "cimserver start incorrectly" && return
		fi
	fi

        tc_add_user_or_break || exit
        USER1=$temp_user
        cimuser -a -u $USER1 -w $USER1 1>/dev/null 2>&1
        cat $PEGASUS_HOME/cimserver.passwd | grep -q "$USER1"
        tc_pass_or_fail $? "cimuser output unexpected"

	tc_register "cimuser del test"
        cimuser -r -u $USER1 1>/dev/null 2>&1
        cat $PEGASUS_HOME/cimserver.passwd | grep -q "$USER1"
        tc_pass_or_fail !$? "cimuser output unexpected"
     
}

function cimprovtest()
{
	tc_register "cimprovider test"
	ps -e | grep -q cimserver 
	if [ $? -ne 0 ]; then
		cimserver -D $PEGASUS_HOME 1>$TCTMP/stdout 2>$TCTMP/stderr
		if [ $? -ne 0 ] ; then
			tc_info "cimserver start incorrectly" && return
		fi
	fi
	cimprovider -l -s 1>/dev/null 2>&1
        tc_pass_or_fail $? "cimprovider output unexpected"		
}

################################################################################
# MAIN
################################################################################

tc_setup
commonfunc
configfunc
#providerfunc
#securityfunc
#indservfunc
cimservtest
cimconftest
cimusertest
cimprovtest
