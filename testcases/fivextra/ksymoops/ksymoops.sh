#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab:
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
# File :	ksymoops.sh
#
# Description:	Test ksymoops utility
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Feb 06 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		Jul 18 2003 - (rcp) separated out from "base" testcase.
#		16 Dec 2003 - (rcp) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

cmd=""		# ksymoops command (must be fully qualified if not root)
		# set in main
mapfile=""

################################################################################
# local utility functions
################################################################################

function tc_local_setup()
{
	tc_exec_or_break uname || exit

	# determind name of system map file. It may or may not have release nbr
	local release=`uname -r || exit`
	local mapfile1=/boot/System.map
	local mapfile2=/boot/System.map-$release
	[ -f $mapfile2 ] && mapfile=$mapfile2 || mapfile=$mapfile1
}

################################################################################
# the testcase functions
################################################################################

#
# test01	is ksymoops installed?
#
function test01()
{
	tc_register "ksymoops installation test"
	tc_exec_or_break $cmd || return
	tc_pass_or_fail $? "ksymoops not installed properly"
}

#
# test02	ksymoops -VKLO -m
#
function test02()
{
	tc_register "ksymoops -VKLO -m $mapfile"
	tc_exec_or_break cat || return

	cat /dev/null | $cmd -VKLO -m $mapfile >$stdout 2>$stderr
	tc_pass_or_fail $? "unexpected response"
}

#
# test03	ksymoops -m
#
function test03()
{
	tc_register "ksymoops -m $mapfile"
	tc_exec_or_break cat grep || return

	cat /dev/null | $cmd -m $mapfile >$stdout 2>$stderr
	tc_fail_if_bad $? "unepected response" || return

	cat $stdout | grep -q System.map >$stdout 2>$stderr
	tc_pass_or_fail $? "Unexpected output" \
		"Expected to see"$'\n'"$mapfile in stdout"
}

################################################################################
# main
################################################################################

TST_TOTAL=3
tc_setup
cmd="ksymoops"
[ "$UID" -eq 0 ] || cmd="/usr/sbin/ksymoops"

test01 &&
test02 &&
test03
