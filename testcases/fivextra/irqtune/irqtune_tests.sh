#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003                                               ##
##									      ##
## This program is free software;  you can redistribute it and/or modify      ##
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
# File :	   irqtune_tests.sh
#
# Description: This program tests basic functionality of irqtune commands.
#
# Author:	   Manoj Iyer  manjo@mail.utexas.edu
#
# History:	   July 15 2003 - created - Manoj Iyer
#		   Sept 23 2003 - Fixed   - Manoj Iyer
#		   - missing " sign was added
#		   - irqtune command need to be called with fully qualified name.
#		   - I think this is a bogus requirement, but I am kludging the 
#		   - the testcase anyway.
#		Oct 20 2003 (rcp)
#		- correct hard-coded paths in expected output
#		- use new tc_local_setup and tc_local_cleanup functions
#		- fix tabbing
#		08 Jan 2004 - (RR) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

irqtune=/sbin/irqtune

#
# tc_local_cleanup
#
tc_local_cleanup()
{
	$irqtune -s &>/dev/null
}

#
# tc_local_setup
#
function tc_local_setup()
{
	tc_root_or_break || return
	tc_exec_or_break diff sed awk cat || return
}

#
# test01	installation check
#
function test01()
{
	tc_register	"installation check"
	tc_executes $irqtune
	tc_pass_or_fail $? "not properly installed"
}

#
# test02	irqtune -x show inactive devices in table
#
test02()
{
	tc_register    "irqtune -x show inactive devices in table"

	$irqtune -x &>$stdout
	tc_fail_if_bad $? "unexpected response from irqtune -x"

	local exp1="irqtune: setting system IRQ priority to 3/14"
	local exp2="irqtune: loading.*irqtune_mod.o"
	local exp3="irqtune: unloading.*irqtune_mod.o"
	local exp4="Inactive"
	local exp5="irqtune: complete"

	grep -q "$exp1" $stdout
	tc_fail_if_bad $? "expected \"$exp1\" in stdout" || return
	grep -q "$exp2" $stdout
	tc_fail_if_bad $? "expected \"$exp2\" in stdout" || return
	grep -q "$exp3" $stdout
	tc_fail_if_bad $? "expected \"$exp3\" in stdout" || return
	grep -q "$exp4" $stdout
	tc_fail_if_bad $? "expected \"$exp4\" in stdout" || return
	grep -q "$exp5" $stdout
	tc_pass_or_fail $? "expected \"$exp5\" in stdout"
}

#
# test03	irqtune -s change priorities of IRQs
#
test03()
{
	tc_register    "irqtune -s change priorities of IRQs"

	$irqtune -s &>$stdout
	tc_fail_if_bad $? "unexpected results from $irqtune -s" || return

	# get the irq of lowerst priority.
	irq_prio=$(sed -ne '/^I/p' $stdout | tail -n 1 | cut -f1 -d/ | cut -f2 -dI)

	# bump up the priority of this irq to highest priority
	$irqtune -s $irq_prio &>$stdout
	tc_fail_if_bad $? "failed to set $irq_prio to higest priority" || return

	grep -q "I$irq_prio/P00:" $stdout 2>$stderr
	tc_pass_or_fail $? "irqtune unable to change priority"
}


#
# test04	irqtune -q suppress priority table printing
#
test04()
{
	tc_register    "irqtune -q suppress priority table printing"

	$irqtune -q &>$stdout
	tc_fail_if_bad $? "failed to print IRQ priority table." || return

	local exp1="irqtune: setting system IRQ priority to 3/14"
	local exp2="irqtune: loading.*irqtune_mod.o"
	local exp3="irqtune: unloading.*irqtune_mod.o"
	local exp5="irqtune: complete"

	grep -q "$exp1" $stdout
	tc_fail_if_bad $? "expected \"$exp1\" in stdout" || return
	grep -q "$exp2" $stdout
	tc_fail_if_bad $? "expected \"$exp2\" in stdout" || return
	grep -q "$exp3" $stdout
	tc_fail_if_bad $? "expected \"$exp3\" in stdout" || return
	grep -q "$exp5" $stdout
	tc_pass_or_fail $? "expected \"$exp5\" in stdout"
}

#
# main
# 
TST_TOTAL=4

tc_setup

test01 || return
test02
test03
test04 
