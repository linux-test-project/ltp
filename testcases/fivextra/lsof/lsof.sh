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
# File :	lsof.sh
#
# Description:	Test the lsof program
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	June 18 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		16 Dec 2003 - (rcp) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

looper=""
myscript=""
myfile=""
TESTLIST="LTbasic LTnlink LTsock LTunix LTdnlc LTlock"

################################################################################
# the utility functions
################################################################################

#
#	tc_local_cleanup
#
function tc_local_cleanup()
{
	[ "$looper" ] && killall $looper &>/dev/null
}

################################################################################
# the testcase functions
################################################################################

#
#	test01	Check to see that lsof is installed.
#
function test01()
{
	tc_register "is lsof installed?"

	tc_executes lsof
	tc_pass_or_fail $? "lsof not installed"
}

#
#	test02	Open a file and see that lsof lists it.
#
function test02()
{
	tc_register	"lsof should list an open file"

	# create script that will loop forever
	# then redirect its output (of which there is none) to a file
	cat > $myscript <<-EOF
		#!$SHELL
		while : ; do
			sleep 1
		done
	EOF
	chmod +x $myscript
	eval $myscript > $myfile &

	# wait for process to start
	while ! ps -ef | grep -q "[l]ooper$$.sh" ; do
		tc_info "waiting for $looper to start"
		sleep 1
	done

	# see that lsof lists the file
	lsof >$stdout 2>$stderr
	grep -q "$myfile" $stdout
	tc_pass_or_fail $? "expected to see file $myfile listed in output"
}

#
#	test03	As test02, but restrict output of lsof with -c.
#		-c compares command names.
#		If the expression matches the file should be listed. 
#
function test03()
{
	tc_register	"lsof -c $looper finds match"

	lsof -c $looper >$stdout 2>$stderr
	grep -q "$myfile" $stdout
	tc_pass_or_fail $? "expected to see file $myfile listed in output"
}

#
#	test04	As test02, but restrict output of lsof with -c.
#		-c compares command names.
#		If the expression does NOT match the file should
#		NOT be listed. 
#
function test04()
{
	tc_register	"lsof -c x$looper does NOT find match"

	lsof -c x$looper >$stdout 2>$stderr
	grep -q "$myfile" $stdout
	[ $? -ne 0 ]
	tc_pass_or_fail $? "expected to NOT see file $myfile listed in output"
}

#
#	test0n	Close the previously opened file and see that lsof no longer
#		lists it. (Depends on file opened by test02.)
function test0n()
{
	tc_register	"lsof should NOT list closed file"

	# kill the process
	tc_info "Terminated message is expected..."
	killall $looper

	# wait for the process to die
	while ps -ef | grep "[l]oop$$.sh" ; do	# wait for the process to die
		tc_info "waiting for $looper to die"
		sleep 5
	done
	sleep 5

	# see that lsof no longer lists the file
	lsof >$stdout 2>$stderr
	grep -q "$myfile" $stdout
	[ $? -ne 0 ]
	tc_pass_or_fail $? "expected to NOT see file $myfile listed in output"
}

#
#	testxx		Run all tests ported from the source tree
#
function testxx()
{
	for t in $TESTLIST ; do
		tc_register "$t"
		LT/$t >$stdout 2>$stderr
		tc_pass_or_fail $? "bad response from test"
	done;
}

################################################################################
# main
################################################################################

set $TESTLIST
let TST_TOTAL=5+$#

tc_setup			# standard setup

tc_exec_or_break grep killall sleep chmod || exit

set the global variables
looper=looper$$.sh
myscript=$TCTMP/$looper
myfile=$TCTMP/myfile

test01 && \
test02 && \
test03 && \
test04 && \
test0n && \
testxx
