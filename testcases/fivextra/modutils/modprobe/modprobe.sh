#!/bin/bash
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2003						      ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File :        modprobe.sh
# Author:	Yu-Pao Lee (yplee@us.ibm.com)
#
# Description:  Test basic functionality of modprobe command
#		modprobe - high level handling of loadable modules
#
# History:      Feb 19 2003 - Created. Yu-Pao Lee (yplee@us.ibm.com) 
#		Jun 18 2003 - BUG 3190 (RC Paulsen)
#		17 Dec 2003 - (rcp) updated to tc_utils.source

################################################################################
# source the standard utility functions 
################################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# globals
################################################################################

mod_dir=/lib/modules/`uname -r`/kernel/drivers/net 
modname=dummy$$
must_remove_mod="no"

################################################################################
# any utility functions specific to this file can go here
################################################################################

#
# tc_local_cleanup	remove installed module
#
function tc_local_cleanup()
{
	[ $must_remove_mod = "yes" ] && {
		rmmod $modname
		rm -f $mod_dir/$modname
		depmod -a &>/dev/null
	}
}

################################################################################
# the testcase functions
################################################################################

#
# test01	check that modprobe is installed
#
function test01()
{
	tc_register	"check that modprobe is installed"
	tc_executes modprobe
	tc_pass_or_fail $? "not installed"
}

#
# test02	modprobe - high level handling of loadable modules 
#
function test02()
{
	tc_register "modprobe"	

	tc_exec_or_break rm cp grep depmod lsmod rmmod || return
	tc_exist_or_break $mod_dir/dummy.o || return 

	must_remove_mod="yes"

	cp $mod_dir/dummy.o $mod_dir/$modname
	depmod >$stdout 2>$stderr
	tc_fail_if_bad $? "depmod failed" || return

	modprobe $modname >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected modprobe results" || return

	lsmod >$stdout 2>$stderr
	tc_fail_if_bad $? "lsmod failed: cannot list modules" || return

	cat $stdout | grep -q $modname
	tc_fail_if_bad $? "$modname apparently not installed. lsmod output follows" || return

	rmmod $modname >$stdout 2>$stderr
	tc_pass_or_fail $? "rmmod failed: cannot remove $modname" || return

	must_remove_mod="no"
}  

################################################################################
# main
################################################################################

TST_TOTAL=2

tc_setup			# standard setup

tc_root_or_break || exit

test01 &&
test02
