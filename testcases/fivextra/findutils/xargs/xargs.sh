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
# File :	xargs.sh
#
# Description:	This is a test kit to test linux command xargs
#
# Author:	Helen Pang, hpang@us.ibm.com
#
# History:	Apr 04 2003 - Created.  Helen Pang, hpang@us.ibm.com
#		May 07 2003 - Updates after code review.
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
# test01	xargs (set/check max args)
#
function test01()
{
	tc_register "set/check max args"

	tc_exec_or_break  touch ls || return
	
	# set max args(num) for xargs
	name=$TCTMP/file
	touch $name
	ls $TCTMP > $name
	num=3
	xargs -n$num < $name >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected result"
	# check max args(num) for xargs
	nlc=$TCTMP/nameline
	slc=$TCTMP/stdoutline
	touch $nlc $slc
	cat $name | wc -l > $nlc
	cat $stdout | wc -l > $slc
	[ $slc==$nlc/$num ]
	tc_pass_or_fail $? " failed to set max numbers of args in xargs"
}

#
# test02       xargs  (set/check interactive)
#
function test02()
{
	tc_register "set/check interactive"
	tc_exec_or_break  touch ls yes || return 
	
	# set/check interactive for xargs
	name=$TCTMP/file
	touch $name
	ls $TCTMP/* > $name
	xargs -p < $name
	tc_pass_or_fail $? "failed to check xargs's interactive in xargs"
}

#
# test03	xargs (set/check max processes) 
#
function test03()
{
	tc_register "set/check max processes"

	tc_exec_or_break  touch ls || return 

	# set max number processes(num) for xargs
	name=$TCTMP/file
	touch $name
	ls $TCTMP > $name	 
	num=2
	xargs -P$num -n$num < $name >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected result"
	# check max number processes(num) for xargs
	nlc=$TCTMP/nameline
	slc=$TCTMP/stdoutline
	touch $nlc $slc
	cat $name | wc -l > $nlc
	cat $stdout | wc -l > $slc
	[ $slc==$nlc/$num ]
	tc_pass_or_fail $? " failed to set max numbers of processes in xargs"
}

#
# test04	xargs (set/check max lines)
#
function test04()
{
	tc_register "set/check max lines"

	tc_exec_or_break  touch ls || return 
	
	# set max number lines for xargs
	name=$TCTMP/file
	touch $name
	ls $TCTMP > $name
	xargs -l < $name >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected result"
	# check max number lines for xargs
	nlc=$TCTMP/nameline
	slc=$TCTMP/stdoutline
	touch $nlc $slc
	cat $name | wc -l > $nlc
	cat $stdout | wc -l > $slc
	[ $slc==$nlc ]
	tc_pass_or_fail $? " failed to set max numbers of lines in xargs"
}

#
# test05       xargs (set/check NULL(-0) option)
#
function test05()
{
	tc_register "set/check NULL"

	tc_exec_or_break  touch ls || return 

	# set NULL for xargs
	name=$TCTMP/file
	touch $name
	ls $TCTMP > $name
	xargs -0 < $name >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected result"
	# check NULL for xargs
	nlc=$TCTMP/nameline
	slc=$TCTMP/stdoutline
	touch $nlc $slc
	cat $name | wc -l > $nlc
	cat $stdout | wc -l > $slc
	[ $slc=="$nlc+1" ]
	tc_pass_or_fail $? " failed to set NULL in xargs"
}

################################################################################
# main
################################################################################

TST_TOTAL=4

# standard tc_setup
tc_setup

test01
# test02	can't run interactive tests!
test03
test04
test05

rm -fr nlc slc
