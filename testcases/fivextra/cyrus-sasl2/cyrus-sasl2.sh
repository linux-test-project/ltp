#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2004		      ##
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
# File :	cyrus_sasl2.sh
#
# Description:	Check that cyrus-sasl2 can manage users, domains, passwords.
#
# Author:	Jue Xie, xiejue@cn.ibm.com	
#		Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Feb 27 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		Mar 12 2003 - Minor updates.
#		15 Dec 2003 (rcp) updated to tc_utils.source
#		20 Apr 2004 (xiejue) add saslauthd test and move it to sasl2 

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# the testcase functions
################################################################################

#
# test01	check that cyrus-sasl2 is installed 
#
#		We consider it a failure if cyrus-sasl2 isn't installed.
#		Test case shouldn't be run if cyrus-sasl2 not installed.
#
function test01()
{
	tc_register "is cyrus-sasl2 installed?"
	which saslpasswd2 >/dev/null
	tc_pass_or_fail $? "cyrus-sasl2 is not installed"
}

#
# test02	create a cyrus-sasl2 user
#
function test02()
{
	tc_register "create cyrus-sasl2 user"
#
	# known problem with saslpasswd: first time use always fails
	echo mypass | saslpasswd2 -c -p dummy$$ &>/dev/null
	saslpasswd2 -d dummy$$ &>/dev/null
#
	# now the actual test
	local user="csuser$$"
	local command="saslpasswd2 -a my_app -p -c -u my_domain $user"
	echo mypass | $command 2>$stderr >$stdout
	tc_pass_or_fail $? "bad result from $command"
}

#
# test03	list the cyrus-sasl2 user
#
function test03()
{
	tc_register "list the cyrus-sasl2 user"
	local user="csuser$$"
	sasldblistusers2 2>$stderr >$stdout
	tc_fail_if_bad $? "Bad response from sasldblistusers2" || return
#
	cat $stdout | grep $user >/dev/null
	tc_pass_or_fail $? "sasldblistusres2 didn't list sasl user $user"
}

#
# test04x	deleting user without specifying domain should not delete user
#
function test04x()
{
	tc_register "user should not delete w/o domain"
	local user="csuser$$"
	local command="saslpasswd2 -d $user "
	$command >$stdout 2>&1			# stderr expected
	[ $? -ne 0 ]				# bad response expected
	tc_fail_if_bad $? "unexpected response from \"$command\"" || return
#
	cat $stdout | grep "user not found" >/dev/null
	tc_pass_or_fail $? "expected to see \"user not found\" in stdout"
}

#
# test05	deleting user and specifying domain should delete user
#
function test04()
{
	tc_register "user should delete OK w/domain"
	local user="csuser$$"
	local command="saslpasswd2 -u my_domain -d $user"
	$command 2>$stderr >$stdout
	tc_pass_or_fail $? "bad response from \"$command\""
}

#
# test05	deleted user should not show in list of users
#
function test05()
{
	tc_register "deleted user should be gone"
	local user="csuser$$"
	sasldblistusers2 2>$stderr >$stdout
	local result="`cat $stdout | grep $user`"
	[ -z "$result" ]
	tc_pass_or_fail $? "Didn't expect to see \"$user\" in stdout"
}

#
# test06	saslauthd  is sasl authentication server	
#
function test06()
{
	tc_register "test saslauthd sasl authentication server"
	local password="password"
	# Check if supporting utilities are available
	tc_exec_or_break testsaslauthd saslauthd || return

	tc_add_user_or_break || return  # add new user with password set to "password"

	ps -e|grep saslauthd 2>&1 >/dev/null && killall saslauthd 
######## On RHEL, use "saslauthd -a shadow -m $TCTMP" ###############
	saslauthd -a shadow -m $TCTMP/mux
	sleep 3
#####################################################################

# give wrong passwd to saslauthd and expect fail. 
	testsaslauthd -u $temp_user -p "x$password" -f $TCTMP/mux >$stdout 2>$stderr
	[ $? -ne 0 ]
	tc_fail_if_bad $? "expected saslauthd fail but pass " || return
# Give the right one, and expect pass
	testsaslauthd -u $temp_user -p $password -f $TCTMP/mux >$stdout 2>$stderr
	tc_pass_or_fail $? "expected to saslauthd pass but fail"
	killall saslauthd
	tc_del_user_or_break || return
}


################################################################################
# main
################################################################################

TST_TOTAL=6

tc_setup			# standard tc_setup

tc_root_or_break || exit

tc_exec_or_break echo grep || exit

test01 && \
test02 && \
test03 && \
test04 && \
test05 && \
test06
