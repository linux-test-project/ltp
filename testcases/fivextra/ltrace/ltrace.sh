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
# File:		ltracetest.sh
#
# Description:	This program tests basic functionality of ltrace program
#
# Author:	CSDL,  James He <hejianj@cn.ibm.com>
#
# History:	16 April 2004 - created - James He
#		29 April 2004 - rcp - added ltrace -f test.
#		17 May   2004 - James - added run /bin/ngptinit. 
# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# Utility functions 
################################################################################

#
# local setup
#	 
function tc_local_setup()
{	
	tc_exec_or_break ltrace touch || return
	
	# run /bin/ngptinit first for initialing the shared memory.
	tc_exist_or_break /bin/ngptinit || return 
	/bin/ngptinit 2>/dev/null 
	tc_break_if_bad $? "can't initialize the shared memory" || return
	
	# create a temporary directory and populate it with empty files.
	mkdir -p $TCTMP/ltrace.d 2>$stderr 1>$stdout
	tc_break_if_bad $? "can't make directory $TCTMP/ltrace.d" || return
	for file in x y z ; do
		touch $TCTMP/ltrace.d/$file 2>$stderr
		tc_break_if_bad $? "can't create file $TCTMP/ltrace.d/$file" \
			|| return
	done
}

################################################################################
# Testcase functions
################################################################################

#
# test01	installation check
#
function test01()
{
	tc_register	"installation check"
	tc_exists /etc/ltrace.conf
	tc_fail_if_bad $? "ltrace not installed properly"  || return
	tc_executes ltrace
	tc_pass_or_fail $? "ltrace not installed properly"

}

#
# test02	ltrace -h
#
function test02()
{
	tc_register	"ltrace -h command"
	ltrace -h >$stdout 2>$stderr
	tc_pass_or_fail $? "ltrace -h command failed"

}

#
# test03	ltrace --help
#
function test03()
{
	tc_register	"ltrace --help command"
	ltrace --help >$stdout 2>$stderr
	tc_pass_or_fail $? "ltrace --help command failed"

}

#
# test04	ltrace -V
#
function test04()
{
	tc_register	"ltrace -V command"
	ltrace -V >$stdout 2>$stderr
	tc_pass_or_fail $? "ltrace -V command failed"

}

#
# test05	ltrace --version
#
function test05()
{
	tc_register	"ltrace --version command"
	ltrace --version >$stdout 2>$stderr
	tc_pass_or_fail $? "ltrace --version command failed"

}

#
# test06	ltrace -o
#
function test06()
{
	tc_register	"ltrace -o command"
	ltrace -o $TCTMP/ltrace.d/ltrace.out ./ltracetest1 >$stdout 2>$stderr
	tc_fail_if_bad $? "ltrace -o command failed" || return
	tc_exist_or_break $TCTMP/ltrace.d/ltrace.out  || return
	tc_pass_or_fail $? "ltrace -o command failed"
}

#
# test07	ltrace -L -S
#
function test07()
{
	tc_register	"ltrace -L -S command"
	ltrace -L -S ./ltracetest1 &>$stdout
	tc_fail_if_bad $? "ltrace -L -S command failed" || return	
	grep -q "SYS_open" $stdout
	tc_pass_or_fail $? "ltrace -L -S command failed"
}

#
# test08	ltrace -e
#
function test08()
{
	tc_register	"ltrace -e command"
	ltrace -e "\!printf",toupper ./ltracetest2 &>$stdout
	tc_fail_if_bad $? "ltrace -e command failed" || return	
	grep -q "toupper" $stdout
	tc_pass_or_fail $? "ltrace -e command failed"
}

#
# test09	ltrace -f
#
function test09()
{
	tc_register	"ltrace -f command"
	tc_info "10 second delay allowing forked PID to create so ltrace -f can attach ..."
	ltrace -f ./ltrace_fork &>$stdout & wait
	tc_fail_if_bad $? "unexpeced results"
	grep -q "^\[pid [[:digit:]]\+" $stdout
	tc_pass_or_fail $? "expected to see \"[pid xxxxx]\" in stdout"
}

################################################################################
# main
################################################################################

TST_TOTAL=9

tc_setup

test01 || exit
test02
test03
test04
test05
test06
test07
test08                                  
test09
