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
# File :   amtu_test.sh
#
# Description: This program tests basic functionality of amtu command.
#
# Author:   Xie Jue <xiejue@cn.ibm.com>
#
# History:	Jun 7 2004 - created - Xie Jue

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

#
# test01	Installation check
#
function test01()
{
	tc_register "amtu installation check"

	tc_executes amtu
	tc_pass_or_fail $? "AMTU not properly installed"
}

#
# test02	Test amtu command
#
function test02()
{
	tc_register  "Execute Memory Test."
	amtu -m >$stdout 2>&1
	tc_pass_or_fail $? "amtu -m failed"  || return

	tc_register  "Execute Memory Separation Test"
	amtu -s >$stdout 2>&1
	tc_pass_or_fail $? "amtu -s failed" || return

	tc_register  "Execute I/O Controller - Disk Test"
	amtu -i >$stdout 2>&1
	tc_pass_or_fail $? "amtu -i failed" || return

	tc_register  "Execute I/O Controller - Network Test"
	amtu -n >$stdout 2>&1
	tc_pass_or_fail $? "amtu -n failed" || return

	tc_register  "Execute I/O Controller - Disk Test"
	amtu -i >$stdout 2>&1
	tc_pass_or_fail $? "amtu -i failed" || return

	tc_register  "Execute I/O Controller - Network Test"
	amtu -p >$stdout 2>&1
	tc_pass_or_fail $? "amtu -p failed" || return

	tc_register    "amtu test all"
	amtu >$stdout 2>&1
	tc_pass_or_fail $? "amtu run inproperly" || return

}

#
# main
# 

TST_TOTAL=8
tc_setup

tc_exec_or_break grep egrep || exit
tc_root_or_break || exit

test01 || exit
test02

