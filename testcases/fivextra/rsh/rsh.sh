#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003		      ##
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
# File :	rsh.sh	
#
# Description:	test remote shell "rsh" 
#
# Author:	Helen Pang, hpang@us.ibm.com
#
# History:	June 18 2003 - Created. Helen Pang. hpang@us.ibm.com
#		June ?  2003 - Updates after code review.
#
#		16 Dec 2003 - (hpang) updated to tc_utils.source
################################################################################
# source the standard utility functions
###############################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

###############################################################################
#  global variables
###############################################################################
startup="/etc/init.d/xinetd"
xinetd_conf="/etc/xinetd.conf"
securetty="/etc/securetty"

#
# mycleanup     cleanup unique to this testcase
#
function mycleanup()
{
	[ -s $TCTMP/orig_xinetd_conf ] && mv -f $TCTMP/orig_xinetd_conf $xinetd_conf
	[ -s $TCTMP/orig_securetty ] && mv -f $TCTMP/orig_securetty $securetty
	rm -fr $TCTMP/test $TCTMP/rtest 
	cleanup
}	
################################################################################
# the testcase functions
################################################################################

#
# test01	set rsh deamon	        
#		
function test01()
{
        tc_register "set rsh"
	tc_exec_or_break cp echo mv || return

	cp -a $xinetd_conf $TCTMP/orig_xinetd_conf
	tc_fail_if_bad $? "could not copy xinetd_conf file" || return
	
	echo "shell stream tcp nowait root /usr/sbin/tcpd in.rshd -L" >> $xinetd_conf
	echo "shell stream tcp nowait root /usr/sbin/tcpd in.rshd -aL" >> $xinetd_conf
	tc_fail_if_bad $? "could not append to xinetd_conf file" || return
	
	mv -f $securetty $TCTMP/orig_securetty
	tc_fail_if_bad $? "could not move securetty file" || return
	
	tc_pass_or_fail 0 "Will Never Fail Here"
}

#
# test02	start rsh 	
#
function test02()
{
	tc_register "start rsh"
	tc_exec_or_break cat grep || return

	$startup restart >$stdout 2>$stderr
	tc_fail_if_bad $? "could not start inetd" || return

	cat $stdout | grep "Starting inetd" 2>$stderr
	tc_fail_if_bad $? "rsh and rlogin deamons not start" || return

	tc_pass_or_fail 0 " Will Never Fail Here"
}

#
# test03	 test rsh    
#
function test03()
{
	tc_register "test rsh"
	tc_exec_or_break cp cat rsh wc || return
	
	echo "Hi, this is the rsh test" > $TCTMP/test
	rsh 127.0.0.1 cp $TCTMP/test $TCTMP/rtest >$stdout
	tc_fail_if_bad $? "rsh could not copy file to local system" || return

	cc_test=cat $TCTMP/test | wc -c
	cc_rtest=cat $TCTMP/rtest | wc -c

	[ $cc_test -eq $cc_rtest ]
	tc_fail_if_bad $? "unexpected result" || return

	tc_pass_or_fail 0 " Will Never Fail Here"
}

################################################################################
# main
################################################################################

TST_TOTAL=3

# standard tc_setup
tc_setup

tc_root_or_break || exit

trap "mycleanup" 0

test01 && 
test02 &&  
test03 
