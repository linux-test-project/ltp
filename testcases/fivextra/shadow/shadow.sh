#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
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
# File :	template_sh
#
# Description:	Test that shadow passwords are used properly
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Mar 02 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		Dec 03 2003 (rcp) BUG 5203
#		09 Dec 2003 (rcp) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

# global variables
shadow_line1=""
shadow_line2=""

################################################################################
# the testcase functions
################################################################################

function test01()	# verify temp user has entry in /etc/shadow
{
	tc_register	"new users go into /etc/shadow"
	tc_exec_or_break cat || return

	shadow_line1="`cat /etc/shadow | grep \"$tempuser2\"`"
	[ "$shadow_line1" ]
	tc_pass_or_fail $? "$tempuser2 was not put into /etc/shadow"
}

function test02()	# set a pasword - should modify user's shadow pw
{
	tc_register	"set password changes /etc/shadow"
	tc_exec_or_break expect cat chmod grep || return

	local expcmd=`which expect`
	cat > $TCTMP/exp$TCID <<-EOF
		#!$expcmd -f
		set timeout 5
		proc abort {} { exit 1 }
		spawn passwd $tempuser2
		expect {
			timeout abort
			assword:
		}
		sleep 2
		send "secret2\r"
		expect {
			timeout abort
			assword:
		}
		send "secret2\r"
		expect {
			timeout abort
			changed
		}
	EOF
	chmod +x $TCTMP/exp$TCID
	$TCTMP/exp$TCID >$stdout 2>$stderr
	tc_fail_if_bad $? "could not set password" || return

	shadow_line2="`cat /etc/shadow | grep \"$tempuser2\"`"
	[ "$shadow_line1" != "$shadow_line2" ]
	tc_pass_or_fail $? \
		"$tempuser2 password not changed in /etc/shadow: $shadow_line2"
}

function test03()	# test password login
{
	tc_register	"login using changed password"
	tc_exec_or_break cat chown chmod expect || return

	# tempuser1's login scripts
	cat >> /home/$tempuser1/.bashrc <<-EOF
		echo "OK1"
	EOF
	chown $tempuser1 /home/$tempuser1/.bashrc
	cp -a /home/$tempuser1/.bashrc /home/$tempuser1/.bash_profile

	# tempuser2's login scripts
	cat >> /home/$tempuser2/.bashrc <<-EOF
		echo "OK2"
	EOF
	chown $tempuser2 /home/$tempuser2/.bashrc
	cp -a /home/$tempuser2/.bashrc /home/$tempuser2/.bash_profile

	local expcmd=`which expect`
	cat > $TCTMP/exp$TCID <<-EOF
		#!$expcmd -f
		set timeout 5
		spawn su -l $tempuser1
		expect {
			timeout { exit 1 }
			OK1
		}
		send "su -l $tempuser2\r"
		expect {
			timeout { exit 2 }
			assword:
		}
		set timeout 30
		send "secret2\r"
		expect {
			timeout { exit 3 }
			OK2
		}
	EOF
	chmod +x $TCTMP/exp$TCID
	$TCTMP/exp$TCID >$stdout 2>$stderr
	tc_pass_or_fail $? "$tempuser2 unable to login using password"
}

################################################################################
# main
################################################################################

TST_TOTAL=3

tc_setup			# standard tc_setup

tc_root_or_break || exit

tc_add_user_or_break || exit
tempuser1=$temp_user
tc_add_user_or_break || exit
tempuser2=$temp_user

test01 && \
test02 && \
test03
