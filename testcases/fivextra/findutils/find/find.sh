#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
###############################################################################
##									      #
## (C) Copyright IBM Corp. 2003		      #
##									      #
## This program is free software;  you can redistribute it and#or modify      #
## it under the terms of the GNU General Public License as published by       #
## the Free Software Foundation; either version 2 of the License, or	      #
## (at your option) any later version.					      #
##									      #
## This program is distributed in the hope that it will be useful, but	      #
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY #
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   #
## for more details.							      #
##									      #
## You should have received a copy of the GNU General Public License	      #
## along with this program;  if not, write to the Free Software		      #
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    #
##									      #
###############################################################################
#
#
# File :	find.sh
#
# Description:	This is a test kit to test linux command find
#
# Author:	Helen Pang, hpang@us.ibm.com
#
# History:	Mar 26 2003 - Created.  Helen Pang, hpang@us.ibm.com
#		May 05 2003 - Updates after code review.
#		16 Dec 2003 - (hpang) updated to tc_utils.source

###############################################################################
#
# source the standard utility functions
###############################################################################
#

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

###############################################################################
#
# utility functions specific to this testcase
###############################################################################
#

################################################################################
# the testcase functions
################################################################################

#
# test01    find (check name)
#
function test01()
{
	tc_register "\"find $TCTMP -name name\" check name"

	tc_exec_or_break echo touch || return

	name=$TCTMP/test
	touch $name
	find $TCTMP -name $name >$stdout 2>$stderr
	tc_pass_or_fail $? "Unexpected Name check fail"
}

#
# test02	find (check name with wild card)
#
function test02()
{
	tc_register "\"find $TCTMP -name name\" check name with wild card"

	tc_exec_or_break echo touch grep || return 

	name="t*"
	s1=$TCTMP/tea
	s2=$TCTMP/team
	s3=$TCTMP/two
	touch $s1
	touch $s2
	touch $s3
	find $TCTMP -name "$name" >$stdout 2>$stderr
	grep $s1 $stdout >/dev/null
	tc_fail_if_bad $? "Unexpected no $s1 check out" || return
	grep $s2 $stdout >/dev/null
	tc_fail_if_bad $? "Unexpected no $s2 check out" || return
	grep $s3 $stdout >/dev/null
	tc_fail_if_bad $? "Unexpected no $s3 check out" || return
	
	tc_pass_or_fail 0 "Will never fail here to check name w/ wide card"
}

#
# test03	find (check newer that yield true))

function test03()
{
	tc_register "\"find $TCTMP -newer $compare\" check newer"

	tc_exec_or_break  touch sleep || return 

	name=$TCTMP/test
	compare=$TCTMP/test1
	touch $compare
	sleep 1
	touch $name
	find $TCTMP -newer $compare >$stdout 2>$stderr
	grep $name $stdout > /dev/null
	tc_pass_or_fail $? "Unexpected check newer fail."
}


#
# test04    find (check -perm with chmod syntax)
#
function test04()
{
	tc_register "\"find $TCTMP -perm -$mode \" check -perm with chmod syntex"

	tc_exec_or_break  touch chmod || return 

	name=$TCTMP/test
	mode=u+wrx
	touch $name
	chmod $mode $name
	find $TCTMP -perm -$mode >$stdout 2>$stderr
	grep $name $stdout >/dev/null
	tc_pass_or_fail $? "Unexpected check perm with chmod syntex fail"
}

#
# test05    find (check -perm with octal syntax)
#
function test05()
{
	tc_register "\"find $TCTMP -perm -$mode \" check -perm with chmod syntex"

	tc_exec_or_break touch chmod || return 

	name=$TCTMP/test
	mode=644
	touch $name
	chmod $mode $name
	find $TCTMP -perm -$mode >$stdout 2>$stderr
	grep $name $stdout >/dev/null
	tc_pass_or_fail $? "Unexpected check perm w/ octal syntex fail."
}

#
# test06    find (check regular expression)
#
function test06()
{
	tc_register "\"find $TCTMP -regex $rexp\" check regular expression"

	tc_exec_or_break  touch || return 

	name=$TCTMP/fubar3
	rexp=".*b.*3"
	touch $name
	find $TCTMP -regex "$rexp" >$stdout 2>$stderr
	grep $name $stdout >/dev/null
	tc_fail_if_bad $? "Expect check regular expression pass" || return
	
	rexp="*bar."
	find $TCTMP -regex "$rexp" >$stdout 2>$stderr
	cc=`cat $stdout | wc -c`
	[ $cc == 0 ]
	tc_pass_or_fail $? "check regular expression no pass w/ wrong regex" 
}

#
# test07     find  (check anewer expression)
#
function test07()
{
	tc_register "\"find $TCTMP -anewer $compare\" check anewer"
	tc_exec_or_break touch sleep cat || return 

	name=$TCTMP/test
	compare=$TCTMP/test1
	touch $compare
	sleep 1
	touch $name
	sleep 1
	cat $compare
	cat $name
	
	find $TCTMP -anewer $compare >$stdout 2>$stderr
	grep $name $stdout >/dev/null
	tc_pass_or_fail $? "Expect check anewer pass" 
}

################################################################################
# main
################################################################################

TST_TOTAL=7
# standard tc_setup
tc_setup

test01
test02
test03
test04
test05
test06
test07

