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
# File :   dmidecode_test.sh
#
# Description: This program tests basic functionality of dmidecode command.
#
# Author:   Xie Jue <xiejue@cn.ibm.com>
#
# History:	May 8 2004 - created - Xie Jue
#		Aug 02 2004 (rcp) eliminated checks for a few things that don't
#				show up on the 330 systems in the Austin Lab.
#				See "RCP1" comments...

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

#
# test01	Installation check
#
function test01()
{
	tc_register "dmidecode installation check"

	tc_executes dmidecode biosdecode vpddecode ownership
	tc_pass_or_fail $? "dmidecode not properly installed"
}

#
# test02	Test dmidecode commands
#
function test02()
{
	tc_register    "dmidecode displays DMI information"

	dmidecode >$stdout 2>$stderr
	tc_fail_if_bad $? "dmidecode failed to print default settings" || return

	grep -q "BIOS Information" $stdout &&
	grep -q "Processor Information" $stdout &&
	grep -q "Chassis Information" $stdout &&
	grep -q "Cache Information" $stdout &&
	grep -q "Port Connector Information" $stdout &&
	grep -q "System Slot Information" $stdout &&
	grep -q "On Board Device Information" $stdout &&
	grep -q "Physical Memory Array" $stdout &&
	grep -q "Memory Device" $stdout
# RCP1	grep -q "System Reset" $stdout
	tc_pass_or_fail $? "expected key words not found"
}

#
# test03	Test biosdecode commands
#
function test03()
{
	tc_register    "biosdecode displays BIOS information"

	biosdecode >$stdout 2>$stderr
	tc_fail_if_bad $? "failed to print default settings" || return

# RCP1	grep -q "SMBIOS"  $stdout &&
	grep -q "BIOS32 Service Directory present" $stdout &&
	grep -q "PNP BIOS" $stdout &&
# RCP1	grep -q "PCI Interrupt Routing" $stdout &&
	grep -q "VPD present" $stdout 
	tc_pass_or_fail $? "expected key words not found"
}


#
# test04	Test vpddecode commands
#
function test04()
{
	tc_register    "vpddecode displays VPD information"

	vpddecode >$stdout 2>$stderr
	tc_fail_if_bad $? "failed to print default settings" || return

	grep -q "Bios Build ID"  $stdout &&
	grep -q "Product Name" $stdout &&
	grep -q "Box Serial Number" $stdout &&
	grep -q "Motherboard Serial Number" $stdout &&
	grep -q "Machine Type\/Model" $stdout 
	tc_pass_or_fail $? "expected key words not found"
}

# 
# main
# 

TST_TOTAL=4
tc_setup

tc_exec_or_break grep egrep || exit
tc_root_or_break || exit
        
uname -m | egrep -q "ia64|i[3456]86"
tc_break_if_bad $? "Must run on ix86 or ia64 architecture" || exit

test01
test02
test03
test04

