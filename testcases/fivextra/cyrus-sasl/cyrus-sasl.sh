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
# File :	cyrus_sasl.sh
#
# Description:	Check that cyrus-sasl can manage users, domains, passwords.
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Feb 27 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		Mar 12 2003 - Minor updates.
#		15 Dec 2003 (rcp) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# the testcase functions
################################################################################

#
# test01	check that cyrus-sasl is installed 
#
#		We consider it a failure if cyrus-sasl isn't installed.
#		Test case shouldn't be run if cyrus-sasl not installed.
#
function test01()
{
	tc_register "is cyrus-sasl installed?"
	which saslpasswd >/dev/null
	tc_pass_or_fail $? "cyrus-sasl is not installed"
}

#
# test02	create a cyrus-sasl user
#
function test02()
{
	tc_register "create cyrus-sasl user"
#
	# known problem with saslpasswd: first time use always fails
	echo mypass | saslpasswd -c -p dummy$$ &>/dev/null
	saslpasswd -d dummy$$ &>/dev/null
#
	# now the actual test
	local user="csuser$$"
	local command="saslpasswd -a my_app -p -c -u my_domain $user"
	echo mypass | $command 2>$stderr >$stdout
	tc_pass_or_fail $? "bad result from $command"
}

#
# test03	list the cyrus-sasl user
#
function test03()
{
	tc_register "list the cyrus-sasl user"
	local user="csuser$$"
	sasldblistusers 2>$stderr >$stdout
	tc_fail_if_bad $? "Bad response from sasldblistusers" || return
#
	cat $stdout | grep $user >/dev/null
	tc_pass_or_fail $? "sasldblistusres didn't list sasl user $user"
}

#
# test04	deleting user without specifying domain should not delete user
#
function test04()
{
	tc_register "user should not delete w/o domain"
	local user="csuser$$"
	local command="saslpasswd -d $user"
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
function test05()
{
	tc_register "user should delete OK w/domain"
	local user="csuser$$"
	local command="saslpasswd -u my_domain -d $user"
	$command 2>$stderr >$stdout
	tc_pass_or_fail $? "bad response from \"$command\""
}

#
# test06	deleted user should not show in list of users
#
function test06()
{
	tc_register "deleted user should be gone"
	local user="csuser$$"
	sasldblistusers 2>$stderr >$stdout
	local result="`cat $stdout | grep $user`"
	[ -z "$result" ]
	tc_pass_or_fail $? "Didn't expect to see \"$user\" in stdout"
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
