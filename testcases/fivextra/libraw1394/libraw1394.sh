#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2004						      ##
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
# File :   libraw1394.sh
#
# Description: This program tests basic functionality of dmidecode command.
#
# Author:   Xie Jue <xiejue@cn.ibm.com>
#
# History:	May 20 2004 - created - Xie Jue

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

#
# test01	Installation check
#
function test01()
{
	tc_register "libraw1394 installation check"
	tc_executes testlibraw
	tc_pass_or_fail $? "libraw1394 not properly installed"
}

#
# test02	Test libraw1394 using testlibraw commands
#
function test02()
{
	tc_register    "testlibraw displays 1394 information"

	testlibraw >$stdout 2>$stderr
	tc_pass_or_fail $? "testlibraw failed to print 1394 devices" 

}
#
# main
# 

TST_TOTAL=2
tc_setup

tc_root_or_break || exit

test01 &&
test02

